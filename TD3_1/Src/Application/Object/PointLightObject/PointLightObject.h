#pragma once

// Engine
#include <Features/Model/ObjectModel.h>

class Camera;

/// <summary>
/// ポイントライトの設置とオブジェクトでの可視化
/// </summary>
class PointLightObject {
public:
	void Initialize(Vector3 position, float maxDistance);
	void Update();
	void Draw(const Camera& camera);

	const Vector3& GetTranslate() { return objectLight_->translate_; }
	const void SetTranslate(Vector3 translate) { objectLight_->translate_ = translate; }

	// 届く距離
	float maxDistance_;
	std::unique_ptr<ObjectModel> objectLight_;

private:
	uint32_t textureLight_;

	// 光が届く範囲
	std::unique_ptr<ObjectModel> objectCircle_;
	uint32_t textureCircle_;
};
