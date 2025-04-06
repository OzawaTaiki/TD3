#include "Enemy.h"

// Engine
#include <Math/Vector/VectorFunction.h>
#include <Core/DXCommon/TextureManager/TextureManager.h>

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
		this->Dead();
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
}

void NormalEnemy::Draw(const Camera* camera)
{
	object_->Draw(camera, { 1, 1, 1, 1 });
}