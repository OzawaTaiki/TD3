#pragma once

// Engine
#include <Features/Model/ObjectModel.h>
#include <Features/Collision/Manager/CollisionManager.h>
#include <System/Input/Input.h>

class Camera;

/// <summary>
/// movableObjectの影の位置にできるオブジェクト（ポイントライトの影実体化が実装され次第削除する予定）
/// </summary>
class ShadowObject {
public:
	void Initialize();
	void Update();
	void Draw(const Camera& camera);

	/// <summary>
	/// 動かせるオブジェクトの位置を設定（この影オブジェクトの寄生先）
	/// </summary>
	void SetMovableObjectPosition(const Vector3& position) { movableObjectPosition_ = position; }
	/// <summary>
	/// ライトの位置を設定
	/// </summary>
	void SetLightPosition(const Vector3& position) { lightPosition_ = position; }

private:
	std::unique_ptr<ObjectModel> object_;
	uint32_t texture_;
	std::unique_ptr<OBBCollider> collider_;

	// 動かせるオブジェクトの位置を保持（寄生先）
	Vector3 movableObjectPosition_;
	// ポイントライトの位置を保持
	Vector3 lightPosition_;

private:
	Matrix4x4 MakeRotateMatrix(const Quaternion& q);
};
