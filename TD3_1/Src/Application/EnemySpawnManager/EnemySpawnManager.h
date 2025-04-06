#pragma once

// Engine
#include <Features/Model/ObjectModel.h>

// Application
#include <Application/Enemy/Enemy.h>

// Externals
#include <json.hpp>

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
	/// 敵スポーンの情報
	/// </summary>
	struct SpawnData {
		bool spawned = false; // スポーン済みかどうか
		std::string type; // 種類
		float spawnTime; // 出現時間（秒）
		Vector3 position; // 出現位置
		/*新しいパラメーターはここから追加*/
	};

	/// <summary>
	/// 敵の出現データを格納
	/// </summary>
	std::vector<SpawnData> spawnData_;

    // スポーンする敵毎のデータ
	struct EnemySpawnData {
		Vector3 spawnOffset = { 0,0,0 }; // スポーン位置のオフセット
        std::string enemyType="Normal"; // 敵の種類 (どのオブジェクトを攻撃するか
        float delayTime = 0.0f; // スポーンまでの遅延時間 グループ内最初のスポーンからの時間
        bool spawned = false; // スポーン済みかどうか

        //std::string targetId; // 攻撃対象の塔のID的なもの
	};
    // 敵のスポーングループ
    struct EnemySpawnGroup {
		std::string groupName = "group"; // グループ名
		int enemyCount = 0; // 敵の数
		float spawnTime = 1.0f; // スポーン時間 wave基準の時間
		Vector3 spawnPosition = { 0,0,0 }; // スポーン位置
		std::vector<EnemySpawnData> spawnData = {}; // スポーンデータ
    };

	struct SpawnWave {
		int waveNumber = 0; // ウェーブ番号
		float startTime = 0; // ウェーブの開始時間
		std::vector<EnemySpawnGroup> enemyGroups = {}; // 敵のスポーングループ
        bool isActive=false; // ウェーブがアクティブかどうか
	};

    std::vector<EnemySpawnData>::iterator selectedEnemy_; // 選択された敵
    std::vector<EnemySpawnGroup>::iterator selectedGroup_; // 選択されたグループ
    int selectedWaveIndex_ = 0; // 選択されたウェーブのインデックス

	std::vector<SpawnWave> nSpawnData_; // スポーンデータ

    int currentWaveIndex_ = 0; // 現在のウェーブのインデックス

	/// <summary>
	/// 敵のスポーン位置
	/// </summary>
	const Vector3 kLeftSpawnPos_ = {-26, 1, -4};
	const Vector3 kTopSpawnPos_ = {1, 1, 22};
	const Vector3 kRightSpawnPos_ = {26, 1, 0};

	/// <summary>
	/// スポーンエディタの表示
	/// </summary>
	void DrawSpawnEditor();

	void nDrawSpawnEditor();

	void SaveToFile(); // JSONファイルへ保存
	void LoadFromFile(); // JSONファイルから読み込み

    void SortSpawnData(); // スポーンデータのソート

	// 経過時間（仮なのであとで整理）
	float elapsedTime_ = 0;
	const float kDeltaTime = 1.0f / 60.0f;
	bool isTimerActive_ = false;

	#ifdef _DEBUG
	// スポーン位置表示用のオブジェクト
	std::vector<std::unique_ptr<ObjectModel>> debugSpawners_;
	#endif

	/// <summary>
	/// 各ターゲット位置を格納
	/// </summary>

	// タワー位置（今は1つのみ）
	Vector3 towerPositon_;
};
