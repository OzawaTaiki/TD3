#pragma once

// C++
#include <vector>

// Application
#include <Application/ShadowObject/ShadowObject.h>

class PointLightObject;
class MovableObject;

class ShadowObjectManager
{
public:
	void Initialize();
	void Update(const std::vector<std::unique_ptr<MovableObject>>& movableObjects, const std::vector<std::unique_ptr<PointLightObject>>& pointLightObjects);
	void Draw(const Camera& camera);

	/// <summary>
	/// 各グループの全ての影オブジェクトを探索し、実体化中の数をカウント（実体化の持続時間変更に使用）
	/// </summary>
	size_t GetScalingShadowObjectsCount() const;

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
	
	void UpdateWaitDuration();


    uint32_t materializationSoundHandle_ = 0; // 影オブジェクトの実体化時のサウンドハンドル
    float materializationSoundVolume_ = 0.5f; // デフォルトのボリュームを設定

	void SaveToFile();
	void LoadFromFile();
};