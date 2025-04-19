#include "PointLightObject.h"

// Engine
#include <Core/DXCommon/TextureManager/TextureManager.h>

void PointLightObject::Initialize(Vector3 position, float maxDistance) { 
	objectLight_ = std::make_unique<ObjectModel>("PointLightObject"); 
	objectLight_->Initialize("Sphere/sphere.obj");
	objectLight_->translate_ = position;
	maxDistance_ = maxDistance;
	textureLight_ = TextureManager::GetInstance()->Load("white.png");

	objectCircle_ = std::make_unique<ObjectModel>("CircleObject");
	objectCircle_->Initialize("circle.obj");
	objectCircle_->translate_ = position;
	objectCircle_->translate_.y = 0.01f;
	objectCircle_->euler_.x = -std::numbers::pi_v<float> / 2.0f;
	textureCircle_ = TextureManager::GetInstance()->Load("white.png");
}

void PointLightObject::Update() { 
	objectLight_->Update(); 

	// ライトと位置を同期
	objectCircle_->translate_.x = objectLight_->translate_.x;
	objectCircle_->translate_.z = objectLight_->translate_.z;
	objectCircle_->scale_ = { maxDistance_, maxDistance_, maxDistance_ };
	objectCircle_->Update();
}

void PointLightObject::Draw(const Camera& camera) { 
	objectLight_->Draw(&camera, textureLight_, {1, 1, 0, 1}); 
	objectCircle_->Draw(&camera, textureCircle_, { 0.75f, 0.75f, 0, 1.0f });
}
