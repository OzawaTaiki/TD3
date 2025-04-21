#include "GameScene.h"

// Engine
#include <Features/Model/Manager/ModelManager.h>
#include <Features/Scene/Manager/SceneManager.h>
#include <Math/Random/RandomGenerator.h>

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

	particleSystem_ = ParticleSystem::GetInstance();
    particleSystem_->SetCamera(&SceneCamera_);

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
	movableObjectManager_->Initialize(SceneCamera_);

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


    rewardGauge_ = std::make_unique<RewardGauge>();
    rewardGauge_->Initialize();

	gameUI_ = std::make_unique<GameUI>();
	gameUI_->Initialize();

	// カメラシェイク初期化
	CameraShake::GetInstance()->Initialize();

	clearChecker_ = std::make_unique<ClearChecker>();

	// フェード初期化
	fade_ = std::make_unique<Fade>();
	fade_->Initialize();
	fade_->Start(Fade::Status::FadeIn, 1.0f);

	LoadFromFile();

	// テストパーティクル
	testParticle_ = std::make_unique<TestParticle>();
	testParticle_->Initialize();
}

void GameScene::Update() {
#ifdef _DEBUG

	if (input_->IsKeyTriggered(DIK_RETURN) && input_->IsKeyPressed(DIK_RSHIFT))
		enableDebugCamera_ = !enableDebugCamera_;

	ImGui::Begin("GameSceneInfo");
	ImGui::Text("fps : %.2f", ImGui::GetIO().Framerate);
	ImGui::Text("count : %d", shadowObjectManager_->GetScalingShadowObjectsCount());
	if (ImGui::Button("Emit")) {
		InitParticles();
	}

	if (ImGui::Button("Emit TestParticle")) {
		testParticle_->Emit();
	}
	ImGui::End();

	ImGui::Begin("Camera");
	ImGui::DragFloat3("translate", &originalCameraTranslate_.x, 0.01f);
	ImGui::DragFloat3("rotate", &SceneCamera_.rotate_.x, 0.01f);
	if (ImGui::Button("Save")) {
		SaveToFile();
	}

	ImGui::End();

#endif // _DEBUG

	ground_->Update();

	if (enableDebugCamera_) {
		debugCamera_.Update();
		SceneCamera_.matView_ = debugCamera_.matView_;
		SceneCamera_.TransferData();
	} else {
		SceneCamera_.Update();
		SceneCamera_.UpdateMatrix();
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
	shadowObjectManager_->Update(
		movableObjectManager_->GetAllObjects(),
		pointLightObjectManager_->GetLights(),
		movableObjectManager_->IsDragging()
	);
	// UI更新
	gameUI_->Update();

    rewardGauge_->Update();

	particleSystem_->Update();
	CollisionManager::GetInstance()->Update();


	if (input_->IsKeyTriggered(DIK_T) &&
		input_->IsKeyPressed(DIK_RSHIFT))
	{
		SceneManager::GetInstance()->ReserveScene("Title");
	}

	// フェード更新
	fade_->Update();
	switch (phase_) {
	case Phase::kFadeIn:
		if (fade_->IsFinished()) {
			phase_ = Phase::kMain;
		}

		break;

	case Phase::kMain:
		// タワーのHPが0になったら
        // 敵が全て倒されたら
		// フェード開始
		if (tower_->GetHP() <= 0 ||
			clearChecker_->IsCleared()) {
			phase_ = Phase::kFadeOut;
			fade_->Start(Fade::Status::FadeOut, 1.0f);
		}

		break;

	case Phase::kFadeOut:
		// 一旦タイトルに戻るようにしておく
		if (fade_->IsFinished()) {
			SceneManager::GetInstance()->ReserveScene("Title");
		}
	}

	// テストパーティクル
	testParticle_->Update();
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

	///
	///	スプライト描画
	///

	// UI描画
	Sprite::PreDraw();
	rewardGauge_->Draw();
	gameUI_->Draw();
	tower_->DrawUI(SceneCamera_);

	// フェード描画
	fade_->Draw();

	particleSystem_->DrawParticles();
}

void GameScene::DrawShadow() {}

void GameScene::SaveToFile() {
	const std::string filePath = "Resources/Data/Camera/cameraParam.json";
	nlohmann::json jsonData;

	jsonData.push_back({
	    {"originalCameraTranslate", {originalCameraTranslate_.x, originalCameraTranslate_.y, originalCameraTranslate_.z}},
	    {"originalCameraRotate",    {SceneCamera_.rotate_.x, SceneCamera_.rotate_.y, SceneCamera_.rotate_.z}            }
    });

	std::ofstream file(filePath);
	file << jsonData.dump(4);
}

void GameScene::LoadFromFile() {
	const std::string filePath = "Resources/Data/Camera/cameraParam.json";

	std::ifstream file(filePath);
	nlohmann::json jsonData;
	file >> jsonData;

	originalCameraTranslate_ = {
		jsonData[0]["originalCameraTranslate"][0].get<float>(),
		jsonData[0]["originalCameraTranslate"][1].get<float>(),
		jsonData[0]["originalCameraTranslate"][2].get<float>()
	};
	SceneCamera_.rotate_ = {
	    jsonData[0]["originalCameraRotate"][0].get<float>(),
		jsonData[0]["originalCameraRotate"][1].get<float>(),
		jsonData[0]["originalCameraRotate"][2].get<float>()
	};
}

void GameScene::InitParticles() {
	const uint32_t emitCount = 10;

	auto rand = RandomGenerator::GetInstance();

	std::vector<Particle*> particles(emitCount);

	for (uint32_t i = 0; i < emitCount; ++i) {
		// 初期化データ
		ParticleInitParam initParam;

		// ビルボードの有無(正常に動くか微妙)
		initParam.isBillboard = {false, false, false};

		// 初期カラー
		initParam.color;
		Vector3 rgb = rand->GetRandValue(Vector3{1, 1, 1}, Vector3{0, 0, 0});
		float alpha = 1.0f;

		initParam.color = Vector4(rgb.x, rgb.y, rgb.z, alpha);

		// 初期方向
		// 上方向
		initParam.direction = Vector3(0, 1, 0);

		// lifiTimeを持つか 基本false
		initParam.isInfiniteLife = false;

		initParam.lifeTime;
		if (!initParam.isInfiniteLife) {
			initParam.lifeTime = rand->GetRandValue(0.5f, 1.0f);
		}

		// 初期位置 親となるようなTransformがあればそれを基準にする
		initParam.position = {4, 4, 0};

		// 回転
		initParam.rotate = {0, 0, 0};

		// 初期サイズ(モデルのスケール)
		initParam.size = {1, 1, 1};

		// 初期速度
		initParam.speed = rand->GetRandValue(1.0f, 3.0f);

		Particle* particle = new Particle();
		particle->Initialize(initParam);

		particles[i] = particle;
	}

	// 描画簡易設定
	ParticleRenderSettings settings;
	settings.blendMode = BlendMode::Add; // ブレンドモード
	settings.cullBack = false;           // 背面カリングの有無 ここではなし

	// 使用するモデル名
	// ※必ずしもモデルのパスではない ModelManagerでキーとして持っている名前(たいていはpathで管理している。)
	std::string useModelName = "Hand/hand.obj";

	uint32_t textureHandle = 0;

	// ParticleModifierFactory に登録されているモディファイアの名前を指定する
	// 新しく実装したら ParticleModifierFactory に追記すること。
	std::vector<std::string> modifiers;
	modifiers.push_back("DecelerationModifier"); // 減速Modifier

	ParticleSystem::GetInstance()->AddParticles(useModelName, particles, settings, textureHandle, modifiers);
}
