#define NOMINMAX
#include "ShadowObject.h"


// C++
#include <algorithm>

// Engine
#include <Core/DXCommon/TextureManager/TextureManager.h>
#include <Math/Vector/VectorFunction.h>
#include <Math/Quaternion/Quaternion.h>
#include <Math/Easing.h>
#include <System/Audio/Audio.h>

// Application
#include <Application/CameraShake/CameraShake.h>

// Externals
#include <imgui.h>

void ShadowObject::Initialize(float waitDuration) { 
	object_ = std::make_unique<ObjectModel>("ShadowObject"); 
	object_->Initialize("Cube/cube.obj");
	object_->useQuaternion_ = true;
	texture_ = TextureManager::GetInstance()->Load("white.png");


	// 実体化した瞬間のコライダーの設定
	collider_ = std::make_unique<OBBCollider>("ShadowObjectCollider");
	collider_->SetLayer("ShadowObject");
	/*判定を取らないオブジェクトを設定*/
	collider_->SetLayerMask("ShadowObject");
	collider_->SetLayerMask("movableObject");
	collider_->SetLayerMask("Wall");
	collider_->SetLayerMask("Tower");


	// 実体化から元に戻るまでのコライダーの設定
	colliderReturning_ = std::make_unique<OBBCollider>("ShadowObjectReturningCollider");
	colliderReturning_->SetLayer("ShadowObjectReturning");
	/*判定を取らないオブジェクトを設定*/
	colliderReturning_->SetLayerMask("ShadowObjectReturning");
	colliderReturning_->SetLayerMask("movableObject");
	colliderReturning_->SetLayerMask("Wall");
	colliderReturning_->SetLayerMask("Tower");


	waitDuration_ = waitDuration;
}

void ShadowObject::Update(const float maxDistance) {
	object_->Update(); 
	CalculateShadowTransform(maxDistance);

	// 影オブジェクトが有効な場合のみ
	if (this->isActive_) {
		// 実体化した瞬間のコライダー登録
		if (!this->isReturning_) {
			// OBBColliderの更新
			Vector3 halfExtents = (object_->GetMax() - object_->GetMin()) * 0.5f;
			collider_->SetHalfExtents(halfExtents);
			Vector3 localPivot = (object_->GetMax() + object_->GetMin()) * 0.5f;
			collider_->SetLocalPivot(localPivot);
			collider_->SetWorldTransform(object_->GetWorldTransform());
			CollisionManager::GetInstance()->RegisterCollider(collider_.get());
		// 実体化から元に戻るときのコライダー登録
		} else {
			Vector3 halfExtents = (object_->GetMax() - object_->GetMin()) * 0.5f;
			colliderReturning_->SetHalfExtents(halfExtents);
			Vector3 localPivot = (object_->GetMax() + object_->GetMin()) * 0.5f;
			colliderReturning_->SetLocalPivot(localPivot);
			colliderReturning_->SetWorldTransform(object_->GetWorldTransform());
			CollisionManager::GetInstance()->RegisterCollider(colliderReturning_.get());
		}
	}

	// SPACE押下で実体化
	HandleAttackInput();
}

void ShadowObject::Draw(const Camera& camera) { 
	// アクティブな状態のときのみ描画
	if (this->isActive_) {
		/*waitDuration（持続時間）に応じて色の濃さを変える*/
		float clampedWaitDuration = std::clamp(waitDuration_, kBaseWaitDuration, kMaxWaitDuration); // 現在のwaitDurationをクランプして安全に計算

		float t = (clampedWaitDuration - kBaseWaitDuration) / (kMaxWaitDuration);
		float colorIntensity = 0.5f * (1.0f - t);

		Vector4 color = { colorIntensity, colorIntensity, colorIntensity, 1.0f };

		object_->Draw(&camera, texture_, color);
	}
}

