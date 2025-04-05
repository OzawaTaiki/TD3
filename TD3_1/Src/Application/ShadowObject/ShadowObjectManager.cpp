#include "ShadowObjectManager.h"

// Application
#include <Application/PointLightObject/PointLightObject.h>

void ShadowObjectManager::Initialize()
{
	shadowGroups_.clear();
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
				shadowObject->Initialize();
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
			shadowGroups_[i].shadowObjects_[j]->Update();
		}
	}
}

void ShadowObjectManager::Draw(const Camera& camera)
{
	for (size_t i = 0; i < shadowGroups_.size(); ++i) {
		for (size_t j = 0; j < shadowGroups_[i].shadowObjects_.size(); ++j) {
			shadowGroups_[i].shadowObjects_[j]->Draw(camera);
		}
	}
}
