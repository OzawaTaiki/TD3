#pragma once
#include <Features/Scene/Interface/BaseScene.h>
#include <Features/Camera/Camera/Camera.h>
#include <Features/Camera/DebugCamera/DebugCamera.h>
#include <Features/Model/ObjectModel.h>
#include <Features/Effect/Manager/ParticleManager.h>
#include <Features/LineDrawer/LineDrawer.h>
#include <System/Input/Input.h>

#include <System/Time/GameTime.h>
#include <Features/UI/UIButton.h>
#include <Features/Model/Primitive/Ring.h>
#include <Features/Model/Primitive/Ellipse.h>
#include <Features/Model/Primitive/Cylinder.h>
#include <Features/UVTransform/SpriteSheetAnimetion.h>
#include <Features/UVTransform/UVTransformAnimation.h>
#include <Features/Animation/Sequence/AnimationSequence.h>
#include "../EdgeDetection.h"

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
    ParticleManager* particleManager_ = nullptr;

    std::unique_ptr<ObjectModel> cube_ = nullptr;

    std::unique_ptr<ObjectModel> bunny_= nullptr;
    std::unique_ptr<ObjectModel> human_= nullptr;
    std::unique_ptr<ObjectModel> plane_ = nullptr;

    std::unique_ptr<ObjectModel> testModel_ = nullptr;

    std::unique_ptr<Sprite> sprite_ = nullptr;

    std::unique_ptr<LightGroup> lights_;
    std::list<std::pair<float, Vector4>> colors;

    EdgeDetection* edgeDetection;

    AnimationSequence* sequence_ = nullptr;
 
    bool test;


#ifdef _DEBUG
    void ImGui();
#endif // _DEBUG
};
