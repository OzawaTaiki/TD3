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

class Camera;

class MovableObjectManager : public iEventListener
{
public:
	void Initialize();
	void Update(const Camera& camera);
	void Draw(const Camera& camera);

	/// <summary>
	/// オブジェクトの追加
	/// </summary>
	/// <param name="position">オブジェクト生成位置</param>
	void AddMovableObject(const Vector3& position);

	/// <summary>
	/// 全てのオブジェクト位置を返す
	/// </summary>
	std::vector<Vector3> GetAllObjectPosition() const;

	void OnEvent(const GameEvent& _event) override;

private:
	std::vector<std::unique_ptr<ObjectModel>> objects_;
	std::vector<std::unique_ptr<AABBCollider>> colliders_;

	Input* input_ = nullptr;

	/// <summary>
	/// オブジェクトをドラッグアンドドロップで動かす処理
	/// </summary>
	void HandleObjectDragAndDrop(const Camera& camera);

	// ドラッグ関連のメンバ変数
	bool isDragging_ = false;
	Vector3 dragOffset_;
	float dragStartHeight_ = 0.0f;          // オブジェクトの元の高さを保持
	ObjectModel* draggingObject_ = nullptr; // ドラッグ中のオブジェクト



	/// <summary>
	/// マウスレイの生成（オブジェクトとの衝突判定用）
	/// </summary>
	Ray CreateMouseRay(const Camera& camera);

	/// <summary>
	/// // マウスレイと平面の交差判定（オブジェクトが地面を貫通して奥に移動するのを防ぐ用）
	/// </summary>
	bool IntersectRayWithPlane(const Ray& ray, const Vector3& planeNormal, float planeD, Vector3& outIntersection);
};

