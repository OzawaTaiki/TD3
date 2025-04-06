#define NOMINMAX
#include "ShadowObject.h"

// C++
#include <algorithm>

// Engine
#include <Core/DXCommon/TextureManager/TextureManager.h>
#include <Math/Vector/VectorFunction.h>
#include <Math/Quaternion/Quaternion.h>
#include <Math/Easing.h>

// Externals
#include <imgui.h>

void ShadowObject::Initialize() { 
	object_ = std::make_unique<ObjectModel>("ShadowObject"); 
	object_->Initialize("Cube/cube.obj");
	object_->useQuaternion_ = true;
	texture_ = TextureManager::GetInstance()->Load("white.png");

	collider_ = std::make_unique<OBBCollider>("ShadowObjectCollider");
	collider_->SetLayer("ShadowObject");
	/*判定を取らないオブジェクトを設定*/
	collider_->SetLayerMask("ShadowObject");
	collider_->SetLayerMask("movableObject");
	collider_->SetLayerMask("Wall");
	collider_->SetLayerMask("Tower");

	// OBBColliderの設定
	collider_->SetHalfExtents(collider_->GetHalfExtents());
	collider_->SetLocalPivot(collider_->GetLocalPivot());
	collider_->SetWorldTransform(object_->GetWorldTransform());
}

void ShadowObject::Update(const float maxDistance) {
	object_->Update(); 
	CalculateShadowTransform(maxDistance);

	// SPACE押下で実体化
	HandleAttackInput();

	// OBBColliderの更新
	collider_->SetHalfExtents(collider_->GetHalfExtents());
	collider_->SetLocalPivot(collider_->GetLocalPivot());
	collider_->SetWorldTransform(object_->GetWorldTransform());
	CollisionManager::GetInstance()->RegisterCollider(collider_.get());
}

void ShadowObject::Draw(const Camera& camera) { 
	// アクティブな状態のときのみ描画
	if (this->isActive_) {
		object_->Draw(&camera, texture_, { 0, 0, 0, 1 });
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
		}
	}

	// アニメーション処理
	if (isScaling_) {
		const float kDeltaTime = 1.0f / 60.0f;
		animationTime_ += kDeltaTime;

		// アニメーション割合を計算
		float t = std::min(animationTime_ / animationDuration_, 1.0f);
		// 増加時と戻り時で別のイージング関数を仕様
		float easedT = isReturning_ ? Easing::EaseOutQuad(t) : Easing::EaseInQuad(t); // 戻り時 : 増加時

		// 現在のスケールを計算
		scaleYCurrent_ = scaleYStart_ + (scaleYTarget_ - scaleYStart_) * easedT;
		object_->scale_.y = scaleYCurrent_;

		// アニメーションが完了した場合
		if (t >= 1.0f) {
			if (!isReturning_) {
				// 戻りアニメーションの準備
				isReturning_ = true;
				scaleYStart_ = 5.0f;
				scaleYTarget_ = 1.0f;
				animationTime_ = 0.0f;
				animationDuration_ = scaleDownDuration_; // 戻る時間を設定
			} else {
				// 全アニメーション終了
				isScaling_ = false;
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
