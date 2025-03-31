#include "Enemy.h"

// Engine
#include <Math/Vector/VectorFunction.h>

void NormalEnemy::Initialize(const Vector3& spawnPosition)
{
	object_ = std::make_unique<ObjectModel>("normalEnemy");
	object_->Initialize("Sphere/sphere.obj");
	object_->translate_ = spawnPosition;

	collider_ = std::make_unique<SphereCollider>("enemyCollider");
	collider_->SetLayer("enemy");
	collider_->SetRadius(1.0f);
	collider_->SetWorldTransform(object_->GetWorldTransform());

	speed_ = 4.0f;
}

void NormalEnemy::Update()
{
	///
	/// タワーへ移動させる
	/// 

	Vector3 direction = targetPosition_ - object_->translate_;
	direction = Normalize(direction);

	object_->translate_ += direction * speed_ * kDeltaTime;

	if (isDead_) {
		object_->translate_.z += 10.0f;
	}

	// 衝突したら死亡させる
	if (collider_->IsColliding()) {
		isDead_ = true;
	}

	object_->Update();
	CollisionManager::GetInstance()->RegisterCollider(collider_.get());
}

void NormalEnemy::Draw(const Camera* camera)
{
	object_->Draw(camera, { 1, 1, 1, 1 });
}
