#include "Player.h"

// Engine
#include <Core/DXCommon/TextureManager/TextureManager.h>

// ----------------------------------------
// 初期化処理
// ----------------------------------------
void Player::Initialize()
{
	input_ = Input::GetInstance();

	// オブジェクト生成
	objectPlayer_ = std::make_unique<ObjectModel>("Player");
	objectPlayer_->Initialize("Player/player.obj");
	// テクスチャ読み込み
	texturePlayer_ = TextureManager::GetInstance()->Load("game/player.png");
}

// ----------------------------------------
// フレーム更新処理
// ----------------------------------------
void Player::Update()
{
	// 移動処理
	Move();
	// 向き変更処理
	Rotate();

	// オブジェクト更新
	objectPlayer_->Update();
}

// ----------------------------------------
// 描画処理
// ----------------------------------------
void Player::Draw(const Camera& camera)
{
#ifdef _DEBUG
	ImGui::Begin("Player");

	ImGui::DragFloat("moveSpeed", &moveSpeed_, 0.01f);

	ImGui::End();
#endif // _DEBUG


	// オブジェクト描画
	objectPlayer_->Draw(&camera, texturePlayer_, { 1, 1, 1, 1 });
}

// ----------------------------------------
// 移動処理
// ----------------------------------------
void Player::Move()
{
	// 左スティック入力取得
	Vector2 leftStick = input_->GetPadLeftStick();

	// 移動方向の計算
	Vector3 moveDirection(leftStick.x, 0.0f, leftStick.y);
	if (moveDirection.Length() > 0.0f) {
		moveDirection.Normalize();
		moveDirection *= moveSpeed_;
	}

	// プレイヤーに移動を適用
	objectPlayer_->translate_ += moveDirection;
}

// ----------------------------------------
// 向き変更処理
// ----------------------------------------
void Player::Rotate()
{
	// 右スティック入力取得
	Vector2 rightStick = input_->GetPadRightStick();

	// 向いている角度を右スティックの方向に更新
	if (rightStick.Length() >= 0.1f) {
		rotationY_ = std::atan2f(rightStick.x, rightStick.y);
	}

	// プレイヤーに向きを適用
	objectPlayer_->euler_.y = rotationY_;
}