void ShadowObject::CalculateShadowTransform(const float maxDistance)
{
	/*ポイントライトの位置を考慮し、動かせるオブジェクトの影の位置に常に影オブジェクトが配置されるように設定*/
	// 動かせるオブジェクトの中心位置
	Vector3 objectCenter = movableObjectPosition_;

	// 動かせるオブジェクトから見たライトの方向を計算
	Vector3 lightDirection = Normalize(objectCenter - lightPosition_);

	// 影の長さを光との距離から決定
	float distanceToLight = Length(objectCenter - lightPosition_);
	float shadowLength = 1.0f + distanceToLight * 0.15f; // どれくらい伸びるかをかける値で設定

	// 距離チェックを行う : ライトから一定距離離れた場合には非アクティブ化する
	if (distanceToLight > maxDistance) {
		this->isActive_ = false;
		return;
	}
	// 距離チェック成功時には有効化
	this->isActive_ = true;

	// 影オブジェクトの基準点をobjectCenterに設定し、ローカルZ+方向に向けてshadowLengthを伸ばす
	this->object_->scale_.z = shadowLength;

	// 光の方向に合わせて回転を設定
	Vector3 shadowDirection = -lightDirection;
	shadowDirection.y = 0.0f;
	shadowDirection = Normalize(shadowDirection);
	this->object_->quaternion_ = Quaternion::FromToRotation(Vector3(0, 0, 1), shadowDirection);

	// オフセット（影オブジェクトの原点が端になるようにずらす）
	Vector3 localOffset = { 0.0f, 0.0f, -shadowLength / 2.0f };

	// 回転を考慮してローカルオフセットをワールド空間に変換
	Matrix4x4 rotationMatrix = MakeRotateMatrix(this->object_->quaternion_);
	Vector3 worldOffset = Transform(localOffset, rotationMatrix);

	// 最終的なワールド座標
	this->object_->translate_ = objectCenter + worldOffset;
	this->object_->translate_.y = -0.99f; // 高さを固定
}

void ShadowObject::HandleAttackInput()
{
	if (Input::GetInstance()->IsKeyTriggered(DIK_SPACE)) {
		if (!isScaling_) {
			isScaling_ = true;
			isReturning_ = false;
			scaleYStart_ = object_->scale_.y; // 現在のスケールを取得
			scaleYTarget_ = 5.0f; // 増加後のスケール
			animationTime_ = 0.0f; // アニメーションタイマーをリセット
			animationDuration_ = scaleUpDuration_; // 増加時間を設定

			// カメラシェイクを行う
			CameraShake::GetInstance()->StartShake();
            // サウンドを再生
            Audio::GetInstance()->SoundPlay(soundHandle_, volume_); // サウンドを再生
		}
	}

	// アニメーション処理
	if (isScaling_) {
		const float kDeltaTime = 1.0f / 60.0f;

		// 最大値まで実体化した際の待機処理
		if (isWaiting_) {
			waitTime_ += kDeltaTime;
			if (waitTime_ >= waitDuration_) {
				isWaiting_ = false;
				isReturning_ = true; // 戻りアニメーション開始
				isWaiting_ = false; // 待機フラグのリセット
				scaleYStart_ = 5.0f;
				scaleYTarget_ = 1.0f;
				animationTime_ = 0.0f; // 拡大->縮小の双方で使いまわしてるためリセット
				animationDuration_ = scaleDownDuration_; // 元に戻るまでの時間を設定
				waitTime_ = 0.0f; // 待機タイマーリセット
			}
		// 拡大 & 縮小の処理
		} else {
			animationTime_ += kDeltaTime;

			// アニメーション割合を計算
			float t = std::min(animationTime_ / animationDuration_, 1.0f);
			float easedT = isReturning_ ? Easing::EaseOutQuad(t) : Easing::EaseOutExpo(t); // 縮小時 : 拡大時

			// 現在のスケールを計算
			scaleYCurrent_ = scaleYStart_ + (scaleYTarget_ - scaleYStart_) * easedT;
			object_->scale_.y = scaleYCurrent_;

			// 拡大・縮小それぞれが完了した場合
			 if (t >= 1.0f) {
				 // 拡大 終了時の処理
				if (!isReturning_) {
					isReturning_ = true;
					scaleYStart_ = 5.0f;
					scaleYTarget_ = 1.0f;
					animationTime_ = 0.0f;
					animationDuration_ = scaleDownDuration_; // 戻る時間を設定
					isWaiting_ = true; // 待機状態にする
				// 縮小 終了時の処理
				} else {
					isScaling_ = false;
				}
			 }
		}
	}
}

Matrix4x4 ShadowObject::MakeRotateMatrix(const Quaternion& q) {
	Matrix4x4 result;

	float x = q.x;
	float y = q.y;
	float z = q.z;
	float w = q.w;

	result.m[0][0] = 1 - 2 * y * y - 2 * z * z;
	result.m[0][1] = 2 * x * y + 2 * w * z;
	result.m[0][2] = 2 * x * z - 2 * w * y;
	result.m[0][3] = 0;

	result.m[1][0] = 2 * x * y - 2 * w * z;
	result.m[1][1] = 1 - 2 * x * x - 2 * z * z;
	result.m[1][2] = 2 * y * z + 2 * w * x;
	result.m[1][3] = 0;

	result.m[2][0] = 2 * x * z + 2 * w * y;
	result.m[2][1] = 2 * y * z - 2 * w * x;
	result.m[2][2] = 1 - 2 * x * x - 2 * y * y;
	result.m[2][3] = 0;

	result.m[3][0] = 0;
	result.m[3][1] = 0;
	result.m[3][2] = 0;
	result.m[3][3] = 1;

	return result;
}
