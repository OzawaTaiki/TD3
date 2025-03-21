#include "SampleScene.h"

#include <Features/Scene/Manager/SceneManager.h>
#include <Debug/ImGuiManager.h>
#include <Features/Sprite/Sprite.h>
#include <Features/Model/Manager/ModelManager.h>
#include <Core/DXCommon/RTV/RTVManager.h>
#include <Core/DXCommon/TextureManager/TextureManager.h>
#include <Debug/ImguITools.h>


SampleScene::~SampleScene()
{
    delete edgeDetection;
    delete sequence_;
}

void SampleScene::Initialize()
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

    bunny_ = std::make_unique<ObjectModel>("cube");
    bunny_->Initialize("cube/cube.obj");
    bunny_->translate_.x = 3;

    human_ = std::make_unique<ObjectModel>("human");
    human_->Initialize("human/walk.gltf");
    human_->translate_.x = -3;

    cube_ = std::make_unique<ObjectModel>("sample");
    //cube_->Initialize("AnimSample/AnimSample.gltf");
    //cube_->Initialize("sphere/sphere.obj");
    cube_->Initialize("Triangular_Prism/Triangular_Prism.obj");

    //aModel_->translate_.x = -127;
    //aModel_->translate_.z = 126;


    plane_ = std::make_unique<ObjectModel>("plane2");
    plane_->Initialize("Tile/Tile.gltf");
    plane_->GetUVTransform().SetScale({ 100,100 });

    uint32_t textureHandle = TextureManager::GetInstance()->Load("uvChecker.png");
    sprite_ = Sprite::Create("uvChecker", textureHandle);

    lights_ = std::make_unique<LightGroup>();
    lights_->Initialize();


    edgeDetection = new EdgeDetection();
    edgeDetection->Initialize(RTVManager::GetInstance()->GetRenderTexture("ShadowMap"));

    sequence_ = new AnimationSequence("test");
    sequence_->Initialize("Resources/Data/");

    test = false;
}

void SampleScene::Update()
{
    // シーン関連更新
#ifdef _DEBUG
    if (Input::GetInstance()->IsKeyTriggered(DIK_RETURN) &&
        Input::GetInstance()->IsKeyPressed(DIK_RSHIFT))
        enableDebugCamera_ = !enableDebugCamera_;

    if (ImGui::Button("rot"))
    {
        cube_->ChangeAnimation("RotateAnim", 0.5f,true);
    }

    if (ImGui::Button("scale"))
    {
        cube_->ChangeAnimation("ScaleAnim", 0.5f);
    }

    ImGuiTool::GradientEditor("Ambient", colors);

    lights_->DrawDebugWindow();

    ImGuiTool::TimeLine("timeline", sequence_);
#endif // _DEBUG
    LightingSystem::GetInstance()->SetLightGroup(lights_.get());

    // TODO IDを変更できるようにする
    // TODO Normalをなんとか
    // TODO SL PL での影の描画

    if (ImGui::Button("Create Shadow Obj"))
    //if(test)
    {
        if (edgeDetection)
            delete edgeDetection;

        edgeDetection = new EdgeDetection();
        edgeDetection->Initialize(RTVManager::GetInstance()->GetRenderTexture("ShadowMap"));

        edgeDetection->Execute();

        DirectionalLight light = lights_->GetDirectionalLight();

        if (testModel_)
        {
            testModel_.reset();
        }

        testModel_ = std::make_unique<ObjectModel>("test");
        testModel_->Initialize(edgeDetection->GenerateMeshFromContourPoints(Inverse(light.viewProjection), 1.0f, testModel_->translate_));
    }

    //test = true;


    if (testModel_)
    {
        testModel_->Update();
    }
    bunny_->Update();
    human_->Update();
    cube_->Update();
    plane_->Update();
    sprite_->Update();

    if (input_->IsKeyTriggered(DIK_TAB))
    {
        SceneManager::GetInstance()->ReserveScene("ParticleTest");
    }

    if (enableDebugCamera_)
    {
        debugCamera_.Update();
        SceneCamera_.matView_ = debugCamera_.matView_;
        SceneCamera_.TransferData();
        ParticleManager::GetInstance()->Update(debugCamera_.rotate_);
    }
    else
    {
        SceneCamera_.Update();
        SceneCamera_.UpdateMatrix();
        ParticleManager::GetInstance()->Update(SceneCamera_.rotate_);
    }

}

void SampleScene::Draw()
{

    ModelManager::GetInstance()->PreDrawForObjectModel();

    bunny_->Draw(&SceneCamera_, { 1,1,1,1 });
    //human_->Draw(&SceneCamera_, { 1,1,1,1 });
    plane_->Draw(&SceneCamera_, { 1,1,1,1 });

    cube_->Draw(&SceneCamera_, { 1,1,1,1 });

    if (testModel_)
    {
        testModel_->Draw(&SceneCamera_, { 1,1,1,1 });
    }

    Sprite::PreDraw();
    sprite_->Draw();


    //button_->Draw();

    ParticleManager::GetInstance()->Draw(&SceneCamera_);

}

void SampleScene::DrawShadow()
{
    PSOManager::GetInstance()->SetPipeLineStateObject(PSOFlags::Type_ShadowMap);
    PSOManager::GetInstance()->SetRootSignature(PSOFlags::Type_ShadowMap);


    //bunny_->DrawShadow(&SceneCamera_, 2);
    //oModel2_->DrawShadow(&SceneCamera_,1);
    cube_->DrawShadow(&SceneCamera_, 2);
}

#ifdef _DEBUG
#include <imgui.h>
void SampleScene::ImGui()
{

}
#endif // _DEBUG
