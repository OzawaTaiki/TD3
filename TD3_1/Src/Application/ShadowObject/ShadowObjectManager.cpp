#include "ShadowObjectManager.h"

// C++
#include <fstream>
#include <json.hpp>

// Application
#include <Application/PointLightObject/PointLightObject.h>

void ShadowObjectManager::Initialize()
{
	shadowGroups_.clear();
	LoadFromFile();
}

void ShadowObjectManager::Update(const std::vector<Vector3>& movableObjects, const std::vector<std::unique_ptr<PointLightObject>>& pointLightObjects)
{
	// ポイントライトと影オブジェクトの数を同期
	if (movableObjects.size() != shadowGroups_.size()) {
		shadowGroups_.clear(); // クリアして再生成
		for (const auto& movable : movableObjects) {
			ShadowGroup group;
			group.movableObjectPosition_ = movable;

			for (const auto& pointLight : pointLightObjects) {
				auto shadowObject = std::make_unique<ShadowObject>();
				shadowObject->Initialize(waitDuration_);
				shadowObject->SetMovableObjectPosition(movable);
				shadowObject->SetLightPosition(pointLight->GetTranslate());
				group.shadowObjects_.push_back(std::move(shadowObject));
			}

			shadowGroups_.push_back(std::move(group));
		}
	}

	// 各グループ内の影オブジェクトを更新
	for (size_t i = 0; i < shadowGroups_.size(); ++i) {
		shadowGroups_[i].movableObjectPosition_ = movableObjects[i];
		for (size_t j = 0; j < shadowGroups_[i].shadowObjects_.size(); ++j) {
			shadowGroups_[i].shadowObjects_[j]->SetMovableObjectPosition(movableObjects[i]);
			shadowGroups_[i].shadowObjects_[j]->SetLightPosition(pointLightObjects[j]->GetTranslate());

			shadowGroups_[i].shadowObjects_[j]->SetWaitDuration(waitDuration_); // memo : waitDurationが確定したらこの更新はいらないので消す

			// ライト毎に設定されている最大距離を適用
			float maxDistance = pointLightObjects[j]->maxDistance_;
			shadowGroups_[i].shadowObjects_[j]->Update(maxDistance);
		}
	}

#ifdef _DEBUG
	ImGui::Begin("ShadowObjectManager");
	ImGui::DragFloat("waitDuration", &waitDuration_, 0.01f);
	if (ImGui::Button("Save"))
		SaveToFile();
	ImGui::End();
#endif
}

void ShadowObjectManager::Draw(const Camera& camera)
{
	for (size_t i = 0; i < shadowGroups_.size(); ++i) {
		for (size_t j = 0; j < shadowGroups_[i].shadowObjects_.size(); ++j) {
			shadowGroups_[i].shadowObjects_[j]->Draw(camera);
		}
	}
}

void ShadowObjectManager::SaveToFile()
{
	const std::string filePath = "Resources/Data/ShadowObject/shadowObjectManager.json";
	nlohmann::json jsonData;

	jsonData.push_back({
		{"waitDuration", waitDuration_}
		});

	std::ofstream file(filePath);
	file << jsonData.dump(4);
}

void ShadowObjectManager::LoadFromFile()
{
	const std::string filePath = "Resources/Data/ShadowObject/shadowObjectManager.json";

	std::ifstream file(filePath);
	nlohmann::json jsonData;
	file >> jsonData;

	waitDuration_ = jsonData[0]["waitDuration"].get<float>();
}
