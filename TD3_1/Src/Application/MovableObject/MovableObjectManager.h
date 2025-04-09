#pragma once

// C++
#include <vector>
#include <memory>

// Engine
#include <Features/Model/ObjectModel.h>
#include <Features/Collision/Manager/CollisionManager.h>
#include <Features/Collision/RayCast/Ray.h>
#include <System/Input/Input.h>
#include <Features/Event/EventListener.h>

// Application
#include <Application/MovableObject/MovableObject.h>

class Camera;

class MovableObjectManager : public iEventListener
{
public:
    ~MovableObjectManager() override;

	void Initialize();
	void Update(const Camera& camera);
	void Draw(const Camera& camera);

	/// <summary>
	/// オブジェクトの追加
	/// </summary>
	/// <param name="position">オブジェクト生成位置</param>
	void AddMovableObject(const Vector3& position);

	/// <summary>
	/// 全てのオブジェクトを返す
	/// </summary>
	const std::vector<std::unique_ptr<MovableObject>>& GetAllObjects() const { return objects_; }

	void OnEvent(const GameEvent& _event) override;

private:
	std::vector<std::unique_ptr<MovableObject>> objects_;

	uint32_t texture_;

	Input* input_ = nullptr;

	/// <summary>
	/// オブジェクトをドラッグアンドドロップで動かす処理
	/// </summary>
	void HandleObjectDragAndDrop(const Camera& camera);

	// ドラッグ関連のメンバ変数
	bool isDragging_ = false;
	Vector3 dragOffset_;
	float dragStartHeight_ = 0.0f;          // オブジェクトの元の高さを保持
	MovableObject* draggingObject_ = nullptr; // ドラッグ中のオブジェクト

    uint32_t haveSoundHandle_ = 0; // サウンドハンドル
    uint32_t putSoundHandle_ = 0; // サウンドハンドル

    float haveSoundVolume_ = 0.5f; // デフォルトのボリュームを設定
    float putSoundVolume_ = 0.5f; // デフォルトのボリュームを設定

	/// <summary>
	/// マウスレイの生成（オブジェクトとの衝突判定用）
	/// </summary>
	Ray CreateMouseRay(const Camera& camera);

	/// <summary>
	/// // マウスレイと平面の交差判定（オブジェクトが地面を貫通して奥に移動するのを防ぐ用）
	/// </summary>
	bool IntersectRayWithPlane(const Ray& ray, const Vector3& planeNormal, float planeD, Vector3& outIntersection);
};

