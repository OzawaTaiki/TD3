#include "SampleFramework.h"

#include "SampleScene.h"
#include <Features/Scene/ParticleTestScene.h>
#include "SceneFactory.h"

void SampleFramework::Initialize()
{
    Framework::Initialize();


    JsonHub::GetInstance()->Initialize("Resources/Data/");

    rtvManager_->CreateRenderTarget("default", WinApp::kWindowWidth_, WinApp::kWindowHeight_, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, Vector4(0.4625f, 0.925f, 0.4625f, 1.0f), false);
    rtvManager_->CreateRenderTarget("ShadowMap", 4096, 4096, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,  Vector4(1.0f, 1.0f, 1.0f, 1.0f),true);

    sceneManager_->SetSceneFactory(new SceneFactory());

    // 最初のシーンで初期化
    sceneManager_->Initialize("Game");
}

void SampleFramework::Update()
{
    Framework::Update();


    //========== 更新処理 =========

    sceneManager_->Update();
    //particleManager_->Update(); TODO ; 引数のカメラの回転をなんとかしたい

    //=============================
}

void SampleFramework::Draw()
{
    Framework::PreDraw();

    rtvManager_->SetDepthStencil("ShadowMap");
    sceneManager_->DrawShadow();

    rtvManager_->SetRenderTexture("default");

    // ========== 描画処理 ==========

    sceneManager_->Draw();

    lineDrawer_->Draw();
    //=============================


    dxCommon_->PreDraw();
    // スワップチェインに戻す
    rtvManager_->SetSwapChainRenderTexture(dxCommon_->GetSwapChain());
    // レンダーテクスチャを描画
    rtvManager_->DrawRenderTexture("default");

    Framework::PostDraw();

    // 後にupdateに
    sceneManager_->ChangeScene();

}

void SampleFramework::Finalize()
{
    Framework::Finalize();
}
