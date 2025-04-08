#include "GameScene.h"

// Engine
#include <Features/Model/Manager/ModelManager.h>
#include <Features/Scene/Manager/SceneManager.h>

// Application
#include <Application/CameraShake/CameraShake.h>

GameScene::~GameScene() {
	
}

void GameScene::Initialize() {
	SceneCamera_.Initialize();
	SceneCamera_.translate_ = {0, 50, -50};
	SceneCamera_.rotate_ = {0.81f, 0, 0};
	SceneCamera_.UpdateMatrix();
	debugCamera_.Initialize();

	originalCameraTranslate_ = SceneCamera_.translate_; // カメラの初期位置を保存

	lineDrawer_ = LineDrawer::GetInstance();
	lineDrawer_->Initialize();
	lineDrawer_->SetCameraPtr(&SceneCamera_);

	input_ = Input::GetInstance();

	ground_ = std::make_unique<ObjectModel>("Ground");
	ground_->Initialize("Tile/Tile.gltf");
	ground_->GetUVTransform().SetScale({100, 100});

	particleManager_ = ParticleManager::GetInstance();

	lights_ = std::make_unique<LightGroup>();
	lights_->Initialize();
	//LightingSystem::GetInstance()->SetActiveGroup(lights_.get());


	///
	///	Application
	/// 

	/*---生成と初期化---*/
	
	// フィールド
	field_ = std::make_unique<Field>();
	field_->Initialize("mapData.csv");

	// 動かせるオブジェクトを管理するクラス
	movableObjectManager_ = std::make_unique<MovableObjectManager>();
	movableObjectManager_->Initialize();

	// タワー
	tower_ = std::make_unique<Tower>();
	tower_->Initialize({ 0, 0, -2 });

	// 敵を管理するクラス
	enemySpawnManager_ = std::make_unique<EnemySpawnManager>();
	enemySpawnManager_->Initialize();
	// ターゲットの位置を設定（動かないオブジェクトはここ、動くオブジェクトは更新）
	enemySpawnManager_->SetTowerPosition(tower_->GetTranslate());

	// ポイントライトオブジェクトを管理するクラス
	pointLightObjectManager_ = std::make_unique<PointLightObjectManager>();
	pointLightObjectManager_->Initialize();

	// 影オブジェクトを管理するクラス
	shadowObjectManager_ = std::make_unique<ShadowObjectManager>();
	shadowObjectManager_->Initialize();

	// UI
	gameUI_ = std::make_unique<GameUI>();
	gameUI_->Initialize();

	// カメラシェイク初期化
	CameraShake::GetInstance()->Initialize(1.0f, 0.5f);
}

void GameScene::Update() {
#ifdef _DEBUG

	if (input_->IsKeyTriggered(DIK_RETURN) && input_->IsKeyPressed(DIK_RSHIFT))
		enableDebugCamera_ = !enableDebugCamera_;

	ImGui::Begin("GameSceneInfo");
	ImGui::Text("fps : %.2f", ImGui::GetIO().Framerate);
	ImGui::End();

#endif // _DEBUG

	ground_->Update();

	if (enableDebugCamera_) {
		debugCamera_.Update();
		SceneCamera_.matView_ = debugCamera_.matView_;
		SceneCamera_.TransferData();
		particleManager_->Update(debugCamera_.rotate_);
	} else {
		SceneCamera_.Update();
		SceneCamera_.UpdateMatrix();
		particleManager_->Update(SceneCamera_.rotate_);
	}

	// カメラシェイク更新
	CameraShake::GetInstance()->Update();
	SceneCamera_.translate_ = originalCameraTranslate_ + CameraShake::GetInstance()->GetOffset();

	// フィールド更新
	field_->Update();
	// 動かせるオブジェクト更新
	movableObjectManager_->Update(SceneCamera_);
	// タワー更新
	tower_->Update();
	// 敵管理クラス更新
	enemySpawnManager_->Update();
	// ポイントライトオブジェクト更新
	pointLightObjectManager_->Update();
	// 影オブジェクト更新
	shadowObjectManager_->Update(movableObjectManager_->GetAllObjectPosition(), pointLightObjectManager_->GetLights());
	// UI更新
	gameUI_->Update();

	CollisionManager::GetInstance()->Update();



	if (input_->IsKeyTriggered(DIK_T)) {
		SceneManager::GetInstance()->ReserveScene("Title");
	}
}

void GameScene::Draw() {
	ModelManager::GetInstance()->PreDrawForObjectModel();

	ground_->Draw(&SceneCamera_, {1, 1, 1, 1});

	// フィールド描画
	field_->Draw(&SceneCamera_, { 1, 1, 1, 1 });
	// 動かせるオブジェクト描画
	movableObjectManager_->Draw(SceneCamera_);
	// タワー描画
	tower_->Draw(SceneCamera_);
	// 敵管理クラス描画
	enemySpawnManager_->Draw(&SceneCamera_);
	// ポイントライトオブジェクト描画
	pointLightObjectManager_->Draw(SceneCamera_);
	// 影オブジェクト描画
	shadowObjectManager_->Draw(SceneCamera_);
	// UI描画
	gameUI_->Draw();
}

void GameScene::DrawShadow() {}
