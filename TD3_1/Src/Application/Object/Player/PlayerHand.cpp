#include "PlayerHand.h"

// Engine
#include <Core/DXCommon/TextureManager/TextureManager.h>
#include <System/Input/Input.h>
#include <Core/WinApp/WinApp.h>
#include <Math/Matrix/MatrixFunction.h>
#include <Math/Vector/VectorFunction.h>

void PlayerHand::Initialize(const Camera& camera) { 
	object_ = std::make_unique<ObjectModel>("hand"); 
	object_->Initialize("Hand/player.obj");
	object_->euler_.y = -std::numbers::pi_v<float>;

	texture_ = TextureManager::GetInstance()->Load("game/player.png");

	targetY_ = defaultY_;
	currentY_ = defaultY_;
	easingSpeed_ = 0.5f;
}

void PlayerHand::Update(const Camera& camera) { 
	object_->Update();
	// object_->translate_ = CalclateCursorPosition(camera);

	// 左クリックで上昇
	if (Input::GetInstance()->IsMousePressed(0)) {
		// オブジェクトを掴んでいる状態であれば
		if (isDragging_) {
			targetY_ = defaultY_ + 2.0f; // 上昇する値の設定
		}
	// 左クリックを離したら元の高さへ
	} else {
		targetY_ = defaultY_;
	}

	defaultY_ = CalclateCursorPosition(camera).y; // 現在のY座標取得
	currentY_ += (targetY_ - currentY_) * easingSpeed_; // イージングによるY座標の更新
	Vector3 cursorPosition = CalclateCursorPosition(camera); // カーソル位置の計算
	cursorPosition.y = currentY_; // Y座標を上書き

	object_->translate_ = cursorPosition;
	
#ifdef _DEBUG
	ImGui::Begin("HandObject");
	ImGui::Text("Translate: x:%.2f, y:%.2f, z:%.2f", object_->translate_.x, object_->translate_.y, object_->translate_.z);
	ImGui::Checkbox("isDragging", &isDragging_);
	ImGui::End();
#endif
}

void PlayerHand::Draw(const Camera& camera) { 
	object_->Draw(&camera, texture_, {1, 1, 1, 1}); 
}

Vector3 PlayerHand::CalclateCursorPosition(const Camera& camera) { 
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

	return handPosition;
}
