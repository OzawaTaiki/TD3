#include "Enemy.h"

// Engine
#include <Math/Vector/VectorFunction.h>
#include <Core/DXCommon/TextureManager/TextureManager.h>
#include <Features/Collision/CollisionLayer/CollisionLayerManager.h>
#include <Features/Event/EventManager.h>
#include <System/Audio/Audio.h>
#include <Debug/ImguITools.h>

#include "EnemyAttackInfo.h"

void NormalEnemy::Initialize(const Vector3& spawnPosition, float _blockStopThreshold)
{
	object_ = std::make_unique<ObjectModel>("normalEnemy");
	object_->Initialize("Enemy/enemy.obj");
	object_->translate_ = spawnPosition;
	object_->useQuaternion_ = true;

	texture_ = TextureManager::GetInstance()->Load("game/enemy/enemy.png");

	InitialzeColliders();

	speed_ = 2.0f;

	blockStopThreshold = _blockStopThreshold;

	InitializeAnimSeq();
}

void NormalEnemy::Update()
{
	// ターゲットの位置まで移動
	if (!isBlocked && !isAttacking_)
	{
		// 衝突していないとき
		Vector3 direction = targetPosition_ - object_->translate_;
		direction = Normalize(direction);
		direction.y = 0; // 高さを固定するため、Y成分を無効化
		object_->translate_ += direction * speed_ * kDeltaTime;
	}
	else
	{
        // 衝突しているとき
        blockedTimer_ += kDeltaTime; // 衝突している時間を加算
        if (blockedTimer_ >= blockStopThreshold) {
            // 一定時間衝突していたら死亡
            Dead();
        }
	}

	if (isAttacking_)
		Attack();
	// ターゲット方向に回転を設定
	Vector3 forward = Vector3(0, 0, 1);
	Vector3 targetDirection = Normalize(targetPosition_ - object_->translate_);
	targetDirection.y = 0.0f;
	object_->quaternion_ = Quaternion::FromToRotation(forward, targetDirection);



	object_->Update();
	if (!isDead_) {
		CollisionManager::GetInstance()->RegisterCollider(collider_.get());
		CollisionManager::GetInstance()->RegisterCollider(forwardCheckCollider_.get());
	}

	// 打ち上げられたら重力の影響を受ける
	if (isLaunched_) {
		// 垂直速度の更新（重力の影響）
		verticalVelocity_ += kGravity * kDeltaTime;
		// オブジェクトの高さ更新
		object_->translate_.y += verticalVelocity_ * kDeltaTime;

		// 打ち上げられ後、着地したら落下を止めて死亡させる
		if (object_->translate_.y <= 1.0f) {
			object_->translate_.y = 1.0f; // 地面に固定
			Dead();
		}
	}

#ifdef _DEBUG
	//ImGuiTool::TimeLine("enemy", animSeq_.get());
    //if (ImGui::Button("Seq Save"))
    //{
    //     animSeq_->Save();
    // }
	//blockedTimer_ = 0.0f; // 衝突した瞬間にタイマーをリセット

#endif // ?DEBUG


	if (!isAttacking_)
		isBlocked = false;
}

void NormalEnemy::Draw(const Camera* camera)
{
	object_->Draw(camera, texture_, { 1, 1, 1, 1 });
}

void NormalEnemy::InitializeAnimSeq()
{
	animSeq_ = std::make_unique<AnimationSequence>("enemy");
	animSeq_->Initialize("Resources/data/animationSeq/enemy/");
}

void Enemy::PlayDeathSound() const
{
    Audio::GetInstance()->SoundPlay(soundHandle_, deathSoundVolume_); // 死亡時のサウンドを再生
}

void Enemy::Launched()
{
	/*敵を打ち上げる処理*/
	if (!isLaunched_) {
		verticalVelocity_ = 20.0f; // 初期垂直初速を設定
		isLaunched_ = true; // 打ち上げられたことを記録
	}
}

