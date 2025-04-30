#pragma once

#include <Features/Scene/Interface/BaseScene.h>

#include <Features/Camera/Camera/Camera.h>
#include <Features/Camera/DebugCamera/DebugCamera.h>
#include <Features/Model/ObjectModel.h>
#include <Features/Effect/Manager/ParticleSystem.h>
#include <Features/LineDrawer/LineDrawer.h>
#include <System/Input/Input.h>

#include <Features/Collision/Manager/CollisionManager.h>
#include <Features/Collision/RayCast/RayCollisionManager.h>

// Application
#include <Application/Object/MovableObject/MovableObjectManager.h>
#include <Application/Object/Field/Field.h>
#include <Application/Object/Tower/Tower.h>
#include <Application/EnemySpawnManager/EnemySpawnManager.h>
#include <Application/Object/PointLightObject/PointLightObjectManager.h>
#include <Application/Object/ShadowObject/ShadowObjectManager.h>
#include <Application/Event/RewardGauge.h>
#include <Application/UI/Game/GameUI.h>
#include <Application/ClearChecker/ClearChecker.h>
#include <Application/Transition/Fade/Fade.h>
#include <Application/Object/Player/Player.h>

class GameScene : public BaseScene
{
public:
    ~GameScene() override;
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
	Vector3 originalCameraTranslate_; // カメラの初期位置を保持しておく（シェイクでずれる対策）

    std::vector<Particle> particles_;

    LineDrawer* lineDrawer_ = nullptr;
    Input* input_ = nullptr;
    ParticleSystem* particleSystem_ = nullptr;


    std::unique_ptr<ObjectModel> ground_ = nullptr;
	uint32_t textureGround_;

    std::shared_ptr<LightGroup> lights_ = nullptr;

/// <summary>
/// Application
/// </summary>
private:
    // プレイヤー
    std::unique_ptr<Player> player_;
    // フィールド
	std::unique_ptr<Field> field_;
    // 動かせるオブジェクトを管理するクラス
    std::unique_ptr<MovableObjectManager> movableObjectManager_;
    // タワー
    std::unique_ptr<Tower> tower_;
    // 敵を管理するクラス
	std::unique_ptr<EnemySpawnManager> enemySpawnManager_;
    // ポイントライトオブジェクトを管理するクラス
    std::unique_ptr<PointLightObjectManager> pointLightObjectManager_;
    // 影オブジェクトを管理するクラス
    std::unique_ptr<ShadowObjectManager> shadowObjectManager_;
    // 報酬ゲージ
    std::unique_ptr<RewardGauge> rewardGauge_ = nullptr;
    // UI
    std::unique_ptr<GameUI> gameUI_;
    // クリアチェック
    std::unique_ptr<ClearChecker> clearChecker_ = nullptr;

private:
    // フェード関連
    enum class Phase {
        kFadeIn,
        kMain,
        kFadeOut
    };
    std::unique_ptr<Fade> fade_;
    Phase phase_ = Phase::kFadeIn;

    void SaveToFile();
	void LoadFromFile();
};