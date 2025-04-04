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
}

void ShadowObject::Update() { 
	object_->Update(); 

	/*ポイントライトの位置を考慮し、動かせるオブジェクトの影の位置に常に影オブジェクトが配置されるように設定*/

	// 動かせるオブジェクトから見たライトの方向を計算
	Vector3 lightDirection = Normalize(movableObjectPosition_ - lightPosition_);
	// 影オブジェクトの位置を設定
	float shadowDistance = 5.0f;
	Vector3 shadowPosition = movableObjectPosition_ + lightDirection * shadowDistance;
	shadowPosition.y = -0.99f; // 高さは固定（実体化攻撃実装時にここは変更予定）
	this->object_->translate_ = shadowPosition;
	// 影オブジェクトがライトから離れるほど伸びるように設定
	float distanceToLight = Length(this->object_->translate_ - lightPosition_);
	this->object_->scale_.z = 1.0f + distanceToLight * 0.1f;
	// 影オブジェクトが動かせるオブジェクトの方向を向くように設定
	Vector3 directionToMovableObject = Normalize(movableObjectPosition_ - this->object_->translate_);
	directionToMovableObject.y = 0.0f;
	this->object_->quaternion_ = Quaternion::FromToRotation(Vector3(0, 0, 1), directionToMovableObject);

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
