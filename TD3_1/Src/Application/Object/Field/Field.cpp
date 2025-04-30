#include "Field.h"

// Engine
#include <Core/DXCommon/TextureManager/TextureManager.h>

void Field::Initialize(const std::string& filename) {
	object_ = std::make_unique<ObjectModel>("field");
	object_->Initialize("field/field2.obj");
	object_->translate_ = { -2.5f, 2.0f, 0.0f };
	object_->euler_ = { 0.0f, std::numbers::pi_v<float> / 2.0f , 0.0f };
	textureField_ = TextureManager::GetInstance()->Load("game/field.png");
}

void Field::Update() {
	object_->Update();

#ifdef _DEBUG
	ImGui::Begin("field");
	ImGui::DragFloat3("translate", &object_->translate_.x);
	ImGui::DragFloat3("scale", &object_->scale_.x);
	ImGui::DragFloat3("rotate", &object_->euler_.x, 0.01f);
	ImGui::End();
#endif
}

void Field::Draw(const Camera* _camera, const Vector4& _color) {
	object_->Draw(_camera, textureField_ ,{ 1, 1, 1, 1 });
}