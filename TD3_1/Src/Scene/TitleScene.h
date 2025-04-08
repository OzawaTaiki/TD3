#pragma once

#include <Features/Scene/Interface/BaseScene.h>

#include <Features/Camera/Camera/Camera.h>
#include <Features/Camera/DebugCamera/DebugCamera.h>
#include <Features/Model/ObjectModel.h>
#include <Features/Effect/Manager/ParticleManager.h>
#include <Features/LineDrawer/LineDrawer.h>
#include <System/Input/Input.h>

// Application
#include <Application/Transition/Fade/Fade.h>

class TitleScene : public BaseScene
{
public:
    ~TitleScene() override;
    void Initialize() override;
    void Update() override;
    void Draw() override;
    void DrawShadow() override;

    /// <summary>
    /// base
    /// </summary>
private:
    // シーン関連
    Camera SceneCamera_ = {};
    DebugCamera debugCamera_ = {};
    bool enableDebugCamera_ = false;

    std::vector<Particle> particles_;

    LineDrawer* lineDrawer_ = nullptr;
    Input* input_ = nullptr;
    ParticleManager* particleManager_ = nullptr;
    std::unique_ptr<LightGroup> lights_;


    std::unique_ptr<ObjectModel> ground_ = nullptr;

private:
    // フェード関連
    enum class Phase {
        kFadeIn,
        kMain,
        kFadeOut
    };
    std::unique_ptr<Fade> fade_;
    Phase phase_ = Phase::kFadeIn;
};

