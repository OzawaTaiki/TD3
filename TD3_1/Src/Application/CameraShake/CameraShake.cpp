#include "CameraShake.h"

// C++
#include <random>
#include <fstream>

// Externals
#include <imgui.h>
#include <json.hpp>

CameraShake* CameraShake::GetInstance() {
	static CameraShake instance;
	return &instance;
}

void CameraShake::Initialize() { 
	LoadToFile();

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
	ImGui::DragFloat("duration", &duration_, 0.01f);
	ImGui::DragFloat("intensity", &intensity_, 0.01f);
	if (ImGui::Button("Save")) {
		SaveToFile();
	}

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

void CameraShake::SaveToFile() { 
	const std::string filePath = "Resources/Data/CameraShake/cameraShakeData.json"; 
	nlohmann::json jsonData;

	jsonData.push_back({
	    {"duration",  duration_ },
        {"intensity", intensity_}
    });

	std::ofstream file(filePath);
	file << jsonData.dump(4);
}

void CameraShake::LoadToFile() { 
	const std::string filePath = "Resources/Data/CameraShake/cameraShakeData.json";

	std::ifstream file(filePath);
	nlohmann::json jsonData;
	file >> jsonData;

	duration_ = jsonData[0]["duration"].get<float>();
	intensity_ = jsonData[0]["intensity"].get<float>();
}
