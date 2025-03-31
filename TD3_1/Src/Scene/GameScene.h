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
#include <Application/Field/Field.h>

class GameScene : public BaseScene
{
public:
    ~GameScene() override;
    void Initialize() override;
    void Update() override;
    void Draw() override;
    void DrawShadow() override;

/// <summary>
/// 基盤機能
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
/// アプリケーション
/// </summary>
private:
	/// <summary>
	/// ゲームシーン用オブジェクト関連
	/// </summary>
    
    // 初期化・更新・描画
	void InitializeGameObjects();
	void UpdateGameObjects();
	void DrawGameObjects();

    // フィールド
	std::unique_ptr<Field> field_;

    /// <summary>
	/// オブジェクトのドラッグ&ドロップ関連
	/// </summary>
    
    // ドラッグによって動かせる複数のオブジェクトを配列で管理
	std::vector<std::unique_ptr<ObjectModel>> movableObjects_;
	std::vector<std::unique_ptr<OBBCollider>> colliders_;

    // ドラッグによって動かせるオブジェクトを追加
	void AddMovableObject(const Vector3& position);
    // オブジェクトのドラッグアンドドロップ処理
    void HandleObjectDragAndDrop();

    // マウスレイの生成（オブジェクトとの衝突判定用）
    Ray CreateMouseRay();
    // マウスレイと平面の交差判定（オブジェクトが地面を貫通して奥に移動するのを防ぐ用）
    bool IntersectRayWithPlane(const Ray& ray, const Vector3& planeNormal, float planeD, Vector3& outIntersection);
};