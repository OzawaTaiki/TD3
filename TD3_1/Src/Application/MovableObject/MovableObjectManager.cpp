#include "MovableObjectManager.h"

// Engine
#include <Features/Camera/Camera/Camera.h>
#include <Features/Collision/RayCast/RayCollisionManager.h>
#include <Features/LineDrawer/LineDrawer.h>
#include <System/Input/Input.h>
#include <Math/Vector/VectorFunction.h>
#include <Math/Matrix/MatrixFunction.h>

// Externals
#include <imgui.h>

void MobableObjectManager::Initialize()
{
	input_ = Input::GetInstance();
}

void MobableObjectManager::Update(const Camera& camera)
{
	// オブジェクト更新
	for (size_t i = 0; i < objects_.size(); i++) {
		objects_[i]->Update();
		CollisionManager::GetInstance()->RegisterCollider(colliders_[i].get()); // コライダー登録
	}

	// オブジェクトをドラッグアンドドロップで動かす処理
	HandleObjectDragAndDrop(camera);
}

void MobableObjectManager::Draw(const Camera& camera)
{
	// オブジェクト描画
	for (const auto& object : objects_) {
		object->Draw(&camera, { 1, 1, 1, 1 });
	}
}

void MobableObjectManager::AddMovableObject(const Vector3& position)
{
	// オブジェクトを生成
	auto object = std::make_unique<ObjectModel>("cube" + std::to_string(objects_.size()));
	object->Initialize("Cube/cube.obj");
	object->translate_ = position;
	object->useQuaternion_ = true;

	// オブジェクト用コライダーを生成
	auto collider = std::make_unique<AABBCollider>();
	collider->SetLayer("cube");
	collider->SetMinMax(object->GetMin(), object->GetMax());
	collider->SetWorldTransform(object->GetWorldTransform());

	// 配列に追加
	objects_.push_back(std::move(object));
	colliders_.push_back(std::move(collider));
}

void MobableObjectManager::HandleObjectDragAndDrop(const Camera& camera)
{
	///
	///	マウスレイの生成と描画
	/// 

	Ray mouseRay = CreateMouseRay(camera);

	// 終点を設定して描画（デバッグ用）
	Vector3 mouseRayEnd = mouseRay.GetOrigin() + mouseRay.GetDirection() * mouseRay.GetLength();
	LineDrawer::GetInstance()->RegisterPoint(mouseRay.GetOrigin(), mouseRayEnd);

	///
	///	オブジェクトをドラッグで移動
	///

	static bool isDragging = false;
	static Vector3 dragOffset;
	static float dragStartHeight = 0.0f;          // オブジェクトの元の高さを保持
	static ObjectModel* draggingObject = nullptr; // ドラッグ中のオブジェクト

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
			isDragging = true;
			draggingObject = hitObject;
			dragStartHeight = draggingObject->translate_.y; // 掴んだ際の高さを記録
			dragOffset = draggingObject->translate_ - hit.point; // マウスとオブジェクトのオフセット計算
		}
	}

	if (isDragging) {
		if (draggingObject) {
			// マウスレイとオブジェクトの初期高さの平面との交点を求める
			Vector3 intersection;
			if (IntersectRayWithPlane(mouseRay, Vector3(0, 1, 0), dragStartHeight, intersection)) {
				draggingObject->translate_.x = intersection.x + dragOffset.x;
				draggingObject->translate_.y = dragStartHeight;
				draggingObject->translate_.z = intersection.z + dragOffset.z;
			}
		}
	}

	// 左クリックを離したら終了
	if (input_->IsMouseReleased(0)) {
		isDragging = false;
	}

	// デバッグ表示
	ImGui::Begin("movableObject");
	ImGui::Text("fps:%.2f", ImGui::GetIO().Framerate);
	if (ImGui::Button("AddObject")) {
		AddMovableObject({ 0, 1, 0 });
	}
	ImGui::Checkbox("isDragging", &isDragging);
	ImGui::End();
}

Ray MobableObjectManager::CreateMouseRay(const Camera& camera)
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

bool MobableObjectManager::IntersectRayWithPlane(const Ray& ray, const Vector3& planeNormal, float planeD, Vector3& outIntersection)
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
