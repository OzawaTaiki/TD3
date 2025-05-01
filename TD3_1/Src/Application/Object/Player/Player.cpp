#include "Player.h"

// Engine
#include <Core/DXCommon/TextureManager/TextureManager.h>

// Application
#include <Application/Object/MovableObject/MovableObjectManager.h>
#include <Application/Util/MyMath/MyMath.h>

// ----------------------------------------
// 初期化処理
// ----------------------------------------
void Player::Initialize()
{
	input_ = Input::GetInstance();

	///
	///	オブジェクト関連
	///	

	// オブジェクト生成
	objectPlayer_ = std::make_unique<ObjectModel>("Player");
	objectPlayer_->Initialize("Player/player.obj");
	objectPlayer_->useQuaternion_ = true;

	// テクスチャ読み込み
	texturePlayer_ = TextureManager::GetInstance()->Load("game/player.png");

	///
	/// コライダー関連
	///

	// プレイヤー本体のコライダー生成
	colliderPlayer_ = std::make_unique<OBBCollider>("playerCollider");
	colliderPlayer_->Initialize();
	colliderPlayer_->SetLayer("player");

	colliderPlayer_->SetLayerMask("playerForward");

	// プレイヤー前方のコライダー生成
	colliderForward_ = std::make_unique<SphereCollider>("playerForwardCollider");
	colliderForward_->Initialize();
	colliderForward_->SetLayer("playerForward");
	colliderForward_->SetRadius(1.0f);
	colliderForward_->SetOffset(Vector3(0, 2, 2));

	colliderForward_->SetLayerMask("player");
}

// ----------------------------------------
// 毎フレーム更新処理
// ----------------------------------------
void Player::Update(const std::vector<std::unique_ptr<MovableObject>>& movableObjects)
{
	// 移動処理
	HandleMove();
	// 向き変更処理
	HandleRotate();
	// ブロック掴み処理
	HandleObjectPickup(movableObjects);


	// コライダー更新
	UpdateColliders();
	// オブジェクト更新
	objectPlayer_->Update();
}

// ----------------------------------------
// 描画処理
// ----------------------------------------
void Player::Draw(const Camera& camera)
{
	ImGui::Begin("Player");
	ImGui::DragFloat3("velocity", &velocity_.x);
	ImGui::End();

	// オブジェクト描画
	objectPlayer_->Draw(&camera, texturePlayer_, { 1, 1, 1, 1 });
}

// ----------------------------------------
// 移動処理
// ----------------------------------------
void Player::HandleMove()
{
	// 左スティック入力取得
	Vector2 leftStick = input_->GetPadLeftStick();

	// 入力方向をベースにした目標速度ベクトルを作成
	Vector3 targetVelocity(leftStick.x, 0.0f, leftStick.y);

	if (targetVelocity.Length() > 1.0f) {
		targetVelocity.Normalize();
	}
	targetVelocity *= moveSpeed_;

	// 加減速処理
	Vector3 velocityDelta = targetVelocity - velocity_;

	// 加速または減速
	float adjustmentRate = (targetVelocity.LengthSquared() > 0.01f) ? acceleration : deceleration;

	// 差分に調整率を掛けて、現在速度を更新
	velocity_ += velocityDelta * adjustmentRate;

	// 移動を適用
	objectPlayer_->translate_ += velocity_;
}

