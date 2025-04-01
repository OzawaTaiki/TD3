#pragma once

// Engine
#include <Features/Model/ObjectModel.h>
#include <Features/Collision/Manager/CollisionManager.h>

class Camera;

/// <summary>
/// 基底クラス
/// </summary>
class Enemy
{
public:
	virtual ~Enemy() = default;

	virtual void Initialize(const Vector3& position) = 0;
	virtual void Update() = 0;
	virtual void Draw(const Camera* camera) = 0;

	void SetTarget(const Vector3& target) { targetPosition_ = target; }
	bool IsDead() const { return isDead_; }
	void Dead() { isDead_ = true; }

protected:
	std::unique_ptr<ObjectModel> object_;
	std::unique_ptr<SphereCollider> collider_;

	const float kDeltaTime = 1.0f / 60.0f;

	Vector3 targetPosition_;
	float speed_;

	bool isDead_ = false;
};



/// <summary>
/// ノーマル
/// </summary>
class NormalEnemy : public Enemy {
public:
	void Initialize(const Vector3& spawnPosition) override;
	void Update() override; // ノーマルの敵はタワーへ向かう
	void Draw(const Camera* camera) override;
};

