#include "Enemy.h"

// Engine
#include <Math/Vector/VectorFunction.h>
#include <Core/DXCommon/TextureManager/TextureManager.h>
#include <Features/Collision/CollisionLayer/CollisionLayerManager.h>
#include <Features/Event/EventManager.h>
#include <System/Audio/Audio.h>
#include <Debug/ImguITools.h>
#include <Math/Random/RandomGenerator.h>

#include "EnemyAttackInfo.h"

// Application
#include <Application/Util/SimpleEasing/SimpleEasing.h>

void NormalEnemy::Initialize(const Vector3& spawnPosition, float _blockStopThreshold)
{
	object_ = std::make_unique<ObjectModel>("normalEnemy");
	object_->Initialize("Enemy/enemy.obj");
	object_->translate_ = spawnPosition;
	object_->useQuaternion_ = true;

	texture_ = TextureManager::GetInstance()->Load("game/enemy.png");

	InitialzeColliders();

	speed_ = 1.5f;

	blockStopThreshold = _blockStopThreshold;

	InitializeAnimSeq();

	// 移動時パーティクル初期化
	bubbleParticle_ = std::make_unique<BubbleParticle>();
	bubbleParticle_->Initialize();
}

void NormalEnemy::Update()
{
	// 前フレームの位置を記録
	Vector3 prePos = object_->translate_;

	// ターゲットの位置まで移動
	if (!isBlocked && !isAttacking_ && !isLaunched_)
	{
		if (route_ == nullptr)
		{
			// 衝突していないとき
			Vector3 direction = targetPosition_ - object_->translate_;
			direction = Normalize(direction);
			direction.y = 0; // 高さを固定するため、Y成分を無効化
			object_->translate_ += direction * speed_ * kDeltaTime;
		}
        else
        {
            distance_ += speed_ * kDeltaTime;
            float totalLength = route_->CalculateTotalLength();
            if (distance_ > totalLength)
            {
                distance_ = 1.0f;
            }

			Vector3 targetPos = route_->GetPointAtDistance(distance_);
			targetPos.y = object_->translate_.y;
            object_->translate_ = targetPos;
		}
	}
	else
	{
        // 衝突しているとき
        blockedTimer_ += kDeltaTime; // 衝突している時間を加算
        if (blockedTimer_ >= blockStopThreshold) {
            // 一定時間衝突していたら拡縮して死亡
			ScalingAndDead();
        }
	}

	if (isAttacking_ && !isLaunched_)
		Attack();
	// ターゲット方向に回転を設定
	Vector3 forward = Vector3(0, 0, 1);
	Vector3 targetDirection = Normalize(targetPosition_ - object_->translate_);
	targetDirection.y = 0.0f;
	object_->quaternion_ = Quaternion::FromToRotation(forward, targetDirection);



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

		// 打ち上げられ後、着地したら落下を止める
		if (object_->translate_.y <= 1.0f) {
			object_->translate_.y = 1.0f; // 地面に固定
			/*Dead();*/

			// 着地したら拡縮して死亡
			ScalingAndDead();
		}
	}

	// 生きていれば移動パーティクルを発生
	if (!IsDead()) {
		// 5~10フレーム毎に生成
		emitCounter_++;
		const uint32_t interval = RandomGenerator::GetInstance()->GetRandValue(5, 10);
		if (emitCounter_ >= interval) {
			// 後ろの位置を計算
			Vector3 backwardPosition = object_->translate_ - CalculateFoward(object_->quaternion_) * (object_->scale_.z * 0.5f * 3.0f); // 0.5fの後に掛ける値でどれくらい後ろから出るか調整

			bubbleParticle_->Emit(backwardPosition);
			emitCounter_ = 0;
		}
	}

	object_->Update();

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

