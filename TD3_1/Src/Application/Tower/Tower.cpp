#include "Tower.h"

// Engine
#include <Features/Camera/Camera/Camera.h>
#include <Core/DXCommon/TextureManager/TextureManager.h>

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
}

void Tower::Update()
{
	object_->Update();
	CollisionManager::GetInstance()->RegisterCollider(collider_.get());

#ifdef _DEBUG
	ImGui::Begin("tower");
	ImGui::DragFloat3("translate", &object_->translate_.x, 0.01f);
	ImGui::End();
#endif
}

void Tower::Draw(const Camera& camera)
{
	object_->Draw(&camera, texture_, {1, 1, 1, 1});
}
