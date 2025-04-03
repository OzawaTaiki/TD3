#pragma once

#include <Features/Scene/Interface/BaseScene.h>

#include <Features/Camera/Camera/Camera.h>
#include <Features/Camera/DebugCamera/DebugCamera.h>
#include <Features/Model/ObjectModel.h>
#include <Features/Effect/Manager/ParticleManager.h>
#include <Features/LineDrawer/LineDrawer.h>
#include <System/Input/Input.h>

#include <Features/Collision/Manager/CollisionManager.h>
#include <Features/Collision/RayCast/RayCollisionManager.h>

// Application
#include <Application/MovableObject/MovableObjectManager.h>
#include <Application/Field/Field.h>
#include <Application/Tower/Tower.h>
#include <Application/EnemySpawnManager/EnemySpawnManager.h>

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

    std::vector<Particle> particles_;

    LineDrawer* lineDrawer_ = nullptr;
    Input* input_ = nullptr;
    ParticleManager* particleManager_ = nullptr;
    std::unique_ptr<LightGroup> lights_;


    std::unique_ptr<ObjectModel> ground_ = nullptr;

/// <summary>
/// Application
/// </summary>
private:
    // フィールド
	std::unique_ptr<Field> field_;
    // 動かせるオブジェクトを管理するクラス
    std::unique_ptr<MovableObjectManager> movableObjectManager_;
    // タワー
    std::unique_ptr<Tower> tower_;
    // 敵を管理するクラス
	std::unique_ptr<EnemySpawnManager> enemySpawnManager_;
};