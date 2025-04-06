#include "PointLightObjectManager.h"

// C++
#include <fstream>
#include <unordered_set>

// Externals
#include <imgui.h>
#include <json.hpp>

void PointLightObjectManager::Initialize()
{
	pointLightObjects_.clear();

	// JSONから読み込んだ数だけポイントライトオブジェクトを生成
	LoadFromFile();
}

void PointLightObjectManager::Update()
{
	for (auto& light : pointLightObjects_) {
		light->Update();
	}

#ifdef _DEBUG
	ImGui::Begin("PointLightObjectManager");
	if (ImGui::Button("Add Light")) { AddLight(); } ImGui::SameLine();
	if (ImGui::Button("Save")) { SaveToFile(); }

	// 全てのオブジェクト位置を設定できるように
	for (size_t i = 0; i < pointLightObjects_.size(); ++i) {
		if (!pointLightObjects_[i]) continue;
		Vector3 position = pointLightObjects_[i]->GetTranslate(); // 現在位置の取得

		std::string label = "Light[" + std::to_string(i) + "]";
		ImGui::DragFloat3(label.c_str(), &position.x, 0.1f);

		pointLightObjects_[i]->SetTranslate(position); // 位置の設定

		// Deleteボタンを同じ行に配置
		/*ImGui::SameLine();
		std::string deleteLabel = "Delete##" + std::to_string(i);
		if (ImGui::Button(deleteLabel.c_str())) {
			
		}*/
	}
	ImGui::End();
#endif
}

void PointLightObjectManager::Draw(const Camera& camera)
{
	for (auto& light : pointLightObjects_) {
		light->Draw(camera);
	}
}

void PointLightObjectManager::AddLight()
{
	auto object = std::make_unique<PointLightObject>();
	object->Initialize(Vector3(0, 5, 0));
	pointLightObjects_.push_back(std::move(object));
}

void PointLightObjectManager::SaveToFile()
{
	const std::string filePath = "Resources/Data/PointLightObject/pointLightObject_data.json";

	nlohmann::json jsonData;

	for (const auto& light : pointLightObjects_) {
		jsonData.push_back({
			{"position", {light->GetTranslate().x, light->GetTranslate().y, light->GetTranslate().z}}
			});
	}

	std::ofstream file(filePath);
	file << jsonData.dump(4);
}

void PointLightObjectManager::LoadFromFile()
{
	const std::string filePath = "Resources/Data/PointLightObject/pointLightObject_data.json";

	std::ifstream file(filePath);
	nlohmann::json jsonData;
	file >> jsonData;

	pointLightObjects_.clear();
	for (const auto& entry : jsonData) {
		auto object = std::make_unique<PointLightObject>();
		object->Initialize({entry["position"][0], entry["position"][1], entry["position"][2]});
		pointLightObjects_.push_back(std::move(object));
	}
}
