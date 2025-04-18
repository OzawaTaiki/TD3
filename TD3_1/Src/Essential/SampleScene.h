#pragma once
#include <Features/Scene/Interface/BaseScene.h>
#include <Features/Camera/Camera/Camera.h>
#include <Features/Camera/DebugCamera/DebugCamera.h>
#include <Features/Model/ObjectModel.h>
#include <Features/Model/AnimationModel.h>

#include <Features/Effect/Manager/ParticleSystem.h>
#include <Features/LineDrawer/LineDrawer.h>
#include <System/Input/Input.h>
#include <System/Time/GameTime.h>
#include <Features/UI/UIButton.h>
#include <Features/Model/Primitive/Ring.h>
// #include <Features/Model/Primitive/Ellipse.h>
#include <Features/Model/Primitive/Cylinder.h>
#include <Features/UVTransform/SpriteSheetAnimetion.h>
#include <Features/UVTransform/UVTransformAnimation.h>
#include <Features/Animation/Sequence/AnimationSequence.h>

#include <Features/Collision/Manager/CollisionManager.h>


#include <memory>

class SampleScene : public BaseScene
{
public:

     ~SampleScene() override;

    void Initialize() override;
    void Update() override;
    void Draw() override;
    void DrawShadow() override;

private:
    // シーン関連
    Camera SceneCamera_ = {};
    DebugCamera debugCamera_ = {};
    bool enableDebugCamera_ = false;

    std::vector<Particle> particles_;

    LineDrawer* lineDrawer_ = nullptr;
    Input* input_ = nullptr;
    ParticleSystem* particleSystem_ = nullptr;

    std::unique_ptr<ObjectModel> aModel_ = nullptr;

    std::unique_ptr<ObjectModel> oModel_= nullptr;
    std::unique_ptr<ObjectModel> oModel2_= nullptr;
    std::unique_ptr<ObjectModel> plane_ = nullptr;

    std::unique_ptr <Sprite> sprite_ = nullptr;

    std::shared_ptr<LightGroup> lights_;
    std::list<std::pair<float, Vector4>> colors;

    std::unique_ptr<AnimationSequence> sequence_ = nullptr;

    AABBCollider* bunnyCollider_ = nullptr;
    SphereCollider* cubeCollider_ = nullptr;
    CapsuleCollider* cubeCollider2_ = nullptr;




#ifdef _DEBUG
    void ImGui();
#endif // _DEBUG
};
