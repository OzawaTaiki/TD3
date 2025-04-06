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
	void Update(const float maxDistance);
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

	// 動かせるオブジェクトの位置を保持
	Vector3 movableObjectPosition_;
	// ポイントライトの位置を保持
	Vector3 lightPosition_;

	// 有効化フラグ : ライトから一定距離離れた場合には描画やコライダー登録をしないために使用
	bool isActive_ = false;

	/// <summary>
	/// 動かせるオブジェクトとポイントライトの位置を考慮して、影オブジェクトのTransformを計算して設定
	/// </summary>
	void CalculateShadowTransform(const float maxDistance);

	/// <summary>
	/// 実体化関連パラメーター
	/// </summary>
	bool isScaling_ = false; // 攻撃アニメーション中かどうか
	float scaleYStart_ = 1.0f; // 開始スケール
	float scaleYTarget_ = 1.0f; // 目標スケール
	float scaleYCurrent_ = 1.0f; // 現在のスケール
	float animationTime_ = 0.0f; // アニメーション経過時間
	float animationDuration_ = 0.0f; // アニメーション全体の時間
	float scaleUpDuration_ = 0.2f; // 増加の時間 :（SPACEを押してから最大値まで実体化する時間）
	float scaleDownDuration_ = 1.6f; // 戻る時間 :（最大値まで実体化してから元の位置まで戻るまでの時間）
	bool isReturning_ = false; // 戻りアニメーション中かどうか

	/// <summary>
	/// スペース押下時、影オブジェクトの実体化（コライダー設定、アニメーション処理含む）
	/// </summary>
	void HandleAttackInput();

private:
	Matrix4x4 MakeRotateMatrix(const Quaternion& q);
};
