#pragma once

// Engine
#include <System/Input/Input.h>
#include <Features/Model/ObjectModel.h>
#include <Features/Collision/Manager/CollisionManager.h>

class MovableObject;

class Player
{
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 更新
	/// </summary>
	void Update(const std::vector<std::unique_ptr<MovableObject>>& movableObjects);

	/// <summary>
	/// 描画
	/// </summary>
	/// <param name="camera"></param>
	void Draw(const Camera& camera);

private:
	// ---------------------------------------------------------
	// 内部処理
	// ---------------------------------------------------------

	/// <summary>
	/// 移動処理（左スティック入力）
	/// </summary>
	void HandleMove();

	/// <summary>
	/// 向き変更処理（右スティック入力）
	/// </summary>
	void HandleRotate();

	/// <summary>
	/// オブジェクトの掴み処理（RB入力）
	/// </summary>
	void HandleObjectPickup(const std::vector<std::unique_ptr<MovableObject>>& movableObjects); 

	/// <summary>
	/// コライダー更新処理
	/// </summary>
	void UpdateColliders();

	

private:
	// ---------------------------------------------------------
	// システム関連
	// ---------------------------------------------------------
	
	// 入力
	Input* input_;

	// ---------------------------------------------------------
	// オブジェクト関連
	// ---------------------------------------------------------
	
	// プレイヤーオブジェクト
	std::unique_ptr<ObjectModel> objectPlayer_;

	// プレイヤーテクスチャ
	uint32_t texturePlayer_;

	// ---------------------------------------------------------
	// コライダー
	// ---------------------------------------------------------

	// プレイヤー本体のコライダー
	std::unique_ptr<OBBCollider> colliderPlayer_;

	// プレイヤー前方のコライダー（掴む対象のオブジェクトとの衝突判定に使用）
	std::unique_ptr<SphereCollider> colliderForward_;

	// ---------------------------------------------------------
	// パラメーター
	// ---------------------------------------------------------
	
	// 移動速度
	float moveSpeed_ = 0.15f;
	// プレイヤーの向き（Y軸回転角度）
	float rotationY_ = 0.0f;

	// ---------------------------------------------------------
	// オブジェクト掴み関連
	// ---------------------------------------------------------

	// 掴んでいるかどうか
	bool isHolding_ = false;

	// 掴んでいるオブジェクトのポインタ
	MovableObject* holdingObject_ = nullptr;
};