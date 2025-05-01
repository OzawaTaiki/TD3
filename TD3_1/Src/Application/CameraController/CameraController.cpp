#include "CameraController.h"

void CameraController::Initialize(const Vector3& offset)
{
	offset_ = offset;
}

void CameraController::SetTarget(const Vector3* translate)
{
	targetTranslate_ = translate;
	if (targetTranslate_) {
		currentPos_ = *targetTranslate_ + offset_;
	}
}

void CameraController::Update()
{
	Vector3 targetPos = *targetTranslate_ + offset_;
	currentPos_ = Vector3::Lerp(currentPos_, targetPos, 0.1f);

	// 範囲制限
	const float range = 4.0f;
	currentPos_.x = std::clamp(currentPos_.x, offset_.x - range, offset_.x + range);
	currentPos_.z = std::clamp(currentPos_.z, offset_.z - range, offset_.z + range);
}

Vector3 CameraController::GetCameraPosition() const
{
	return currentPos_;
}
