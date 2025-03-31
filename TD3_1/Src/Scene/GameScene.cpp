#include "GameScene.h"

#include <Features/Model/Manager/ModelManager.h>

GameScene::~GameScene() {
	
}

void GameScene::Initialize() {
	SceneCamera_.Initialize();
	SceneCamera_.translate_ = {0, 30, -40};
	SceneCamera_.rotate_ = {0.66f, 0, 0};
	SceneCamera_.UpdateMatrix();
	debugCamera_.Initialize();

	lineDrawer_ = LineDrawer::GetInstance();
	lineDrawer_->Initialize();
	lineDrawer_->SetCameraPtr(&SceneCamera_);

	input_ = Input::GetInstance();

	ground_ = std::make_unique<ObjectModel>("Ground");
	ground_->Initialize("Tile/Tile.gltf");
	ground_->GetUVTransform().SetScale({100, 100});

	particleManager_ = ParticleManager::GetInstance();

	lights_ = std::make_unique<LightGroup>();
	lights_->Initialize();
	LightingSystem::GetInstance()->SetLightGroup(lights_.get());

	InitializeGameObjects();
}

void GameScene::Update() {
#ifdef _DEBUG

	if (input_->IsKeyTriggered(DIK_RETURN) && input_->IsKeyPressed(DIK_RSHIFT))
		enableDebugCamera_ = !enableDebugCamera_;

#endif // _DEBUG

	ground_->Update();

	UpdateGameObjects();
	HandleObjectDragAndDrop();

	if (enableDebugCamera_) {
		debugCamera_.Update();
		SceneCamera_.matView_ = debugCamera_.matView_;
		SceneCamera_.TransferData();
		particleManager_->Update(debugCamera_.rotate_);
	} else {
		SceneCamera_.Update();
		SceneCamera_.UpdateMatrix();
		particleManager_->Update(SceneCamera_.rotate_);
	}
}

void GameScene::Draw() {
	ModelManager::GetInstance()->PreDrawForObjectModel();

	ground_->Draw(&SceneCamera_, {1, 1, 1, 1});

	DrawGameObjects();
}

void GameScene::DrawShadow() {}

void GameScene::InitializeGameObjects() {

}

void GameScene::UpdateGameObjects() { 
	for (size_t i = 0; i < movableObjects_.size(); i++) {
		movableObjects_[i]->Update();

		// コライダーのサイズをオブジェクトのスケールに応じて更新（見た目おかしいけど当たり判定は正しく取れてる）
		Vector3 halfExtents = (movableObjects_[i]->GetMax() - movableObjects_[i]->GetMin()) * 0.5f;
		halfExtents *= movableObjects_[i]->scale_; // スケールを考慮
		colliders_[i]->SetHalfExtents(halfExtents);

		CollisionManager::GetInstance()->RegisterCollider(colliders_[i].get());
	}

	CollisionManager::GetInstance()->Update();
}

void GameScene::DrawGameObjects() { 
	for (const auto& object : movableObjects_) {
		object->Draw(&SceneCamera_, {1, 1, 1, 1});
	}
}

void GameScene::AddMovableObject(const Vector3& position) { 
	// オブジェクトを生成
	auto object = std::make_unique<ObjectModel>("cube" + std::to_string(movableObjects_.size()));
	object->Initialize("Cube/cube.obj");
	object->translate_ = position;
	object->useQuaternion_ = true;

	// オブジェクト用コライダーを生成
	auto collider = std::make_unique<OBBCollider>();
	collider->SetLayer("cube");
	collider->SetHalfExtents((object->GetMax() - object->GetMin()) * 0.5f);
	collider->SetLocalPivot((object->GetMin() + object->GetMax()) * 0.5f);
	collider->SetWorldTransform(object->GetWorldTransform());

	// 配列に追加
	movableObjects_.push_back(std::move(object));
	colliders_.push_back(std::move(collider));
}

void GameScene::HandleObjectDragAndDrop() {
	///
	///	マウスレイの生成と描画
	/// 
	
	Ray mouseRay = CreateMouseRay();

	// 終点を設定して描画（デバッグ用）
	Vector3 mouseRayEnd = mouseRay.GetOrigin() + mouseRay.GetDirection() * mouseRay.GetLength();
	LineDrawer::GetInstance()->SetColor({ 1.0f, 0.0f, 0.0f, 1.0f });
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
	for (size_t i = 0; i < movableObjects_.size(); i++) {
		if (RayCollisionManager::GetInstance()->RayCast(mouseRay, colliders_[i].get(), hit)) {
			hitObject = movableObjects_[i].get();
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
	ImGui::Begin("hoge");
	if (ImGui::Button("AddObject")) {
		AddMovableObject({0, 1, 0});
	}
	ImGui::Checkbox("isDragging", &isDragging);
	ImGui::End();
}

Ray GameScene::CreateMouseRay()
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
	float tanFovY = std::tanf(SceneCamera_.fovY_ * 0.5f);
	float tanfovX = tanFovY * SceneCamera_.aspectRatio_; // X方向のFOV計算

	// レイの方向を視錐台の形に合わせる
	Vector3 rayDir(ndcX * tanfovX, ndcY * tanFovY, 1.0f);
	rayDir.Normalize();

	// カメラの向いている方向に変換
	Matrix4x4 viewMatrix = MakeRotateMatrix(SceneCamera_.rotate_);
	rayDir = Transform(rayDir, viewMatrix).Normalize();

	// レイの生成
	Ray ray;
	ray.SetOrigin(SceneCamera_.translate_);
	ray.SetDirection(rayDir.Normalize());
	ray.SetLength(100.0f);

	return ray;
}

bool GameScene::IntersectRayWithPlane(const Ray& ray, const Vector3& planeNormal, float planeD, Vector3& outIntersection)
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
