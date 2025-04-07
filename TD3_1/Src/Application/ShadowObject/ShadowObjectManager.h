#pragma once

// C++
#include <vector>

// Application
#include <Application/ShadowObject/ShadowObject.h>

class PointLightObject;

class ShadowObjectManager
{
public:
	void Initialize();
	void Update(const std::vector<Vector3>& movableObjects, const std::vector<std::unique_ptr<PointLightObject>>& pointLightObjects);
	void Draw(const Camera& camera);

private:
	struct ShadowGroup {
		Vector3 movableObjectPosition_;
		std::vector<std::unique_ptr<ShadowObject>> shadowObjects_;


		ShadowGroup() = default;

		ShadowGroup(const ShadowGroup&) = delete;
		ShadowGroup& operator=(const ShadowGroup&) = delete;

		ShadowGroup(ShadowGroup&&) noexcept = default;
		ShadowGroup& operator=(ShadowGroup&&) noexcept = default;
	};

	// 各 movableObject　に対応する影オブジェクトのグループ
	std::vector<ShadowGroup> shadowGroups_;

	float waitDuration_ = 0.0f; // 最大値まで拡大->縮小を始めるまでの待機時間

	void SaveToFile();
	void LoadFromFile();
};