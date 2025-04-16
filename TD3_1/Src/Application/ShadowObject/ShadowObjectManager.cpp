#include "ShadowObjectManager.h"
// Engine
#include <System/Audio/Audio.h>

// C++
#include <fstream>

// Application
#include <Application/PointLightObject/PointLightObject.h>
#include <Application/MovableObject/MovableObject.h>

// Externals
#include <json.hpp>

void ShadowObjectManager::Initialize()
{
	shadowGroups_.clear();
	LoadFromFile();

    materializationSoundHandle_ = Audio::GetInstance()->SoundLoadWave("Resources/audio/shadowMaterialization.wav");
    materializationSoundVolume_ = 0.5f; // デフォルトのボリュームを設定

}

void ShadowObjectManager::Update(const std::vector<std::unique_ptr<MovableObject>>& movableObjects, const std::vector<std::unique_ptr<PointLightObject>>& pointLightObjects, bool isDragging) {
	// 動かせるオブジェクトとグループの数を同期
	if (movableObjects.size() < shadowGroups_.size()) {
		shadowGroups_.resize(movableObjects.size()); // 余分なグループを削除
	}

	for (size_t i = 0; i < movableObjects.size(); ++i) {
		// movableObjectが増えた場合には新しいグループの追加
		if (i >= shadowGroups_.size()) { // movableObjectsがshadowGroupsを超えた場合（追加された場合）を探知
			ShadowGroup group;
			group.movableObjectPosition_ = movableObjects[i]->GetTranslate();

			// pointLightの数だけ、このグループにshadowObjectを生成
			for (const auto& pointLight : pointLightObjects) {
				auto shadowObject = std::make_unique<ShadowObject>();
				shadowObject->Initialize(waitDuration_);
				shadowObject->SetMovableObjectPosition(movableObjects[i]->GetTranslate());
				shadowObject->SetLightPosition(pointLight->GetTranslate());
				// 生成したshadowObjectをこのグループのshadowObjectsに追加していく
				group.shadowObjects_.push_back(std::move(shadowObject));
			}
			// 生成し終えたグループをshadowGroupsに追加
			shadowGroups_.push_back(std::move(group));

		// movableObjectが増えていない状態を維持している場合には既存のグループを再利用
		} else {
			shadowGroups_[i].movableObjectPosition_ = movableObjects[i]->GetTranslate();

			if (pointLightObjects.size() != shadowGroups_[i].shadowObjects_.size()) {
				shadowGroups_[i].shadowObjects_.resize(pointLightObjects.size());
				for (size_t j = 0; j < pointLightObjects.size(); ++j) {
					if (!shadowGroups_[i].shadowObjects_[j]) {
						shadowGroups_[i].shadowObjects_[j] = std::make_unique<ShadowObject>();
						shadowGroups_[i].shadowObjects_[j]->Initialize(waitDuration_);
					}
					shadowGroups_[i].shadowObjects_[j]->SetMovableObjectPosition(movableObjects[i]->GetTranslate());
					shadowGroups_[i].shadowObjects_[j]->SetLightPosition(pointLightObjects[j]->GetTranslate());
				}
			}
		}
	}

	// 各グループ内の影オブジェクトを更新
	for (size_t i = 0; i < shadowGroups_.size(); ++i) {
		bool isScalingAny = false; // グループ内のshadowObjectが1つでも実体化しているかどうか

		shadowGroups_[i].movableObjectPosition_ = movableObjects[i]->GetTranslate();
		for (size_t j = 0; j < shadowGroups_[i].shadowObjects_.size(); ++j) {
			shadowGroups_[i].shadowObjects_[j]->SetMovableObjectPosition(movableObjects[i]->GetTranslate());
			shadowGroups_[i].shadowObjects_[j]->SetLightPosition(pointLightObjects[j]->GetTranslate());

			shadowGroups_[i].shadowObjects_[j]->SetWaitDuration(waitDuration_); // memo : waitDurationが確定したらこの更新はいらないので消す

			// ライト毎に設定されている最大距離を適用
			float maxDistance = pointLightObjects[j]->maxDistance_;
			shadowGroups_[i].shadowObjects_[j]->Update(maxDistance, isDragging);

			// グループ内で1つでも実体化しているか確認
			if (shadowGroups_[i].shadowObjects_[j]->IsScaling()) {
				isScalingAny = true;
			}
		}
		// グループ内で1つでも実体化していれば、寄生先のmovableObjectが動かないように設定
		movableObjects[i]->SetCanMove(!isScalingAny);
	}

	// 実体化してる影オブジェクトのカウントに応じて、持続時間を動的に変更
	UpdateWaitDuration();

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

size_t ShadowObjectManager::GetScalingShadowObjectsCount() const
{
	size_t count = 0;

	for (const auto& group : shadowGroups_) {
		for (const auto& shadowObject : group.shadowObjects_) {
			if (shadowObject && shadowObject->IsScaling()) {
				++count;
			}
		}
	}

	return count;
}

void ShadowObjectManager::UpdateWaitDuration()
{
	// 実体化中の影オブジェクトをカウント
	size_t scalingCount = GetScalingShadowObjectsCount();

	// 基準値と持続時間の係数を設定
	const float factor = 0.1f; // 実体化中オブジェクト1つあたりの増加時間

	// 動的にwaitDurationを計算
	waitDuration_ = ShadowObject::kBaseWaitDuration + static_cast<float>(scalingCount) * factor;

	// 上限値を設定する
	if (waitDuration_ > ShadowObject::kMaxWaitDuration) {
		waitDuration_ = ShadowObject::kMaxWaitDuration;
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
