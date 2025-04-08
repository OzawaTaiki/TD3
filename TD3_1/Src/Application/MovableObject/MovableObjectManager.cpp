#include "MovableObjectManager.h"

// Engine
#include <Features/Camera/Camera/Camera.h>
#include <Features/Collision/RayCast/RayCollisionManager.h>
#include <Features/LineDrawer/LineDrawer.h>
#include <Features/Event/EventManager.h>
#include <System/Input/Input.h>
#include <Math/Vector/VectorFunction.h>
#include <Math/Matrix/MatrixFunction.h>
#include <Math/Quaternion/Quaternion.h>
#include <Core/DXCommon/TextureManager/TextureManager.h>

// Externals
#include <imgui.h>

// Application
#include <Application/Event/RewardEventData.h>


void MovableObjectManager::Initialize()
{
	input_ = Input::GetInstance();

	texture_ = TextureManager::GetInstance()->Load("game/player/objectBox.png");

	AddMovableObject({ 0, 1, -6 });

    // イベントリスナーの登録
    EventManager::GetInstance()->AddEventListener("GiveReward", this);
}

void MovableObjectManager::Update(const Camera& camera)
{
	// オブジェクト更新
	for (size_t i = 0; i < objects_.size(); i++) {
		objects_[i]->Update();
		CollisionManager::GetInstance()->RegisterCollider(colliders_[i].get()); // コライダー登録


		// ドラッグ中のオブジェクトのみ敵との当たり判定を無効化
		if (draggingObject_ == objects_[i].get()) {
			colliders_[i]->SetLayerMask("enemy");
		} else {
			colliders_[i]->ExcludeLayerMask("enemy");
		}
	}

	// オブジェクトをドラッグアンドドロップで動かす処理
	HandleObjectDragAndDrop(camera);
}

void MovableObjectManager::Draw(const Camera& camera)
{
	// オブジェクト描画
	for (const auto& object : objects_) {
		object->Draw(&camera, texture_, { 1, 1, 1, 1 });
	}
}

void MovableObjectManager::AddMovableObject(const Vector3& position)
{
	// オブジェクトを生成
	auto object = std::make_unique<ObjectModel>("movableObjectManager" + std::to_string(objects_.size()));
	object->Initialize("movableObjects/objectBox.obj");
	object->translate_ = position;
	object->useQuaternion_ = true;

	// オブジェクト用コライダーを生成
	auto collider = std::make_unique<AABBCollider>();
	collider->SetLayer("movableObject");
	collider->SetMinMax(object->GetMin(), object->GetMax());
	collider->SetWorldTransform(object->GetWorldTransform());

	// 配列に追加
	objects_.push_back(std::move(object));
	colliders_.push_back(std::move(collider));
}

std::vector<Vector3> MovableObjectManager::GetAllObjectPosition() const
{
	std::vector<Vector3> positions;
	for (const auto& object : objects_) {
		positions.push_back(object->translate_);
	}
	return positions;
}

void MovableObjectManager::OnEvent(const GameEvent& _event)
{
    // イベントの種類を確認
	if (_event.GetEventType() == "GiveReward") {

		ReawardEventData* eventData = static_cast<ReawardEventData*>(_event.GetData());

		if (eventData->item == RewardItem::MovableObject) {
			// オブジェクトを追加する処理
			AddMovableObject({ 0, 1, -6 });
		}
        else if (eventData->item == RewardItem::Haelth) {
            // ヘルスを追加する処理
            // ここにヘルス追加の処理を書く
        }
		else {
			// その他のアイテムに対する処理
			// ここにその他のアイテムに対する処理を書く
		}
	}
}

