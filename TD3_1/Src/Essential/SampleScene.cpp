#include "SampleScene.h"

#include <Features/Scene/Manager/SceneManager.h>
#include <Debug/ImGuiManager.h>
#include <Features/Sprite/Sprite.h>
#include <Features/Model/Manager/ModelManager.h>
#include <Core/DXCommon/TextureManager/TextureManager.h>
#include <Core/DXCommon/RTV/RTVManager.h>
#include <Features/Collision/Manager/CollisionManager.h>
#include <Debug/ImguITools.h>

#include <Features/Collision/RayCast/RayCollisionManager.h>


SampleScene::~SampleScene()
{
    delete bunnyCollider_;
    delete cubeCollider_;
    delete cubeCollider2_;
}

void SampleScene::Initialize()
{
    SceneCamera_.Initialize();
    SceneCamera_.translate_ = { 0,5,-20 };
    SceneCamera_.rotate_ = { 0.26f,0,0 };
    SceneCamera_.UpdateMatrix();
    debugCamera_.Initialize();


    lineDrawer_ = LineDrawer::GetInstance();
    lineDrawer_->SetCameraPtr(&SceneCamera_);

    input_ = Input::GetInstance();

    oModel_ = std::make_unique<ObjectModel>("plane");
    oModel_->Initialize("bunny.gltf");
    oModel_->translate_.x = 3;

    oModel2_ = std::make_unique<ObjectModel>("cube");
    oModel2_->Initialize("Cube/Cube.obj");
    oModel2_->translate_.x = -3;

    aModel_ = std::make_unique<ObjectModel>("sample");
    aModel_->Initialize("AnimSample/AnimSample.gltf");

    plane_ = std::make_unique<ObjectModel>("plane2");
    plane_->Initialize("Tile/Tile.gltf");
    plane_->GetUVTransform().SetScale({ 100,100 });

    uint32_t textureHandle = TextureManager::GetInstance()->Load("uvChecker.png");
    sprite_ = Sprite::Create("uvChecker", textureHandle);

    lights_ = std::make_shared<LightGroup>();
    lights_->Initialize();
    auto pl = std::make_shared<PointLightComponent>();
    lights_->AddPointLight("a", pl);
    LightingSystem::GetInstance()->SetActiveGroup(lights_);

    sequence_ = std::make_unique<AnimationSequence>("test");
    sequence_->Initialize("Resources/Data/");

    bunnyCollider_ = new AABBCollider();
    bunnyCollider_->SetLayer("bunny");
    bunnyCollider_->SetMinMax({ -1,-1,-1 }, { 1,1,1 });
    bunnyCollider_->SetOnCollisionCallback([](Collider* _other, const ColliderInfo& _info) {
        });

    cubeCollider_ = new SphereCollider();
    cubeCollider_->SetLayer("cube");
    cubeCollider_->SetRadius(.5f);
    cubeCollider_->SetWorldTransform(oModel2_->GetWorldTransform());
    cubeCollider_->SetOnCollisionCallback([](Collider* _other, const ColliderInfo& _info) {
        });

    cubeCollider2_ = new CapsuleCollider();
    cubeCollider2_->SetLayer("cube2");
    cubeCollider2_->SetRadius(1);
    cubeCollider2_->SetHeight(5);
    cubeCollider2_->SetWorldTransform(aModel_->GetWorldTransform());
    cubeCollider2_->SetOnCollisionCallback([](Collider* _other, const ColliderInfo& _info) {
        });

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
        aModel_->ChangeAnimation("RotateAnim", 0.5f,true);
    }

    if (ImGui::Button("scale"))
    {
        aModel_->ChangeAnimation("ScaleAnim", 0.5f);
    }

    ImGuiTool::TimeLine("TimeLine", sequence_.get());

    //lights_->DrawDebugWindow();

    static bool play = false;
    if (ImGui::Button("Play"))
    {
        //play = !play;
        sequence_->Save();
    }
    if (play)
        oModel_->translate_ = sequence_->GetValue<Vector3>("a");


#endif // _DEBUG
    //LightingSystem::GetInstance()->SetLightGroup(lights_.get());

    lights_->ImGui();

    oModel_->Update();
    oModel2_->Update();
    aModel_->Update();
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
        ParticleSystem::GetInstance()->Update();
    }
    else
    {
        SceneCamera_.Update();
        SceneCamera_.UpdateMatrix();
    }


    ParticleSystem::GetInstance()->Update();
#pragma region 半直線との衝突判定

    //if(input_->IsMouseTriggered(0))
    //{
    //    // カーソルからRayを生成 (うまくいかない
    //    Ray ray = Ray::CreateFromMouseCursor(SceneCamera_, input_->GetMousePosition());

    //    Ray ray2 = Ray::CreateFromPointAndTarget(SceneCamera_.translate_, oModel2_->GetWorldTransform()->GetWorldPosition());
    //    RayCollisionManager::GetInstance()->RegisterCollider(cubeCollider_);

    //    std::vector<RayCastHit> hits;
    //    RayCollisionManager::GetInstance()->RayCastAll(ray2, hits, 0xffffffff);

    //    RayCastHit hit2;
    //    // 個々での衝突判定
    //    if (RayCollisionManager::GetInstance()->RayCast(ray, cubeCollider_, hit2))
    //    {
    //        // 衝突した
    //        Debug::Log("Ray Hit\n");
    //    }
    //    else
    //        Debug::Log("Ray Not Hit\n");
    //}

#pragma endregion

    CollisionManager::GetInstance()->RegisterCollider(bunnyCollider_);
    CollisionManager::GetInstance()->RegisterCollider(cubeCollider_);
    CollisionManager::GetInstance()->RegisterCollider(cubeCollider2_);

    CollisionManager::GetInstance()->Update();
}

void SampleScene::Draw()
{
    ModelManager::GetInstance()->PreDrawForObjectModel();

    oModel_->Draw(&SceneCamera_, { 1,1,1,1 });
    oModel2_->Draw(&SceneCamera_, { 1,1,1,1 });
    plane_->Draw(&SceneCamera_, { 1,1,1,1 });

    aModel_->Draw(&SceneCamera_, { 1,1,1,1 });

    Sprite::PreDraw();
    sprite_->Draw();


    //button_->Draw();

    ParticleSystem::GetInstance()->DrawParticles();

}

void SampleScene::DrawShadow()
{

    oModel_->DrawShadow(&SceneCamera_, 0);
    oModel2_->DrawShadow(&SceneCamera_, 1);
    aModel_->DrawShadow(&SceneCamera_, 2);

}

#ifdef _DEBUG
#include <imgui.h>
void SampleScene::ImGui()
{

}
#endif // _DEBUG
