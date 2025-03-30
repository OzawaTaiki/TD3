#include "GameScene.h"

#include <Features/Model/Manager/ModelManager.h>

GameScene::~GameScene() {
	delete colliderObjectCube_;
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

	///
	/// ゲームシーン用
	///

	objectCube_->Update();
}

void GameScene::Draw() {
	ModelManager::GetInstance()->PreDrawForObjectModel();

	ground_->Draw(&SceneCamera_, {1, 1, 1, 1});

	DrawGameObjects();
}

void GameScene::DrawShadow() {}

void GameScene::InitializeGameObjects() {
	// キューブオブジェクトの生成
	objectCube_ = std::make_unique<ObjectModel>("cube");
	objectCube_->Initialize("Cube/cube.obj");
	objectCube_->translate_.y = 1.0f;
	objectCube_->useQuaternion_ = true;
	
	// キューブオブジェクト用のOBBColliderを生成
	colliderObjectCube_ = new OBBCollider();
	colliderObjectCube_->SetLayer("cube");

	Vector3 localMin = objectCube_->GetMin();
	Vector3 localMax = objectCube_->GetMax();

	Vector3 halfExtents = (localMax - localMin) * 0.5f;
	colliderObjectCube_->SetHalfExtents(halfExtents); // 半分の大きさを計算してセット

	Vector3 localPivot = (localMin + localMax) * 0.5f;
	colliderObjectCube_->SetLocalPivot(localPivot); // 基準点を計算してセット

	colliderObjectCube_->SetWorldTransform(objectCube_->GetWorldTransform());
}

void GameScene::UpdateGameObjects() { 
	objectCube_->Update(); 
	CollisionManager::GetInstance()->RegisterCollider(colliderObjectCube_);

	CollisionManager::GetInstance()->Update();
}

void GameScene::DrawGameObjects() { 
	objectCube_->Draw(&SceneCamera_, {1, 1, 1, 1}); 
}

void GameScene::HandleObjectDragAndDrop() {
	///
	///	マウスレイとオブジェクトの衝突判定
	/// 
	
	// マウスレイの生成
	Ray mouseRay = CreateMouseRay();

	// マウスレイの描画（デバッグ用）
	Vector3 mouseRayEnd = mouseRay.GetOrigin() + mouseRay.GetDirection() * mouseRay.GetLength();
	LineDrawer::GetInstance()->SetColor({ 1.0f, 0.0f, 0.0f, 1.0f });
	LineDrawer::GetInstance()->RegisterPoint(mouseRay.GetOrigin(), mouseRayEnd);

	// コライダー登録
	RayCollisionManager::GetInstance()->RegisterCollider(colliderObjectCube_);

	// マウスレイとキューブオブジェクトの衝突判定
	RayCastHit hit;
	bool isHit = RayCollisionManager::GetInstance()->RayCast(mouseRay, colliderObjectCube_, hit);

	///
	///	オブジェクトをドラッグで移動
	/// 

	static bool isDragging = false;
	static Vector3 dragOffset;
	static float dragStartHeight = 0.0f; // オブジェクトの元の高さを保持

	// 左クリックした瞬間
	if (input_->IsMouseTriggered(0)) {
		if (isHit) {
			isDragging = true;
			dragStartHeight = objectCube_->translate_.y; // 高さを記録
			dragOffset = objectCube_->translate_ - hit.point; // マウスとオブジェクトのオフセット計算
		}
	}

	if (isDragging) {
		// マウスレイとオブジェクトの初期高さの平面との交点を求める
		Vector3 intersection;
		if (IntersectRayWithPlane(mouseRay, Vector3(0, 1, 0), dragStartHeight, intersection)) {
			objectCube_->translate_.x = intersection.x + dragOffset.x;
			objectCube_->translate_.y = dragStartHeight;
			objectCube_->translate_.z = intersection.z + dragOffset.z;
		}
	}

	// 左クリックを離したら終了
	if (input_->IsMouseReleased(0)) {
		isDragging = false;
	}

	// デバッグ表示
	ImGui::Begin("hoge");
	ImGui::Checkbox("isHit", &isHit);
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
