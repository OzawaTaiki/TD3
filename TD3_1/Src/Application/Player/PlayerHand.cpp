#include "PlayerHand.h"

// Engine
#include <Core/DXCommon/TextureManager/TextureManager.h>
#include <System/Input/Input.h>
#include <Core/WinApp/WinApp.h>
#include <Math/Matrix/MatrixFunction.h>
#include <Math/Vector/VectorFunction.h>

void PlayerHand::Initialize() { 
	object_ = std::make_unique<ObjectModel>("hand"); 
	object_->Initialize("Hand/hand.obj");

	texture_ = TextureManager::GetInstance()->Load("game/player/hand.png");
}

void PlayerHand::Update(const Camera& camera) { 
	object_->Update();

	// マウス座標を取得
	Vector2 mousePos = Input::GetInstance()->GetMousePosition();
	// 0~1の範囲に正規化
	float normalizeX = mousePos.x / WinApp::kWindowWidth_;
	float normalizeY = mousePos.y / WinApp::kWindowHeight_;
	// -1~1の範囲に変換（NDC）
	float ndcX = normalizeX * 2.0f - 1.0f;
	float ndcY = 1.0f - normalizeY * 2.0f;
	// 視錐台の遠近の計算
	float tanFovY = std::tanf(camera.fovY_ * 0.5f);
	float tanFovX = tanFovY * camera.aspectRatio_;
	// レイ方向の計算
	Vector3 rayDir(ndcX * tanFovX, ndcY * tanFovY, 1.0f);
	rayDir.Normalize();
	// カメラの回転行列を取得
	Matrix4x4 viewMatrix = MakeRotateMatrix(camera.rotate_);
	rayDir = Transform(rayDir, viewMatrix).Normalize();
	// レイの原点から固定した距離で位置を計算
	Vector3 handPosition = camera.translate_ + rayDir * 50.0f;

	object_->translate_ = handPosition;
}

void PlayerHand::Draw(const Camera& camera) { 
	object_->Draw(&camera, texture_, {1, 1, 1, 1}); 
}
