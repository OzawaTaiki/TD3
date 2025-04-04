#include "ShadowObject.h"

// Engine
#include <Core/DXCommon/TextureManager/TextureManager.h>
#include <Math/Vector/VectorFunction.h>
#include <Math/Quaternion/Quaternion.h>

// Externals
#include <imgui.h>

void ShadowObject::Initialize() { 
	object_ = std::make_unique<ObjectModel>("ShadowObject"); 
	object_->Initialize("Cube/cube.obj");
	object_->useQuaternion_ = true;
	texture_ = TextureManager::GetInstance()->Load("white.png");

	collider_ = std::make_unique<OBBCollider>("ShadowObjectCollider");
	collider_->SetLayer("ShadowObject");
	collider_->SetLayerMask("movableObject"); // 動かせるオブジェクトとは判定を取らない

	// OBBColliderの設定
	Vector3 localMin = object_->GetMin();
	Vector3 localMax = object_->GetMax();
	Vector3 halfExtents = (localMax - localMin) * 0.5f;
	collider_->SetHalfExtents(halfExtents);
	Vector3 localPivot = (localMin + localMax) * 0.5f;
	collider_->SetLocalPivot(localPivot);
	collider_->SetWorldTransform(object_->GetWorldTransform());
}

void ShadowObject::Update() { 
	object_->Update(); 

	/*ポイントライトの位置を考慮し、動かせるオブジェクトの影の位置に常に影オブジェクトが配置されるように設定*/
	// 動かせるオブジェクトの中心位置
	Vector3 objectCenter = movableObjectPosition_;

	// 動かせるオブジェクトから見たライトの方向を計算
	Vector3 lightDirection = Normalize(objectCenter - lightPosition_);

	// 影の長さを光との距離から決定
	float distanceToLight = Length(objectCenter - lightPosition_);
	float shadowLength = 1.0f + distanceToLight * 0.15f; // どれくらい伸びるかをかける値で設定

	// 影オブジェクトの基準点をobjectCenterに設定し、ローカルZ+方向に向けてshadowLengthを伸ばす
	this->object_->scale_.z = shadowLength;

	// 光の方向に合わせて回転を設定
	Vector3 shadowDirection = -lightDirection;
	shadowDirection.y = 0.0f;
	shadowDirection = Normalize(shadowDirection);
	this->object_->quaternion_ = Quaternion::FromToRotation(Vector3(0, 0, 1), shadowDirection);

	// オフセット（影オブジェクトの原点が端になるようにずらす）
	Vector3 localOffset = {0.0f, 0.0f, -shadowLength / 2.0f};

	// 回転を考慮してローカルオフセットをワールド空間に変換
	Matrix4x4 rotationMatrix = MakeRotateMatrix(this->object_->quaternion_);
	Vector3 worldOffset = Transform(localOffset, rotationMatrix);

	// 最終的なワールド座標
	this->object_->translate_ = objectCenter + worldOffset;
	this->object_->translate_.y = -0.99f; // 高さを固定



	// OBBColliderの更新
	Vector3 localMin = object_->GetMin();
	Vector3 localMax = object_->GetMax();
	Vector3 halfExtents = (localMax - localMin) * 0.5f;
	collider_->SetHalfExtents(halfExtents);
	Vector3 localPivot = (localMin + localMax) * 0.5f;
	collider_->SetLocalPivot(localPivot);
	collider_->SetWorldTransform(object_->GetWorldTransform());

	/*CollisionManager::GetInstance()->RegisterCollider(collider_.get());*/

#ifdef _DEBUG
	ImGui::Begin("shadowObject");
	ImGui::DragFloat3("translate", &this->object_->translate_.x);
	ImGui::DragFloat3("quaternion", &this->object_->quaternion_.x);
	ImGui::DragFloat3("scale", &this->object_->scale_.x);
	ImGui::End();
#endif
}

void ShadowObject::Draw(const Camera& camera) { 
	object_->Draw(&camera, texture_, {0, 0, 0, 1}); 
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
