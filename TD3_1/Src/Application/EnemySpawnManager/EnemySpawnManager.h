#pragma once

// Engine
#include <Features/Model/ObjectModel.h>

// Application
#include <Application/Enemy/Enemy.h>

class Camera;

class EnemySpawnManager {
public:
	void Initialize();
	void Update();
	void Draw(const Camera* camera);

	/// <summary>
	/// ターゲットの位置をセット
	/// </summary>
	// タワーの位置をセット
	void SetTowerPosition(const Vector3& towerPosition) { towerPositon_ = towerPosition; }

private:
	/// <summary>
	/// 全ての敵を格納
	/// </summary>
	std::vector<std::unique_ptr<Enemy>> enemies_;

	/// <summary>
	/// 敵のスポーン位置
	/// </summary>
	const Vector3 kLeftSpawnPos_ = {-26, 1, -4};
	const Vector3 kTopSpawnPos_ = {1, 1, 22};
	const Vector3 kRightSpawnPos_ = {28, 1, 0};

	#ifdef _DEBUG
	std::vector<std::unique_ptr<ObjectModel>> debugSpawners_;
	#endif

	/// <summary>
	/// 各ターゲット位置を格納
	/// </summary>
	
	// タワー位置（今は1つのみ）
	Vector3 towerPositon_;
};
