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

	// 実体化した影オブジェクトに打ち上げられた際の処理
	void Launched();

protected:
	std::unique_ptr<ObjectModel> object_;
	std::unique_ptr<OBBCollider> collider_;

	const float kDeltaTime = 1.0f / 60.0f;
	const float kGravity = -30.0f;

	Vector3 targetPosition_;
	float speed_;

	bool isDead_ = false;

	// 打ち上げられ時の垂直速度
	float verticalVelocity_;
	// 打ち上げられたかどうか記録（何回も打ち上げられないように）
	bool isLaunched_ = false;
	// 着地したか記録
	bool isLanding_ = false;
};



/// <summary>
/// ノーマル
/// </summary>
class NormalEnemy : public Enemy {
public:
	void Initialize(const Vector3& spawnPosition) override;
	void Update() override; // ノーマルの敵はタワーへ向かう
	void Draw(const Camera* camera) override;

private:
	uint32_t texture_;
};