// ----------------------------------------
// 向き変更処理
// ----------------------------------------
void Player::HandleRotate()
{
	// 右スティック入力取得
	Vector2 rightStick = input_->GetPadRightStick();

	const float inputThreshold = 0.1f; // 入力の閾値
	const float rotationLerpSpeed = 0.1f; // 回転速度係数

	// 入力があるときのみ回転処理を行う
	if (rightStick.Length() >= inputThreshold) {
		// 入力方向を角度に変換
		float targetRotationY = std::atan2f(rightStick.x, rightStick.y);

		// 差分を計算（-π ~ π に正規化）
		float angleDiff = targetRotationY - rotationY_;
		if (angleDiff > std::numbers::pi_v<float>) {
			angleDiff -= std::numbers::pi_v <float> * 2.0f;
		} else if (angleDiff < -std::numbers::pi_v<float>) {
			angleDiff += std::numbers::pi_v<float> * 2.0f;
		}

		// 回転角度を補間して更新
		rotationY_ += angleDiff * rotationLerpSpeed;

		// 正規化（-π ~ π）
		if (rotationY_ > std::numbers::pi_v<float>) {
			rotationY_ -= std::numbers::pi_v<float> * 2.0f;
		} else if (rotationY_ < -std::numbers::pi_v<float>) {
			rotationY_ += std::numbers::pi_v<float> * 2.0f;
		}

		// クォータニオンを適用
		objectPlayer_->quaternion_ = Quaternion::MakeRotateAxisAngleQuaternion(Vector3(0, 1, 0), rotationY_);
	}
}

// ----------------------------------------
// オブジェクト掴み処理
// 
// memo : プレイヤー前方のコライダーと動かせるオブジェクトの衝突判定を行っている
//		　衝突した状態でRB押下でオブジェクトを掴む。その状態でもう一度RB押下でオブジェクトを離す。
// ----------------------------------------
void Player::HandleObjectPickup(const std::vector<std::unique_ptr<MovableObject>>& movableObjects)
{
	///
	/// RB入力でオブジェクトの掴み状態を切り替える
	/// 
	if (input_->IsPadTriggered(PadButton::iPad_RB)) {
		///
		/// オブジェクトを掴む処理
		/// 
		if (!isHolding_) {
			MovableObject* hitObject = nullptr;

			// オブジェクトとの当たり判定
			for (const auto& obj : movableObjects) {
				// オブジェクトのコライダーを取得
				AABBCollider* objCollider = obj->GetCollider();

				// プレイヤー前方のコライダーとの衝突判定
				if (colliderForward_->IsCollidingWith(objCollider)) {
					// 掴み状態にする
					isHolding_ = true;
					// 掴んだオブジェクトのポインタを保持
					holdingObject_ = obj.get();
					break;
				}
			}
		///
		/// 掴んだオブジェクトを離す処理
		/// 
		} else {
			// オブジェクトのY座標を1に戻す（掴んだ際に上に上がるため）
			Vector3 currentPos = holdingObject_->GetTranslate();
			holdingObject_->SetTranslate(Vector3(currentPos.x, 1.0f, currentPos.z));

			// 掴み状態を解除
			isHolding_ = false;
			// 掴んだオブジェクトを解除
			holdingObject_ = nullptr;
		}
	}



	///
	/// 掴んだオブジェクトの移動処理
	/// 
	if (isHolding_ && holdingObject_) {
		// プレイヤーの位置
		Vector3 playerPos = objectPlayer_->translate_;

		// プレイヤーの向いている方向
		Vector3 forward = GetForwardFromYRotation(objectPlayer_->quaternion_);

		// オブジェクトをプレイヤーの前方少し上に配置
		Vector3 targetPos = playerPos + forward * 2.0f + Vector3(0, 4, 0); // 前方に2.0, 上に4.0

		// 位置を反映
		holdingObject_->SetTranslate(targetPos);
	}
}

// ----------------------------------------
// コライダー更新処理
// ----------------------------------------
void Player::UpdateColliders()
{
	// プレイヤー本体のコライダー更新
	colliderPlayer_->SetHalfExtents((objectPlayer_->GetMax() - objectPlayer_->GetMin()) * 0.5f);
	colliderPlayer_->SetLocalPivot((objectPlayer_->GetMax() + objectPlayer_->GetMin()) * 0.5f);
	colliderPlayer_->SetWorldTransform(objectPlayer_->GetWorldTransform());
	CollisionManager::GetInstance()->RegisterCollider(colliderPlayer_.get());

	// プレイヤー前方のコライダー更新
	colliderForward_->SetWorldTransform(objectPlayer_->GetWorldTransform());
	CollisionManager::GetInstance()->RegisterCollider(colliderForward_.get());
}
