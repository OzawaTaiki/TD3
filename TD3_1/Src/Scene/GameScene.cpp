#include "GameScene.h"

#include <Features/Model/Manager/ModelManager.h>

GameScene::~GameScene()
{
}

void GameScene::Initialize()
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
    ground_->GetUVTransform().SetScale({ 100,100 });

    particleManager_ = ParticleManager::GetInstance();

    lights_ = std::make_unique<LightGroup>();
    lights_->Initialize();
    LightingSystem::GetInstance()->SetLightGroup(lights_.get());


}

void GameScene::Update()
{
#ifdef _DEBUG

    if (input_->IsKeyTriggered(DIK_RETURN) &&
        input_->IsKeyPressed(DIK_RSHIFT))
        enableDebugCamera_ = !enableDebugCamera_;

#endif // _DEBUG


    ground_->Update();


    if (enableDebugCamera_)
    {
        debugCamera_.Update();
        SceneCamera_.matView_ = debugCamera_.matView_;
        SceneCamera_.TransferData();
        particleManager_->Update(debugCamera_.rotate_);
    }
    else
    {
        SceneCamera_.Update();
        SceneCamera_.UpdateMatrix();
        particleManager_->Update(SceneCamera_.rotate_);
    }

}

void GameScene::Draw()
{
    ModelManager::GetInstance()->PreDrawForObjectModel();

    ground_->Draw(&SceneCamera_, { 1,1,1,1 });
}

void GameScene::DrawShadow()
{
}
