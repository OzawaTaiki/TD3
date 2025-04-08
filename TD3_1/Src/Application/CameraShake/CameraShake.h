#pragma once

// Engine
#include <Math/Vector/Vector3.h>

class CameraShake {
public:
	static CameraShake* GetInstance();
	void Initialize(float duration, float intensity);
	void Update();

	// これを呼んでシェイク開始
	void StartShake();
	const Vector3& GetOffset() const { return offset_; }

private:
	void ApplyShake();

	float duration_ = 0.0f; // 継続時間
	float intensity_ = 0.0f; // 強度
	float elapsedTime_ = 0.0f; // 経過時間
	const float kDeltaTime = 1.0f / 60.0f;
	bool isShaking_ = false; // シェイク中？
	Vector3 offset_; // カメラに加えるオフセット
};
