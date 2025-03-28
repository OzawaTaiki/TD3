#pragma once

#include <Features/Scene/Interface/BaseScene.h>

#include <Features/Camera/Camera/Camera.h>
#include <Features/Camera/DebugCamera/DebugCamera.h>
#include <Features/Model/ObjectModel.h>
#include <Features/Effect/Manager/ParticleManager.h>
#include <Features/LineDrawer/LineDrawer.h>
#include <System/Input/Input.h>


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

    // キューブオブジェクト
	std::unique_ptr<ObjectModel> objectCube_ = nullptr;


    /// <summary>
	/// オブジェクトのドラッグ&ドロップ関連
	/// </summary>
    
    // オブジェクトのドラッグアンドドロップ処理
    void HandleObjectDragAndDrop();

    // オブジェクトを持ち上げている状態の管理
    bool isHoldingObject_ = false;
	// 持ち上げているオブジェクト（持ち上げているオブジェクトを一時的に保存）
	ObjectModel* grabbedObject_ = nullptr;

    Vector4 Transform(const Matrix4x4& mat, const Vector4& vec);
};