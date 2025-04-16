#pragma once

// Engine
#include <Features/Model/ObjectModel.h>
#include <Features/Collision/Manager/CollisionManager.h>

class MovableObject {
public:
	virtual ~MovableObject() = default;

	virtual void Initialize(float _hp) = 0;
	virtual void Update() = 0;
	virtual void Draw(const Camera& camera) = 0;

	void SetTranslate(const Vector3& translate) { object_->translate_ = translate; }
	void SetTranslateY(const float& y) { object_->translate_.y = y; }
	Vector3 GetTranslate() const { return object_->translate_; }

	void SetCanMove(bool canMove) { canMove_ = canMove; }
	bool CanMove() const { return canMove_; }

	AABBCollider* GetCollider() const { return collider_ ? collider_.get() : nullptr; }

	void OnCollision(Collider* _other, const ColliderInfo& _info);

	void SetLayerMask(std::string name) { collider_->SetLayerMask(name); }
	void ExcludeLayerMask(std::string name) { collider_->ExcludeLayerMask(name); }

	void Damage(const std::string& name, float damage);

    bool IsDead() const { return isDead_; }

	bool IsCollidingWithMovableObject() const { return isCollidingWithMovableObject_; }
	void SetIsCollidingWithMovableObject(bool flag) { isCollidingWithMovableObject_ = flag; }

protected:
	std::unique_ptr<ObjectModel> object_;
	std::unique_ptr<AABBCollider> collider_;

	uint32_t texture_;

    float hp_ = 10.0f; // HP
    bool isDead_ = false; // 死亡フラグ
    bool isHit_ = false; // 衝突フラグ
	bool canMove_ = true; // ドラッグで動かせるかどうか : 初期は動かせる状態
	bool isCollidingWithMovableObject_; // 動かせるオブジェクトと衝突しているか
};

/// <summary>
/// 箱型オブジェクト
/// </summary>
class BoxObject : public MovableObject {
public:
	void Initialize(float _hp) override;
	void Update() override;
	void Draw(const Camera& camera) override;
};

/// <summary>
/// 筒型オブジェクト
/// </summary>
class CylinderObject : public MovableObject {
public:
	void Initialize(float _hp) override;
	void Update() override;
	void Draw(const Camera& camera) override;
};