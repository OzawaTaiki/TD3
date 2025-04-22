#include "Tower.h"

// Engine
#include <Features/Camera/Camera/Camera.h>
#include <Core/DXCommon/TextureManager/TextureManager.h>
#include <Features/Collision/CollisionLayer/CollisionLayerManager.h>
#include <Math/Matrix/MatrixFunction.h>
#include <Math/Vector/VectorFunction.h>
#include <Core/WinApp/WinApp.h>

// Application
#include <Application/Util/SimpleEasing/SimpleEasing.h>

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

	// スプライト初期化
	uint32_t textureHP = TextureManager::GetInstance()->Load("white.png");
	spriteHP_ = Sprite::Create("towerHP", textureHP);
	spriteHP_->Initialize();
	spriteHP_->SetColor(Vector4(0.25f, 1.0f, 0.0f, 1.0f));
}

void Tower::Update()
{
	object_->Update();
	CollisionManager::GetInstance()->RegisterCollider(collider_.get());

	spriteHP_->Update();

	// 終了時に追跡セットのリセット
	processedColliders_.clear();
#ifdef _DEBUG
	ImGui::Begin("tower");
	ImGui::DragFloat3("translate", &object_->translate_.x, 0.01f);
	ImGui::Text("HP : %d", hp_);

	// イージング移動テスト
	if (ImGui::Button("Move Test : float")) {
		SimpleEasing::Animate(object_->translate_.x, object_->translate_.x, 10.0f, Easing::EaseOutExpo, 2.0f);
	}
	if (ImGui::Button("Move Test : Vector3")) {
		SimpleEasing::Animate(object_->translate_, object_->translate_, {5.0f, 1.0f, 10.0f}, Easing::EaseOutExpo, 2.0f);
	}

	ImGui::End();
#endif
}

void Tower::Draw(const Camera& camera)
{
	object_->Draw(&camera, texture_, {1, 1, 1, 1});
}

void Tower::DrawUI(const Camera& camera)
{
	Sprite::PreDraw();

#pragma region タワーHPのスプライト描画（汚いので後で整理）
	// ワールド座標 -> スクリーン座標に変換して、スプライト位置を指定
	Vector3 towerPosition = object_->translate_;
	towerPosition.y = towerPosition.y + 6.0f; // いい感じにタワー上部になるよう指定

	Matrix4x4 worldViewProjMatrix = Multiply(camera.matView_, camera.matProjection_);
	Vector3 screenPosition = Transform(towerPosition, worldViewProjMatrix);
	
	screenPosition.x = (screenPosition.x + 1.0f) * 0.5f * WinApp::kWindowWidth_;
	screenPosition.y = (1.0f - screenPosition.y) * 0.5f * WinApp::kWindowHeight_;

	spriteHP_->translate_ = Vector2(screenPosition.x, screenPosition.y);

	// 残りHPをもとに、スプライトサイズを変更
	const float kMaxWidth = 80.0f;
	const float kMaxHP = 10.0f;
	float currentWidth = kMaxWidth * (static_cast<float>(hp_) / kMaxHP);
	spriteHP_->SetSize({ currentWidth, 15.0f });

	spriteHP_->Draw();
#pragma endregion
}

void Tower::DrawShadow(const Camera& camera)
{
    object_->DrawShadow(&camera, 0);
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
