#include "ResultScene.h"

// Engine
#include <Features/Model/Manager/ModelManager.h>
#include <Features/Scene/Manager/SceneManager.h>


ResultScene::~ResultScene()
{
}

void ResultScene::Initialize()
{

	SceneCamera_.Initialize();
	SceneCamera_.translate_ = { 0,5,-20 };
	SceneCamera_.rotate_ = { 0.26f,0,0 };
	SceneCamera_.UpdateMatrix();
	debugCamera_.Initialize();

	lineDrawer_ = LineDrawer::GetInstance();
	lineDrawer_->Initialize();
	lineDrawer_->SetCameraPtr(&SceneCamera_);

	input_ = Input::GetInstance();

	ground_ = std::make_unique<ObjectModel>("Ground");
	ground_->Initialize("Tile/Tile.gltf");
	ground_->GetUVTransform().SetScale({ 100, 100 });

	particleSystem_ = ParticleSystem::GetInstance();
	particleSystem_->SetCamera(&SceneCamera_);


	lights_ = std::make_shared<LightGroup>();
	lights_->Initialize();

	auto pl = std::make_shared<PointLightComponent>();
	pl->SetIntensity(0.0f);
	lights_->AddPointLight("PointLight", pl);


	LightingSystem::GetInstance()->SetActiveGroup(lights_);

	///
	///	Application
	///

	fade_ = std::make_unique<Fade>();
	fade_->Initialize();
	fade_->Start(Fade::Status::FadeIn, 1.0f);

    resultModels_ = std::make_unique<ResultModels>();
    resultModels_->Initialize();

    //scoreSprites_ = std::make_unique<ScoreSprites>();
    //scoreSprites_->Initialize();

    //otohime_ = std::make_unique<ResultOtohime>();
    //otohime_->Initialize();

}

void ResultScene::Update()
{
	// フェード更新
	fade_->Update();

#ifdef _DEBUG

	if (input_->IsKeyTriggered(DIK_RETURN) && input_->IsKeyPressed(DIK_RSHIFT))
		enableDebugCamera_ = !enableDebugCamera_;

    resultModels_->DebugWindow();

#endif // _DEBUG

	if (enableDebugCamera_) {
		debugCamera_.Update();
		SceneCamera_.matView_ = debugCamera_.matView_;
		SceneCamera_.TransferData();
	}
	else {
		SceneCamera_.Update();
		SceneCamera_.UpdateMatrix();
	}

	ground_->Update();
    resultModels_->Update();
    //scoreSprites_->Update();
    //otohime_->Update();


	switch (phase_) {
	case Phase::kFadeIn:
		if (fade_->IsFinished()) {
			phase_ = Phase::kMain;
		}

		break;
	case Phase::kMain:
		// スペース押下でフェード開始
		if (input_->IsKeyTriggered(DIK_SPACE)) {
			phase_ = Phase::kFadeOut;
			fade_->Start(Fade::Status::FadeOut, 1.0f);
		}

		break;
	case Phase::kFadeOut:
		// フェードが終わったらゲームシーンへ
		if (fade_->IsFinished()) {
			SceneManager::GetInstance()->ReserveScene("Title");
		}

		break;
	}

	particleSystem_->Update();
}

void ResultScene::Draw()
{
	// 背景スプライト
    Sprite::PreDraw();
    //scoreSprites_->Draw();

	ModelManager::GetInstance()->PreDrawForObjectModel();

	resultModels_->Draw(&SceneCamera_);

	//ground_->Draw(&SceneCamera_, { 1, 1, 1, 1 });
    //otohime_->Draw(&SceneCamera_);

	// フェード描画
	fade_->Draw();
}

void ResultScene::DrawShadow()
{
}
