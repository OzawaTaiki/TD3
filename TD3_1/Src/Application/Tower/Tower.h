#pragma once

// C++
#include <unordered_set>

// Engine
#include <Features/Model/ObjectModel.h>
#include <Features/Collision/Manager/CollisionManager.h>
#include <Features/Sprite/Sprite.h>

class Camera;

class Tower
{
public:
	void Initialize(const Vector3& position);
	void Update();
	void Draw(const Camera& camera);
	void DrawUI(const Camera& camera);
    void DrawShadow(const Camera& camera);

	void OnCollision(Collider* _other, const ColliderInfo& _info);

	Vector3 GetTranslate() const { return object_->translate_; }
	uint32_t GetHP() const { return hp_; }

private:
	std::unique_ptr<ObjectModel> object_;
	uint32_t texture_;
	std::unique_ptr<AABBCollider> collider_;

	uint32_t hp_ = 10; 

	// 敵と1度だけ衝突判定を行うよう、処理済みコライダーを追加
	std::unordered_set<Collider*> processedColliders_;

	// シェイク関連
	void StartShake(float duration, float intensity);
	void ApplyShake();
	const float kDeltaTime = 1.0f / 60.0f;
	Vector3 originalPosition_;
	float shakeDuration_ = 0.0f;
	float shakeElapsed_ = 0.0f;
	float shakeIntensity_ = 0.0f;
	bool isShaking_ = false;

private:
	std::unique_ptr<Sprite> spriteHP_;
};

