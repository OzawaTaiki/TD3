#include "MovableObject.h"

// Engine
#include <Core/DXCommon/TextureManager/TextureManager.h>

void BoxObject::Initialize() {
	// オブジェクト生成
	object_ = std::make_unique<ObjectModel>("boxObject");
	object_->Initialize("movableObjects/objectBox.obj");
	object_->useQuaternion_ = true;

	// コライダー生成
	collider_ = std::make_unique<AABBCollider>();
	collider_->Initialize();
	collider_->SetLayer("movableObject");
	collider_->SetLayerMask("movableObject");
	collider_->SetMinMax(object_->GetMin(), object_->GetMax());
	collider_->SetWorldTransform(object_->GetWorldTransform());

	// テクスチャ読み込み
	texture_ = TextureManager::GetInstance()->Load("game/player/objectBox.png");
}

void BoxObject::Update() {
	object_->Update();
	CollisionManager::GetInstance()->RegisterCollider(collider_.get());
}

void BoxObject::Draw(const Camera& camera) {
	object_->Draw(&camera, texture_, {1, 1, 1, 1});
}



void CylinderObject::Initialize() {
	// オブジェクト生成
	object_ = std::make_unique<ObjectModel>("cylinderObject");
	object_->Initialize("movableObjects/objectCylinder.obj");
	object_->useQuaternion_ = true;

	// コライダー生成
	collider_ = std::make_unique<AABBCollider>();
	collider_->Initialize();
	collider_->SetLayer("movableObject");
	collider_->SetLayerMask("movableObject");
	collider_->SetMinMax(object_->GetMin(), object_->GetMax());
	collider_->SetWorldTransform(object_->GetWorldTransform());

	// テクスチャ読み込み
	texture_ = TextureManager::GetInstance()->Load("game/player/objectCylinder.png");
}

void CylinderObject::Update() {
	object_->Update();
	CollisionManager::GetInstance()->RegisterCollider(collider_.get());
}

void CylinderObject::Draw(const Camera& camera) {
	object_->Draw(&camera, texture_, {1, 1, 1, 1});
}

bool MovableObject::Damage(const std::string& _name, float _damage)
{
    //if(collider_->GetName() == _name)

}
