#include "Enemy.h"

// Engine
#include <Math/Vector/VectorFunction.h>

void NormalEnemy::Initialize(const Vector3& spawnPosition)
{
	object_ = std::make_unique<ObjectModel>("normalEnemy");
	object_->Initialize("Sphere/sphere.obj");
	object_->translate_ = spawnPosition;

	collider_ = std::make_unique<AABBCollider>("enemyCollider");
	collider_->SetLayer("enemy");
	/*衝突判定を行わないコライダーを設定*/
	collider_->SetLayerMask("enemy");
	/*----------------------------*/
	collider_->SetMinMax(object_->GetMin(), object_->GetMax());
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


	object_->Update();
	if (!isDead_) {
		CollisionManager::GetInstance()->RegisterCollider(collider_.get());
	}
}

void NormalEnemy::Draw(const Camera* camera)
{
	object_->Draw(camera, { 1, 1, 1, 1 });
}