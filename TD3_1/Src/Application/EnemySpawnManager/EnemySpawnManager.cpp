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

	selectedWaveIndex_ = -1;

#endif

    currentWaveIndex_ = 0; // 現在のウェーブのインデックス


	// 敵スポーンデータをロード
	LoadFromFile();
}

void EnemySpawnManager::Update() {
	if (isTimerActive_) {
		if (kDeltaTime == 0.0f)
			elapsedTime_ += 1.0f / 60.0f;
		else
			elapsedTime_ += kDeltaTime;
	}

	for (auto& data : nSpawnData_)
	{
        // ウェーブの開始時間が経過している場合、ウェーブをアクティブにする
        if (elapsedTime_ >= data.startTime) {
            data.isActive = true;
            currentWaveIndex_ = data.waveNumber; // 現在のウェーブインデックスを更新
        }
	}

    auto& wave = nSpawnData_[currentWaveIndex_];

	if (wave.isActive) {
		// ウェーブがアクティブな場合、敵のスポーンを行う
		for (auto& group : wave.enemyGroups) {
            // グループのスポーン時間が経過している場合、敵をスポーン
            float waveElapsedTime = elapsedTime_ - wave.startTime; // ウェーブ開始からの経過時間 ウェーブの経過時間
            if (waveElapsedTime >= group.spawnTime) {
                float groupActiveTime = waveElapsedTime - group.spawnTime; // グループの経過時間
				for (auto i = 0; i < group.spawnData.size(); ++i) {
                    // スポーンデータを取得
                    auto& spawnData = group.spawnData[i];
                    // 敵の種類に応じてスポーン
					if (spawnData.delayTime <= groupActiveTime && !spawnData.spawned)
					{
						if (spawnData.enemyType == "normal"|| spawnData.enemyType == "Normal") { // ノーマル敵の場合
							auto enemy = std::make_unique<NormalEnemy>();
							Vector3 spawnPos = group.spawnPosition + spawnData.spawnOffset;
							enemy->Initialize(spawnPos);
							enemy->SetTarget(towerPositon_); // タワーをターゲットに設定

							enemies_.push_back(std::move(enemy));
						}
						else if (spawnData.enemyType == "Empty")
							continue;

						spawnData.spawned = true; // スポーン済みとしてマーク
					}
                }
            }
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
		for (auto& data : nSpawnData_) {
			data.isActive = false;
			for (auto& group : data.enemyGroups) {
				for (auto& spawnData : group.spawnData) {
					spawnData.spawned = false;
				}
			}
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
	//DrawSpawnEditor();
    nDrawSpawnEditor();

	// デバッグ用スポナーの描画
	for (auto& spawner : debugSpawners_) {
		spawner->Draw(camera, {1, 1, 1, 1});
	}
	#endif
}


void EnemySpawnManager::nDrawSpawnEditor()
{
#ifdef _DEBUG

    ImGui::Begin("Enemy Spawn Editor");

	if (ImGui::Button("Save"))
	{
        SaveToFile();
	}

	// WAVE群の表示

    size_t waveCount = nSpawnData_.size();

    ImGui::BeginChild("WaveList", ImVec2(200, 0), true);
    // WAVEの選択
    ImGui::Text("Wave List");
    ImGui::Separator();
	for (int i = 0; i < waveCount; ++i)
	{
		if (ImGui::Selectable(("Wave_" + std::to_string(i)).c_str(), selectedWaveIndex_ == i))
        {
            selectedWaveIndex_ = i;
			selectedGroup_ = nSpawnData_[i].enemyGroups.end();
        }
	}
    // Waveの追加・削除
	if (ImGui::Button("+ Add Wave"))
	{
		SpawnWave newWave;
		newWave.waveNumber = static_cast<int>(nSpawnData_.size());
        nSpawnData_.push_back(newWave);

		selectedWaveIndex_ = newWave.waveNumber;
		selectedGroup_ = nSpawnData_.back().enemyGroups.end();
	}
	ImGui::SameLine();
    if (ImGui::Button("- Remove Wave"))
    {
        if (selectedWaveIndex_ < nSpawnData_.size())
        {
            nSpawnData_.erase(nSpawnData_.begin() + selectedWaveIndex_);
			if(!nSpawnData_.empty())
			{
				selectedWaveIndex_ = (std::min)(selectedWaveIndex_, static_cast<int>(nSpawnData_.size()) - 1);
				selectedGroup_ = nSpawnData_[selectedWaveIndex_].enemyGroups.end();

			}
			else
			{
				selectedWaveIndex_ = -1;
			}
		}
    }

    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("DetailsPanel", ImVec2(0, 0), true);


	if (selectedWaveIndex_ >= 0 && selectedWaveIndex_ < nSpawnData_.size())
	{
		// 選択されたWAVEの詳細を表示
		// Waveの編集
		auto& wave = nSpawnData_[selectedWaveIndex_];
		ImGui::SeparatorText("Wave Details");
		{
			//ImGui::InputInt("Wave Number", &wave.waveNumber);
			ImGui::Checkbox("Is Active", &wave.isActive);
			ImGui::DragFloat("Start Time", &wave.startTime, 0.01f, 0.0f, 10000000.0f);

			int groupCount = static_cast<int>(wave.enemyGroups.size());

			// スポーンgroup群の表示
			// スポーンgroupの選択
			if (ImGui::BeginTable("EnemyGroupTable", 3, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_Borders))
			{
				for (int i = 0; i < groupCount; i++)
				{
					ImGui::PushID(&wave.enemyGroups[i]);
					ImGui::TableNextColumn();
					if (ImGui::Selectable(wave.enemyGroups[i].groupName.c_str(), selectedGroup_ == wave.enemyGroups.begin() + i))
					{
						selectedGroup_ = wave.enemyGroups.begin() + i;
						selectedEnemy_ = selectedGroup_->spawnData.end();
					}
					ImGui::PopID();
				}
				ImGui::EndTable();
			}

			// スポーンgroupの追加・削除
			if (ImGui::Button("+ Add Group"))
			{
				EnemySpawnGroup newGroup;
				newGroup.spawnPosition = kLeftSpawnPos_;
				wave.enemyGroups.push_back(newGroup);
				selectedGroup_ = wave.enemyGroups.end() - 1;
				selectedEnemy_ = selectedGroup_->spawnData.end();
			}
			ImGui::SameLine();
			if (ImGui::Button("- Remove Group"))
			{
				if (selectedGroup_ != wave.enemyGroups.end())
				{
					wave.enemyGroups.erase(selectedGroup_);
					if (!wave.enemyGroups.empty())
					{
						selectedGroup_ = wave.enemyGroups.end() - 1;
						selectedEnemy_ = selectedGroup_->spawnData.end();
					}
					else
					{
						selectedGroup_ = wave.enemyGroups.end();
					}
				}
			}
		}

		// スポーンgroupの表示
		if (selectedGroup_ != nSpawnData_[selectedWaveIndex_].enemyGroups.end())
		{
			auto& group = *selectedGroup_;

			ImGui::SeparatorText("Group Details");

			group.enemyCount;

			group.groupName;
			char nameBuffer[128];
			// GroupNameの編集・表示
			strcpy_s(nameBuffer, group.groupName.c_str());
			if (ImGui::InputText("Group Name", nameBuffer, sizeof(nameBuffer)))
			{
				group.groupName = nameBuffer;
			}

			// Groupのスポーン位置
			group.spawnPosition;

			static const char* spawnPositionNames[] = { "Left", "Top", "Right" };
			static int selectedSpawnPositionIndex = 0;
			static Vector3 spawnPositions[] = { kLeftSpawnPos_, kTopSpawnPos_, kRightSpawnPos_ };
			if (ImGui::Combo("Spawn Position", &selectedSpawnPositionIndex, spawnPositionNames, IM_ARRAYSIZE(spawnPositionNames))) {
				group.spawnPosition = spawnPositions[selectedSpawnPositionIndex];
			}

			// Groupのスポーン時間
			group.spawnTime;
			ImGui::DragFloat("Spawn Time", &group.spawnTime, 0.01f);

			// Groupのスポーンデータの表示
			// スポーンenemy群の表示
			group.spawnData;
			if (ImGui::BeginTable("EnemySpawnDataTable", 3, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_Borders))
			{
				for (int i = 0; i < group.spawnData.size(); i++)
				{
					ImGui::PushID(&group.spawnData[i]);
					ImGui::TableNextColumn();
					if (ImGui::Selectable(group.spawnData[i].enemyType.c_str(), selectedEnemy_ == group.spawnData.begin() + i))
					{
						selectedEnemy_ = group.spawnData.begin() + i;
					}
					ImGui::PopID();
				}
				ImGui::EndTable();
			}

			// スポーンenemyの追加・削除
			if (ImGui::Button("+ Add Enemy"))
			{
				EnemySpawnData newEnemy;
				group.spawnData.push_back(newEnemy);
				selectedEnemy_ = group.spawnData.end();
			}
			ImGui::SameLine();
			if (!group.spawnData.empty())
			{
				if (ImGui::Button("- Remove Enemy"))
				{
					if (selectedEnemy_ != group.spawnData.end())
					{
						group.spawnData.erase(selectedEnemy_);
						if (!group.spawnData.empty())
							selectedEnemy_ = group.spawnData.end() - 1;
						else
							selectedEnemy_ = group.spawnData.end();
					}
				}
			}

			// スポーンenemyの表示
			if(!group.spawnData.empty())
			{
				if (selectedEnemy_ != group.spawnData.end())
				{
					auto& enemy = *selectedEnemy_;
					ImGui::SeparatorText("Enemy Details");
					// 敵の種類
					static const char* enemyTypes[] = { "Normal" , "Empty" };
					static int selectedEnemyTypeIndex = 0;
					if (ImGui::Combo("Type", &selectedEnemyTypeIndex, enemyTypes, IM_ARRAYSIZE(enemyTypes))) {
						enemy.enemyType = enemyTypes[selectedEnemyTypeIndex];
					}
					// 敵のスポーン位置
					ImGui::DragFloat3("Spawn Offset", &enemy.spawnOffset.x, 0.01f);
					// 敵のスポーン時間
					ImGui::DragFloat("Delay Time", &enemy.delayTime, 0.01f);
				}
			}
		}
	}


	ImGui::EndChild();

	ImGui::End();

#endif // _DEBUG
}

void EnemySpawnManager::SaveToFile()
{
	const std::string filePath = "Resources/Data/EnemySpawn/spawn_data.json";

	SortSpawnData();

	nlohmann::json j;

	for (const auto& spawnData : nSpawnData_)
	{
        nlohmann::json waveJson;
        waveJson["wave_number"] = spawnData.waveNumber;
        waveJson["start_time"] = spawnData.startTime;
        for (const auto& group : spawnData.enemyGroups)
        {
            nlohmann::json groupJson;
            groupJson["group_name"] = group.groupName;
            groupJson["enemy_count"] = group.enemyCount;
            groupJson["spawn_time"] = group.spawnTime;
            groupJson["spawn_position"] = { group.spawnPosition.x, group.spawnPosition.y, group.spawnPosition.z };
            for (const auto& enemy : group.spawnData)
            {
                nlohmann::json enemyJson;
                enemyJson["enemy_type"] = enemy.enemyType;
                enemyJson["spawn_offset"] = { enemy.spawnOffset.x, enemy.spawnOffset.y, enemy.spawnOffset.z };
                enemyJson["delay_time"] = enemy.delayTime;
                groupJson["spawn_data"].push_back(enemyJson);
            }
            waveJson["enemy_groups"].push_back(groupJson);
        }
        j["waves"].push_back(waveJson);
	}

    std::ofstream file2(filePath);
    file2 << j.dump(4);
    file2.close();

}

void EnemySpawnManager::LoadFromFile()
{
	const std::string filePath = "Resources/Data/EnemySpawn/spawn_data.json";

	std::ifstream file(filePath);
	nlohmann::json jsonData;
	file >> jsonData;

	spawnData_.clear();
	for (const auto& entry : jsonData["waves"]) {
		SpawnWave wave;
		if (entry.contains("wave_number")) wave.waveNumber = entry["wave_number"];
        if (entry.contains("start_time"))        wave.startTime = entry["start_time"];

		if (entry.contains("enemy_groups")) {
			for (const auto& groupEntry : entry["enemy_groups"]) {
				EnemySpawnGroup group;
                if (groupEntry.contains("group_name")) group.groupName = groupEntry["group_name"];
                if (groupEntry.contains("enemy_count")) group.enemyCount = groupEntry["enemy_count"];
                if (groupEntry.contains("spawn_time")) group.spawnTime = groupEntry["spawn_time"];
                if (groupEntry.contains("spawn_position")) {
                    group.spawnPosition = { groupEntry["spawn_position"][0], groupEntry["spawn_position"][1], groupEntry["spawn_position"][2] };
					if (group.spawnPosition == Vector3{ 0,0,0 }) group.spawnPosition = kLeftSpawnPos_;
                }
                if (groupEntry.contains("spawn_data")) {
                    for (const auto& enemyEntry : groupEntry["spawn_data"]) {
                        EnemySpawnData enemy;
                        if (enemyEntry.contains("enemy_type")) enemy.enemyType = enemyEntry["enemy_type"];
                        if (enemyEntry.contains("spawn_offset")) {
                            enemy.spawnOffset = { enemyEntry["spawn_offset"][0], enemyEntry["spawn_offset"][1], enemyEntry["spawn_offset"][2] };
                        }
                        if (enemyEntry.contains("delay_time")) enemy.delayTime = enemyEntry["delay_time"];

                        group.spawnData.push_back(enemy);
                    }
                }
				wave.enemyGroups.push_back(group);
			}
		}
		nSpawnData_.push_back(wave);
	}

    SortSpawnData();

}

void EnemySpawnManager::SortSpawnData()
{
    // nSpawnData_をspawnTimeでソート
    std::sort(nSpawnData_.begin(), nSpawnData_.end(), [](const SpawnWave& a, const SpawnWave& b) {
        return a.startTime < b.startTime;
        });

    // wavenumberを振り直す
    for (size_t i = 0; i < nSpawnData_.size(); ++i) {
        nSpawnData_[i].waveNumber = static_cast<int>(i);
    }
}
