#include "EnemySpawnManager.h"

// C++
#include <fstream>
#include <unordered_set>

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

	// 敵スポーンデータをロード
	LoadFromFile();
}

void EnemySpawnManager::Update() {
	if (isTimerActive_) {
		elapsedTime_ += kDeltaTime;
	}

	// 敵のスポーン
	for (auto& data : spawnData_) {
		// 経過時間が出現時間に達しており、まだスポーンしていない場合
		if (elapsedTime_ >= data.spawnTime && !data.spawned) {
			// 敵を生成
			if (data.type == "Normal") { // タイプがノーマルの場合
				auto enemy = std::make_unique<NormalEnemy>();
				enemy->Initialize(data.position);
				enemy->SetTarget(towerPositon_); // タワーをターゲットに設定
				enemies_.push_back(std::move(enemy));

			} else if (data.type == "Empty") { // タイプを追加した場合ここに記述
				continue;
			}
			// スポーン済みとしてマーク
			data.spawned = true;
		}
	}

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
	ImGui::Checkbox("elapsedTime :", &isTimerActive_);
	ImGui::SameLine();
	ImGui::Text("%.fs", elapsedTime_);
	// リセットを行う（仮なので後で整理）
	if (ImGui::Button("Reset")) {
		elapsedTime_ = 0.0f;
		// 敵の出現済みフラグをリセット
		for (auto& data : spawnData_) {
			data.spawned = false;
		}
		// 出現してる敵を全て死亡させる
		for (auto& enemy : enemies_) {
			enemy->Dead();
		}
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
	DrawSpawnEditor();

	// デバッグ用スポナーの描画
	for (auto& spawner : debugSpawners_) {
		spawner->Draw(camera, {1, 1, 1, 1});
	}
	#endif
}

void EnemySpawnManager::DrawSpawnEditor()
{
	///
	///	データの追加・保存
	/// 
	const std::string filePath = "Resources/Data/EnemySpawn/spawn_data.json";

	ImGui::Begin("Enemy Spawn Editor");
	
	// 種類を設定
	static const char* enemyTypes[] = { "Normal" , "Empty" };
	static int selectedEnemyTypeIndex = 0;
	if (ImGui::Combo("Type", &selectedEnemyTypeIndex, enemyTypes, IM_ARRAYSIZE(enemyTypes))) {}
	// 出現位置を設定
	static const char* spawnPositionNames[] = { "Left", "Top", "Right" };
	static int selectedSpawnPositionIndex = 0;
	static Vector3 spawnPositions[] = { kLeftSpawnPos_, kTopSpawnPos_, kRightSpawnPos_ };
	if (ImGui::Combo("Spawn Position", &selectedSpawnPositionIndex, spawnPositionNames, IM_ARRAYSIZE(spawnPositionNames))) {}
	// 出現時間を設定（Intで入力->floatに変換）
	static int spawnTimeInt = 0;
	static float spawnTime = 0.0f;
	ImGui::InputInt("Spawn Time (seconds)", &spawnTimeInt);
	spawnTime = static_cast<float>(spawnTimeInt);

	// 出現データの配列にデータを追加
	if (ImGui::Button("Add Spawn Data")) {
		SpawnData newSpawnData;
		newSpawnData.type = enemyTypes[selectedEnemyTypeIndex];
		newSpawnData.position = spawnPositions[selectedSpawnPositionIndex];
		newSpawnData.spawnTime = spawnTime;
		spawnData_.push_back(newSpawnData);

		// 追加した瞬間にJSONを更新
		SaveToFile();
	}

	ImGui::End();




	///
	///	スポーンデータの表示
	/// 
	
	ImGui::Begin("Spawn Data Viewer");

	if (spawnData_.empty()) {
		ImGui::Text("No spawn data available.");
	} else {
		// SpawnTimeが小さい順にソートする
		std::sort(spawnData_.begin(), spawnData_.end(), [](const SpawnData& a, const SpawnData& b) {
			return a.spawnTime < b.spawnTime;
			});

		ImGui::Columns(5, "SpawnDataColumns"); // 5列のレイアウトを作成
		ImGui::Text("Index"); ImGui::NextColumn();
		ImGui::Text("Type"); ImGui::NextColumn();
		ImGui::Text("Spawn Position"); ImGui::NextColumn();
		ImGui::Text("Spawn Time"); ImGui::NextColumn();
		ImGui::Text("Actions"); ImGui::NextColumn();
		ImGui::Separator();

		uint32_t index = 0;
		for (size_t i = 0; i < spawnData_.size(); ++i) {
			const auto& data = spawnData_[i];

			// Index
			ImGui::Text("%d", index++); ImGui::NextColumn();
			// Type
			ImGui::Text("%s", data.type.c_str()); ImGui::NextColumn();
			// Spawn Position
			const char* positionName = nullptr;
			if (data.position == kLeftSpawnPos_) {
				positionName = "Left";
			} else if (data.position == kTopSpawnPos_) {
				positionName = "Top";
			} else if (data.position == kRightSpawnPos_) {
				positionName = "Right";
			}
			ImGui::Text("%s", positionName); ImGui::NextColumn();
			// Spawn Time
			ImGui::Text("%.fs", data.spawnTime); ImGui::NextColumn();
			// Delete Button
			if (ImGui::Button(("Delete##" + std::to_string(i)).c_str())) {
				spawnData_.erase(spawnData_.begin() + i); // データの削除
				--i; // イテレーターを調整

				// 削除した瞬間にJSONを更新
				SaveToFile();
			} ImGui::NextColumn();
		}
	}

	ImGui::Columns(1); // 列をリセット
	ImGui::End();
}

void EnemySpawnManager::SaveToFile()
{
	const std::string filePath = "Resources/Data/EnemySpawn/spawn_data.json";

	nlohmann::json jsonData;

	for (const auto& spawnData : spawnData_) {
		jsonData.push_back({
			{"type", spawnData.type},
			{"spawn_position", {spawnData.position.x, spawnData.position.y, spawnData.position.z}},
			{"spawn_time", spawnData.spawnTime},
			});
	}

	std::ofstream file(filePath);
	file << jsonData.dump(4);
}

void EnemySpawnManager::LoadFromFile()
{
	const std::string filePath = "Resources/Data/EnemySpawn/spawn_data.json";

	std::ifstream file(filePath);
	nlohmann::json jsonData;
	file >> jsonData;

	spawnData_.clear();
	for (const auto& entry : jsonData) {
		SpawnData data;
		data.type = entry["type"];
		data.position = { entry["spawn_position"][0], entry["spawn_position"][1], entry["spawn_position"][2], };
		data.spawnTime = entry["spawn_time"];
		spawnData_.push_back(data);
	}
}
