#include "GameScene.h"

#include <Features/Model/Manager/ModelManager.h>

GameScene::~GameScene() {
	
}

void GameScene::Initialize() {
	SceneCamera_.Initialize();
	SceneCamera_.translate_ = {0, 50, -50};
	SceneCamera_.rotate_ = {0.81f, 0, 0};
	SceneCamera_.UpdateMatrix();
	debugCamera_.Initialize();

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
	LightingSystem::GetInstance()->SetLightGroup(lights_.get());


	///
	///	Application
	/// 

	// フィールドの生成と初期化
	field_ = std::make_unique<Field>();
	field_->Initialize("mapData.csv");

	// 動かせるオブジェクトを管理するクラスの生成と初期化
	movableObjectManager_ = std::make_unique<MovableObjectManager>();
	movableObjectManager_->Initialize();

	// タワーの生成と初期化
	tower_ = std::make_unique<Tower>();
	tower_->Initialize({ 0, 1, -2 });
}

void GameScene::Update() {
#ifdef _DEBUG

	if (input_->IsKeyTriggered(DIK_RETURN) && input_->IsKeyPressed(DIK_RSHIFT))
		enableDebugCamera_ = !enableDebugCamera_;

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

	// フィールド更新
	field_->Update();
	// 動かせるオブジェクト更新
	movableObjectManager_->Update(SceneCamera_);
	// タワー更新
	tower_->Update();

	CollisionManager::GetInstance()->Update();
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
}

void GameScene::DrawShadow() {}
