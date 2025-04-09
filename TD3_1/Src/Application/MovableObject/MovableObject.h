#pragma once

// Engine
#include <Features/Model/ObjectModel.h>
#include <Features/Collision/Manager/CollisionManager.h>

class MovableObject {
public:
	virtual ~MovableObject() = default;

	virtual void Initialize() = 0;
	virtual void Update() = 0;
	virtual void Draw(const Camera& camera) = 0;

	void SetTranslate(const Vector3& translate) { object_->translate_ = translate; }
	Vector3 GetTranslate() const { return object_->translate_; }

	AABBCollider* GetCollider() const { return collider_ ? collider_.get() : nullptr; }

protected:
	std::unique_ptr<ObjectModel> object_;
	std::unique_ptr<AABBCollider> collider_;

	uint32_t texture_;
};

/// <summary>
/// 箱型オブジェクト
/// </summary>
class BoxObject : public MovableObject {
public:
	void Initialize() override;
	void Update() override;
	void Draw(const Camera& camera) override;
};

/// <summary>
/// 筒型オブジェクト
/// </summary>
class CylinderObject : public MovableObject {
public:
	void Initialize() override;
	void Update() override;
	void Draw(const Camera& camera) override;
};