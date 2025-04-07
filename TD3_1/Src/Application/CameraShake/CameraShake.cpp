#include "CameraShake.h"

// C++
#include <random>

// Externals
#include <imgui.h>

CameraShake* CameraShake::GetInstance() {
	static CameraShake instance;
	return &instance;
}

void CameraShake::Initialize(float duration, float intensity) { 
	duration_ = duration;
	intensity_ = intensity;
	elapsedTime_ = 0.0f;
	isShaking_ = false;
}

void CameraShake::Update() { 
	if (isShaking_) {
		elapsedTime_ += kDeltaTime;
		// シェイク終了
		if (elapsedTime_ >= duration_) {
			isShaking_ = false;
			offset_ = Vector3(0.0f, 0.0f, 0.0f); // オフセットのリセット
		// シェイク中
		} else {
			ApplyShake();
		}
	}

#ifdef _DEBUG
	ImGui::Begin("CameraShake");
	if (ImGui::Button("StartShake")) {
		StartShake();
	}
	ImGui::Text("x : %.2f, y : %.2f, z : %.2f", offset_.x, offset_.y, offset_.z);
	ImGui::End();
#endif
}

void CameraShake::StartShake() { 
	elapsedTime_ = 0.0f;
	isShaking_ = true; // シェイク開始
}

void CameraShake::ApplyShake() { 
	// 残り時間に基づいて強度を線形に減少させる
	float remainingTime = duration_ - elapsedTime_; // 残り時間
	float currentIntensity = intensity_ * (remainingTime / duration_);

	std::random_device rd; 
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> dis(-currentIntensity, currentIntensity);
	offset_ = Vector3(dis(gen), dis(gen), dis(gen));
}
