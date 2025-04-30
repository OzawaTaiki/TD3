#pragma once

// Engine
#include <System/Input/Input.h>
#include <Features/Model/ObjectModel.h>

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
	void Update();

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
	void Move();

	/// <summary>
	/// 向き変更処理（右スティック入力）
	/// </summary>
	void Rotate();

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
	// パラメーター
	// ---------------------------------------------------------
	
	// 移動速度
	float moveSpeed_ = 0.15f;

	// プレイヤーの向き（Y軸回転角度）
	float rotationY_ = 0.0f;
};