void MovableObjectManager::HandleObjectDragAndDrop(const Camera& camera)
{
	///
	///	マウスレイの生成と描画
	///

	Ray mouseRay = CreateMouseRay(camera);

	// 終点を設定して描画（デバッグ用）
	Vector3 mouseRayEnd = mouseRay.GetOrigin() + mouseRay.GetDirection() * mouseRay.GetLength();
	/*LineDrawer::GetInstance()->RegisterPoint(mouseRay.GetOrigin(), mouseRayEnd);*/

	///
	///	オブジェクトをドラッグで移動
	///

	// マウスレイとキューブオブジェクトの衝突判定
	RayCastHit hit;
	ObjectModel* hitObject = nullptr;
	for (size_t i = 0; i < objects_.size(); i++) {
		if (RayCollisionManager::GetInstance()->RayCast(mouseRay, colliders_[i].get(), hit)) {
			hitObject = objects_[i].get();
		}
	}

	// 左クリックした瞬間
	if (input_->IsMouseTriggered(0)) {
		if (hitObject) {
			isDragging_ = true;
			draggingObject_ = hitObject;
			dragStartHeight_ = draggingObject_->translate_.y; // 掴んだ際の高さを記録
			dragOffset_ = draggingObject_->translate_ - hit.point; // マウスとオブジェクトのオフセット計算
		}
	}

	if (isDragging_) {
		if (draggingObject_) {
			// マウスレイとオブジェクトの初期高さの平面との交点を求める
			Vector3 intersection;
			if (IntersectRayWithPlane(mouseRay, Vector3(0, 1, 0), dragStartHeight_, intersection)) {
				draggingObject_->translate_.x = intersection.x + dragOffset_.x;
				draggingObject_->translate_.y = dragStartHeight_;
				draggingObject_->translate_.z = intersection.z + dragOffset_.z;
			}
		}
	}

	// 左クリックを離したら終了
	if (input_->IsMouseReleased(0)) {
		isDragging_ = false;
		draggingObject_ = nullptr; // ドラッグ中オブジェクトをクリア
	}

#ifdef _DEBUG
	ImGui::Begin("movableObject");
	if (ImGui::Button("AddObject")) {
		AddMovableObject({ 0, 1, -6 });
	}
	ImGui::Checkbox("isDragging", &isDragging_);
	ImGui::End();
#endif
}

Ray MovableObjectManager::CreateMouseRay(const Camera& camera)
{
	// マウスカーソル位置の取得
	Vector2 mousePos = input_->GetMousePosition();

	// 0~1の範囲に正規化
	float normalizedX = mousePos.x / WinApp::kWindowWidth_;
	float normalizedY = mousePos.y / WinApp::kWindowHeight_;

	// -1~1の範囲に変換（NDC）
	float ndcX = normalizedX * 2.0f - 1.0f;
	float ndcY = 1.0f - normalizedY * 2.0f;

	// 視錐台の遠近の計算
	float tanFovY = std::tanf(camera.fovY_ * 0.5f);
	float tanfovX = tanFovY * camera.aspectRatio_; // X方向のFOV計算

	// レイの方向を視錐台の形に合わせる
	Vector3 rayDir(ndcX * tanfovX, ndcY * tanFovY, 1.0f);
	rayDir.Normalize();

	// カメラの向いている方向に変換
	Matrix4x4 viewMatrix = MakeRotateMatrix(camera.rotate_);
	rayDir = Transform(rayDir, viewMatrix).Normalize();

	// レイの生成
	Ray ray;
	ray.SetOrigin(camera.translate_);
	ray.SetDirection(rayDir.Normalize());
	ray.SetLength(100.0f);

	return ray;
}

bool MovableObjectManager::IntersectRayWithPlane(const Ray& ray, const Vector3& planeNormal, float planeD, Vector3& outIntersection)
{
	float denom = Dot(ray.GetDirection(), planeNormal);
	if (fabs(denom) > 1e-6f) { // 平行でないことを確認
		float t = (planeD - Dot(ray.GetOrigin(), planeNormal)) / denom;
		if (t >= 0) { // 交差点がレイの正方向にある
			outIntersection = ray.GetOrigin() + ray.GetDirection() * t;
			return true;
		}
	}
	return false;
}
