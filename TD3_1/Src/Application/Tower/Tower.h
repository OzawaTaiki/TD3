#pragma once

// Engine
#include <Features/Model/ObjectModel.h>
#include <Features/Collision/Manager/CollisionManager.h>

class Camera;

class Tower
{
public:
	void Initialize(const Vector3& position);
	void Update();
	void Draw(const Camera& camera);

private:
	std::unique_ptr<ObjectModel> object_;
	uint32_t texture_;
	std::unique_ptr<AABBCollider> collider_;
};

