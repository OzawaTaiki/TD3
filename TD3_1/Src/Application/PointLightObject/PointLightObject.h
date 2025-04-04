#pragma once

// Engine
#include <Features/Model/ObjectModel.h>

class Camera;

/// <summary>
/// ポイントライトの設置とオブジェクトでの可視化
/// </summary>
class PointLightObject {
public:
	void Initialize(Vector3 position);
	void Update();
	void Draw(const Camera& camera);

	const Vector3& GetTranslate() { return object_->translate_; }

private:
	std::unique_ptr<ObjectModel> object_;
	uint32_t texture_;
};
