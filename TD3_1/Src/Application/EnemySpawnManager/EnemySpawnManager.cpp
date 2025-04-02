#include "EnemySpawnManager.h"

// Externals
#include "imgui.h"

void EnemySpawnManager::Initialize() {
#ifdef _DEBUG
	// スポーン位置に対応するデバッグスポナーを生成
	const std::vector<Vector3> spawnPositions = {kLeftSpawnPos_, kTopSpawnPos_, kRightSpawnPos_};
	for (const auto& pos : spawnPositions) {
		auto spawner = std::make_unique<ObjectModel>("EnemySpawner");
		spawner->Initialize("Cube/cube.obj");
		spawner->translate_ = pos;
		debugSpawners_.push_back(std::move(spawner));
	}
#endif
}

void EnemySpawnManager::Update() {
	// 敵全ての更新
	for (auto& enemy : enemies_) {
		enemy->Update();
	}
	// 死亡した敵を削除
	enemies_.erase(std::remove_if(enemies_.begin(), enemies_.end(), 
		[](const std::unique_ptr<Enemy>& enemy) 
		{ return enemy->IsDead(); 
		}), 
		enemies_.end());

	#ifdef _DEBUG

	ImGui::Begin("enemySpawnManager");
	if (ImGui::Button("Top-Spawn")) {
		auto enemy = std::make_unique<NormalEnemy>();
		enemy->Initialize(this->kTopSpawnPos_); // 上位置からスポーン
		enemy->SetTarget(towerPositon_); // ターゲットにタワーを設定
		enemies_.push_back(std::move(enemy));
	}
	ImGui::End();

	// デバッグ用スポナーの更新
	for (auto& spawner : debugSpawners_) {
		spawner->Update();
	}

	#endif
}

void EnemySpawnManager::Draw(const Camera* camera) {
	// 敵全ての描画
	for (auto& enemy : enemies_) {
		enemy->Draw(camera);
	}

	#ifdef _DEBUG
	// デバッグ用スポナーの描画
	for (auto& spawner : debugSpawners_) {
		spawner->Draw(camera, {1, 1, 1, 1});
	}
	#endif
}