void Enemy::OnCollsion(Collider* _other, const ColliderInfo& _info)
{
	CollisionLayerManager* collisionManager = CollisionLayerManager::GetInstance();

	// 影オブジェクトとの衝突
	uint32_t shadowObjectLayer = collisionManager->GetLayer("ShadowObject"); // 影オブジェクトのレイヤーを取得
	if (_other->GetLayer() == shadowObjectLayer) {
		this->Launched(); // 衝突した敵を打ち上げる
		if (_info.state == CollisionState::Enter)
			EventManager::GetInstance()->DispatchEvent(GameEvent("EnemyLaunchKill", nullptr)); // イベントを発行
	}

	// タワーとの衝突
	uint32_t towerObjectLayer = collisionManager->GetLayer("Tower");
	if (_other->GetLayer() == towerObjectLayer) {
		this->Dead();
	}

}

void Enemy::OnForwardCollision(Collider* _other, const ColliderInfo& _info)
{
	CollisionLayerManager* collisionManager = CollisionLayerManager::GetInstance();

	uint32_t movableObjectLayer = collisionManager->GetLayer("movableObject");
	if (_other->GetLayer() == movableObjectLayer) {

		if (_info.state == CollisionState::Enter)
		{
			if (!isAttacking_ && !isBlocked)
            blockedTimer_ = 0.0f; // 衝突した瞬間にタイマーをリセット
		}
        if (_info.state == CollisionState::Stay)
        {
			if (canAttack_ && !isAttacking_)
			{
                isAttacking_ = true; // 攻撃中にする
                prePos_ = object_->translate_; // 衝突した瞬間の位置を保存
                attackObjectName_ = _other->GetName(); // 攻撃対象のオブジェクト名を保存
			}
        }

		isBlocked = true;
	}
}

void Enemy::InitialzeColliders()
{
	collider_ = std::make_unique<OBBCollider>("enemyCollider");
	collider_->SetLayer("enemy");
	/*衝突判定を行わないコライダーを設定*/
	collider_->SetLayerMask("enemy");
	/*----------------------------*/
	Vector3 halfExtents = (object_->GetMax() - object_->GetMin()) * 0.5f;
	collider_->SetHalfExtents(halfExtents);
	Vector3 localPivot = (object_->GetMax() + object_->GetMin()) * 0.5f;
	collider_->SetLocalPivot(localPivot);
	collider_->SetWorldTransform(object_->GetWorldTransform());
	collider_->SetOnCollisionCallback([this](Collider* _other, const ColliderInfo& _info) {
		OnCollsion(_other, _info);
		});

	forwardCheckCollider_ = std::make_unique<SphereCollider>("enemy_forward");
    forwardCheckCollider_->SetLayer("enemy_forward");
	forwardCheckCollider_->SetCollisionLayer("movableObject"); // これだけに衝突するよ
    forwardCheckCollider_->SetRadius(1.0f);
	forwardCheckCollider_->SetOffset(Vector3(0, 0, 1));// 前方にオフセット
    forwardCheckCollider_->SetWorldTransform(object_->GetWorldTransform());
	forwardCheckCollider_->SetOnCollisionCallback([this](Collider* _other, const ColliderInfo& _info) {
        OnForwardCollision(_other, _info);
		});
    forwardCheckCollider_->SetDrawFlag(false);

}

void Enemy::Attack()
{
	if (!isBlocked)
	{
		canAttack_ = true; // 攻撃可能状態へ
		return; // ブロックに衝突していないときは攻撃しない
	}


    isAttacking_ = true; // 攻撃中にする
	canAttack_ = false;

	// 攻撃インターバルには止まってる時間を利用

    attackTimer_ += kDeltaTime; // 攻撃タイマーを加算
    animSeq_->Update(kDeltaTime); // アニメーションの更新

    if (attackTimer_ >= attackInterval_)
    {
        EnemyAttackInfo attackInfo;
		attackInfo.name = attackObjectName_; // 攻撃対象のオブジェクト名を保存
        attackInfo.damage = damage_; // 攻撃力を保存


        // 攻撃処理
        EventManager::GetInstance()->DispatchEvent(GameEvent("EnemyAttack", &attackInfo));
		attackTimer_ = 0.0f; // 攻撃後にタイマーをリセット

        isAttacking_ = false; // 攻撃終了
		canAttack_ = true;
        attackObjectName_ = ""; // 攻撃対象のオブジェクト名をリセット
        animSeq_->SetCurrentTime(0.0f); // アニメーションの時間をリセット
    }

	Vector3 move = Transform(animSeq_->GetValue<Vector3>("pos"), object_->quaternion_.ToMatrix());

	// ちょっとした攻撃モーション
	object_->translate_ = prePos_ + move;

}
