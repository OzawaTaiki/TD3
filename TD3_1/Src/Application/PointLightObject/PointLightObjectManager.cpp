#include "PointLightObjectManager.h"

// Externals
#include "imgui.h"

void PointLightObjectManager::Initialize()
{
	pointLightObjects_.clear();

	// 一旦手動で追加しておく（あとでJSONで管理するようにして、ImGuiで追加・削除が行えるように）
	auto pointLightObject0 = std::make_unique<PointLightObject>();
	pointLightObject0->Initialize(Vector3(-13.0f, 5.0f, -15.0f));
	pointLightObjects_.push_back(std::move(pointLightObject0));

	auto pointLightobject1 = std::make_unique<PointLightObject>();
	pointLightobject1->Initialize(Vector3(16.5f, 5.0f, 6.0f));
	pointLightObjects_.push_back(std::move(pointLightobject1));
}

void PointLightObjectManager::Update()
{
	for (auto& light : pointLightObjects_) {
		light->Update();
	}

#ifdef _DEBUG
	ImGui::Begin("PointLightObjectManager");
	for (size_t i = 0; i < pointLightObjects_.size(); i++) {
		Vector3& position = const_cast<Vector3&>(pointLightObjects_[i]->GetTranslate());
		std::string label = "Light[" + std::to_string(i) + "] Position";
		ImGui::DragFloat3(label.c_str(), &position.x, 0.1f);
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