void NormalEnemy::DrawShadow(const Camera* camera)
{
    object_->DrawShadow(camera, 0);
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

void Enemy::ScalingAndDead()
{
	if (!hasScaled_) {
		SimpleEasing::Animate(scale_, scale_, 0.0f, Easing::EaseInBack, 1.0f);
		hasScaled_ = true;
	}
	
	object_->scale_ = { scale_, scale_, scale_ };

	if (scale_ <= 0.0f) {
		Dead();
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
	uint32_t shadowObjectLayer = collisionManager->GetLayer("ShadowObject"); // 影オブジェクトのレイヤーを取得
	uint32_t shadowObjectReturningLayer = collisionManager->GetLayer("ShadowObjectReturning"); // 影オブジェクトのレイヤーを取得
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
	else if (_other->GetLayer() == shadowObjectLayer) {

		if (_info.state == CollisionState::Enter)
		{
			if (!isAttacking_ && !isBlocked)
				blockedTimer_ = 0.0f; // 衝突した瞬間にタイマーをリセット
		}
		isBlocked = true;
	}
	else if (_other->GetLayer() == shadowObjectReturningLayer) {

		if (_info.state == CollisionState::Enter)
		{
			if (!isAttacking_ && !isBlocked)
				blockedTimer_ = 0.0f; // 衝突した瞬間にタイマーをリセット
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
	collider_->SetLayerMask("Wall");
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
    forwardCheckCollider_->AddCollisionLayer("ShadowObject");
    forwardCheckCollider_->AddCollisionLayer("ShadowObjectReturning");
    forwardCheckCollider_->SetRadius(1.0f);
	forwardCheckCollider_->SetOffset(Vector3(0, 0.6f, 1));// 前方にオフセット
    forwardCheckCollider_->SetWorldTransform(object_->GetWorldTransform());
	forwardCheckCollider_->SetOnCollisionCallback([this](Collider* _other, const ColliderInfo& _info) {
        OnForwardCollision(_other, _info);
		});
    forwardCheckCollider_->SetDrawFlag(true);

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

	if (attackTimer_ >= damageActivationTime && !isDamageActivated)
	{
		EnemyAttackInfo attackInfo;
		attackInfo.name = attackObjectName_; // 攻撃対象のオブジェクト名を保存
		attackInfo.damage = damage_; // 攻撃力を保存

		// 攻撃開始後ダメージが入る瞬間の時間を記録する
		isDamageActivated = true;
		// 攻撃対象のオブジェクトにダメージを与える
		EventManager::GetInstance()->DispatchEvent(GameEvent("EnemyAttack", &attackInfo));
		// ダメージを与えたら攻撃対象のオブジェクト名をリセット
		attackObjectName_ = "";

	};

    if (attackTimer_ >= attackInterval_)
	{
        // 攻撃処理
		attackTimer_ = 0.0f; // 攻撃後にタイマーをリセット

        isAttacking_ = false; // 攻撃終了
		canAttack_ = true;
        animSeq_->SetCurrentTime(0.0f); // アニメーションの時間をリセット
        isDamageActivated = false; // ダメージが入る瞬間の時間をリセット
    }

	Vector3 move = Transform(animSeq_->GetValue<Vector3>("pos"), object_->quaternion_.ToMatrix());

	// ちょっとした攻撃モーション
	object_->translate_ = prePos_ + move;

}

Vector3 Enemy::CalculateFoward(const Quaternion& quaternion)
{
	// ローカル座標の前方向
	Vector3 localForward = { 0.0f, 0.0f, 1.0f };

	// クォータニオンを使ってローカル座標を回転
	float x = localForward.x * (1 - 2 * (quaternion.y * quaternion.y + quaternion.z * quaternion.z)) +
		localForward.y * (2 * (quaternion.x * quaternion.y - quaternion.z * quaternion.w)) +
		localForward.z * (2 * (quaternion.x * quaternion.z + quaternion.y * quaternion.w));

	float y = localForward.x * (2 * (quaternion.x * quaternion.y + quaternion.z * quaternion.w)) +
		localForward.y * (1 - 2 * (quaternion.x * quaternion.x + quaternion.z * quaternion.z)) +
		localForward.z * (2 * (quaternion.y * quaternion.z - quaternion.x * quaternion.w));

	float z = localForward.x * (2 * (quaternion.x * quaternion.z - quaternion.y * quaternion.w)) +
		localForward.y * (2 * (quaternion.y * quaternion.z + quaternion.x * quaternion.w)) +
		localForward.z * (1 - 2 * (quaternion.x * quaternion.x + quaternion.y * quaternion.y));

	return Vector3(x, y, z);

}
