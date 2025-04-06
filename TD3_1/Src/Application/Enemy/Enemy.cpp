#include "Enemy.h"

// Engine
#include <Math/Vector/VectorFunction.h>
#include <Core/DXCommon/TextureManager/TextureManager.h>
#include <Features/Collision/CollisionLayer/CollisionLayerManager.h>

void NormalEnemy::Initialize(const Vector3& spawnPosition)
{
	object_ = std::make_unique<ObjectModel>("normalEnemy");
	object_->Initialize("Enemy/enemy.obj");
	object_->translate_ = spawnPosition;
	object_->useQuaternion_ = true;

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
		// 影オブジェクトとの衝突
		uint32_t shadowObjectLayer = CollisionLayerManager::GetInstance()->GetLayer("ShadowObject"); // 影オブジェクトのレイヤーを取得
		if (_other->GetLayer() == shadowObjectLayer) {
			this->Launched(); // 衝突した敵を打ち上げる
		}

		// タワーとの衝突
		uint32_t towerObjectLayer = CollisionLayerManager::GetInstance()->GetLayer("Tower");
		if (_other->GetLayer() == towerObjectLayer) {
			this->Dead();
		}

		});

	speed_ = 4.0f;
}

void NormalEnemy::Update()
{
	// ターゲットの位置まで移動
	Vector3 direction = targetPosition_ - object_->translate_;
	direction = Normalize(direction);
	direction.y = 0; // 高さを固定するため、Y成分を無効化
	object_->translate_ += direction * speed_ * kDeltaTime;

	// ターゲット方向に回転を設定
	Vector3 forward = Vector3(0, 0, 1);
	Vector3 targetDirection = Normalize(targetPosition_ - object_->translate_);
	targetDirection.y = 0.0f;
	object_->quaternion_ = Quaternion::FromToRotation(forward, targetDirection);

	object_->Update();
	if (!isDead_) {
		// 生きている間はコライダー更新
		Vector3 halfExtents = (object_->GetMax() - object_->GetMin()) * 0.5f;
		collider_->SetHalfExtents(halfExtents);
		Vector3 localPivot = (object_->GetMax() + object_->GetMin()) * 0.5f;
		collider_->SetLocalPivot(localPivot);
		collider_->SetWorldTransform(object_->GetWorldTransform());
		CollisionManager::GetInstance()->RegisterCollider(collider_.get());
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
		}
	}
}

void NormalEnemy::Draw(const Camera* camera)
{
	object_->Draw(camera, { 1, 1, 1, 1 });
}

void Enemy::Launched()
{
	/*敵を打ち上げる処理*/
	if (!isLaunched_) {
		verticalVelocity_ = 20.0f; // 初期垂直初速を設定
		isLaunched_ = true; // 打ち上げられたことを記録
	}
}
