#include "MovableObject.h"

// Engine
#include <Core/DXCommon/TextureManager/TextureManager.h>
#include <Features/Collision/CollisionLayer/CollisionLayerManager.h>

void BoxObject::Initialize(float _hp) {
	// オブジェクト生成
	object_ = std::make_unique<ObjectModel>("boxObject");
	object_->Initialize("movableObjects/objectBox.obj");
	object_->useQuaternion_ = true;

	// コライダー生成
	collider_ = std::make_unique<AABBCollider>();
	collider_->Initialize();
	collider_->SetLayer("movableObject");
	collider_->SetMinMax(object_->GetMin(), object_->GetMax());
	collider_->SetWorldTransform(object_->GetWorldTransform());
	collider_->SetOnCollisionCallback([this](Collider* _other, const ColliderInfo& _info) {
		OnCollision(_other, _info);
		});

	// テクスチャ読み込み
	texture_ = TextureManager::GetInstance()->Load("game/player/objectBox.png");

	hp_ = _hp;
}

void BoxObject::Update() {
	object_->Update();
	CollisionManager::GetInstance()->RegisterCollider(collider_.get());

	ImGui::Begin("a");
	ImGui::Checkbox("withMovable", &isCollidingWithMovableObject_);
	ImGui::End();
}

void BoxObject::Draw(const Camera& camera) {

	Vector4 color = isHit_ ? Vector4(1.0f, 0.6f, 0.6f, 1) : Vector4(1, 1, 1, 1);

	object_->Draw(&camera, texture_, color);
	isHit_ = false;
}



void CylinderObject::Initialize(float _hp) {
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

    hp_ = _hp;
}

void CylinderObject::Update() {
	object_->Update();
	CollisionManager::GetInstance()->RegisterCollider(collider_.get());
}

void CylinderObject::Draw(const Camera& camera) {

    Vector4 color = isHit_ ? Vector4(1.0f, 0.6f, 0.6f, 1) : Vector4(1, 1, 1, 1);

    object_->Draw(&camera, texture_, color);
}

void MovableObject::OnCollision(Collider* _other, const ColliderInfo& _info)
{
	// movableObjectとの衝突
	uint32_t movableObjectLayer = CollisionLayerManager::GetInstance()->GetLayer("movableObject"); // レイヤー取得
	if (_other->GetLayer() == movableObjectLayer) {
		isCollidingWithMovableObject_ = true;
	} else {
		isCollidingWithMovableObject_ = false;
	}
}

void MovableObject::Damage(const std::string& _name, float _damage)
{
	if (collider_->GetName() == _name)
	{
        // ダメージを受けた場合の処理
        // ここにダメージ処理を書く
        hp_ -= _damage;

        if (hp_ <= 0.0f)
        {
            isDead_ = true; // 死亡フラグを立てる
        }
		else
            isHit_ = true; // 衝突フラグを立てる
	}

}
