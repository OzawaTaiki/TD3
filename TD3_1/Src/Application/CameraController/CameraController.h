#pragma once

// Engine
#include <Math/Vector/Vector3.h>
#include <Features/Model/ObjectModel.h>

class CameraController
{
public:
	void Initialize(const Vector3& offset);
	void SetTarget(const Vector3* translate);
	void Update();
	Vector3 GetCameraPosition() const;

private:
	const Vector3* targetTranslate_ = nullptr;
	Vector3 offset_;
	Vector3 currentPos_;
};