#include "Tower.h"

// Engine
#include <Features/Camera/Camera/Camera.h>
#include <Core/DXCommon/TextureManager/TextureManager.h>
#include <Features/Collision/CollisionLayer/CollisionLayerManager.h>

// Externals
#include <imgui.h>

void Tower::Initialize(const Vector3& position)
{
	object_ = std::make_unique<ObjectModel>("tower");
	object_->Initialize("Tower/tower.obj");
	object_->translate_ = position;

	texture_ = TextureManager::GetInstance()->Load("tower.png");
	
	collider_ = std::make_unique<AABBCollider>("TowerCollider");
	collider_->SetLayer("Tower");
	collider_->SetMinMax(object_->GetMin(), object_->GetMax());
	collider_->SetWorldTransform(object_->GetWorldTransform());
	collider_->SetOnCollisionCallback([this](Collider* _other, const ColliderInfo& _info) {
		OnCollision(_other, _info);
		});
}

void Tower::Update()
{
	object_->Update();
	CollisionManager::GetInstance()->RegisterCollider(collider_.get());

	// 終了時に追跡セットのリセット
	processedColliders_.clear();
#ifdef _DEBUG
	ImGui::Begin("tower");
	ImGui::DragFloat3("translate", &object_->translate_.x, 0.01f);
	ImGui::Text("HP : %d", hp_);
	ImGui::End();
#endif
}

void Tower::Draw(const Camera& camera)
{
	object_->Draw(&camera, texture_, {1, 1, 1, 1});
}

void Tower::OnCollision(Collider* _other, const ColliderInfo& _info)
{
	// 敵との衝突時処理
	uint32_t enemyLayer = CollisionLayerManager::GetInstance()->GetLayer("enemy");
	if (_other->GetLayer() == enemyLayer) {
		// 処理済みであれば無視
		if (processedColliders_.find(_other) == processedColliders_.end()) {
			this->hp_--; // HPを減らす
			processedColliders_.insert(_other); // 処理済みであることを記録
		}
	}
}
