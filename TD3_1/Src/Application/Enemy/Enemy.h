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

	virtual void Initialize(const Vector3& position,float _blockStopThreshold) = 0;
	virtual void Update() = 0;
	virtual void Draw(const Camera* camera) = 0;

	void SetTarget(const Vector3& target) { targetPosition_ = target; }
	bool IsDead() const { return isDead_; }
	void Dead() { isDead_ = true; PlayDeathSound(); }


	// 実体化した影オブジェクトに打ち上げられた際の処理
	void Launched();

    void SetSoundHandle(uint32_t soundHandle) { soundHandle_ = soundHandle; } // サウンドハンドルをセット
    void SetVolume(float volume) { deathSoundVolume_ = volume; } // ボリュームをセット
	void PlayDeathSound() const;

	void OnCollsion(Collider* _other, const ColliderInfo& _info);

	void OnForwardCollision(Collider* _other, const ColliderInfo& _info);

    void SetForwardCheckColliderOffset(const Vector3& offset) { forwardCheckCollider_->SetOffset(offset); }

protected:
	void InitialzeColliders();

	std::unique_ptr<ObjectModel> object_;
	std::unique_ptr<OBBCollider> collider_;
	std::unique_ptr<SphereCollider> forwardCheckCollider_; // 目の前にブロックがあるか確認するためのコライダー

	const float kDeltaTime = 1.0f / 60.0f;
	const float kGravity = -30.0f;

	Vector3 targetPosition_;
	float speed_;

	bool isDead_ = false;

	bool isBlocked = false; // ブロックに衝突して止まっているか
    float blockStopThreshold = 3.0f; // 止まり続けて死ぬまでの時間
    float blockedTimer_ = 0.0f; // 止まっている時間

	// 打ち上げられ時の垂直速度
	float verticalVelocity_;
	// 打ち上げられたかどうか記録（何回も打ち上げられないように）
	bool isLaunched_ = false;
	// 着地したか記録
	bool isLanding_ = false;

    uint32_t soundHandle_ = 0; // サウンドハンドル
    uint32_t voiceHandle_ = 0; // ボイスハンドル

    float deathSoundVolume_ = 0.5f; // 死亡時のサウンドボリューム
};



/// <summary>
/// ノーマル
/// </summary>
class NormalEnemy : public Enemy {
public:
	void Initialize(const Vector3& spawnPosition, float _blockStopThreshold) override;
	void Update() override; // ノーマルの敵はタワーへ向かう
	void Draw(const Camera* camera) override;

private:
	uint32_t texture_;
};

