#include "PointLightObject.h"

// Engine
#include <Core/DXCommon/TextureManager/TextureManager.h>

void PointLightObject::Initialize(Vector3 position) { 
	object_ = std::make_unique<ObjectModel>("PointLightObject"); 
	object_->Initialize("Sphere/sphere.obj");
	object_->translate_ = position;
	texture_ = TextureManager::GetInstance()->Load("white.png");
}

void PointLightObject::Update() { 
	object_->Update(); 
}

void PointLightObject::Draw(const Camera& camera) { 
	object_->Draw(&camera, texture_, {1, 1, 0, 1}); 
}
