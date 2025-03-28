#include "GameScene.h"

#include <Features/Model/Manager/ModelManager.h>

GameScene::~GameScene() {}

void GameScene::Initialize() {
	SceneCamera_.Initialize();
	SceneCamera_.translate_ = {0, 8, -30};
	SceneCamera_.rotate_ = {0.26f, 0, 0};
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

	ImGui::Begin("TrackStatus");

	ImGui::Checkbox("isHoldingObject", &isHoldingObject_);

	Vector2 mousePosition = input_->GetMousePosition();
	ImGui::Text("mousePosition(Screen) : x:%.f y:%.f", mousePosition.x, mousePosition.y);

	ImGui::End();

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
	objectCube_ = std::make_unique<ObjectModel>("cube");
	objectCube_->Initialize("Cube/cube.obj");
	objectCube_->translate_.y = 1.0f;
}

void GameScene::UpdateGameObjects() { objectCube_->Update(); }

void GameScene::DrawGameObjects() { objectCube_->Draw(&SceneCamera_, {1, 1, 1, 1}); }

void GameScene::HandleObjectDragAndDrop() {
#pragma region 一時的にコメントアウト
	//   // memo : たぶんめっちゃ長くなるので各工程をあとで整理

	//   ///
	//   /// 1.マウスのワールド座標を取得
	//   ///
	//
	//   // *マウスのスクリーン座標を取得
	//   Vector2 mousePosition = input_->GetMousePosition();

	//   // スクリーン座標を[-1, 1]の範囲に正規化
	// float x = (2.0f * mousePosition.x) / WinApp::kWindowWidth_ - 1.0f;
	// float y = 1.0f - (2.0f * mousePosition.y) / WinApp::kWindowHeight_;

	//   // *正規化したスクリーン座標 -> クリップ空間へ変換
	// Vector4 clipSpace(x, y, 1.0f, 1.0f);

	//   // viewProjectionの逆行列を取得
	// Matrix4x4 inverseViewProjection = Inverse(SceneCamera_.GetViewProjection());

	//   // *クリップ空間 -> ビュー空間へ逆変換（投影空間 -> カメラ空間）
	///*Vector4 worldSpace = inverseViewProjection * clipSpace;*/

	//   // memo : Matrix4x4 * Vector4が追加され次第、以下部分を削除して上記のコメントアウトを外す
	//   Matrix4x4 m = inverseViewProjection;
	// Vector4 v = clipSpace;

	//   Vector4 worldSpace = {
	//    m.m[0][0] * v.x + m.m[0][1] * v.y + m.m[0][2] * v.z + m.m[0][3] * v.w,
	//    m.m[1][0] * v.x + m.m[1][1] * v.y + m.m[1][2] * v.z + m.m[1][3] * v.w,
	//    m.m[2][0] * v.x + m.m[2][1] * v.y + m.m[2][2] * v.z + m.m[2][3] * v.w,
	//    m.m[3][0] * v.x + m.m[3][1] * v.y + m.m[3][2] * v.z + m.m[3][3] * v.w,
	//};

	//   // *カメラ空間 -> ワールド空間へ変換
	// Vector3 rayOrigin = SceneCamera_.translate_;
	// Vector3 rayDirection = Vector3(worldSpace.x, worldSpace.y, worldSpace.z) - rayOrigin;
	// rayDirection = Normalize(rayDirection);

	//   ///
	//   /// 2.クリックを押した場合、カーソルの位置にあるオブジェクトを取得
	//   ///

	//   if (input_->IsMousePressed(0) && !isHoldingObject_) {
	//	isHoldingObject_ = true;
	//	grabbedObject_ = objectCube_.get();
	//   }

	//   ///
	//   /// 3.クリックしている間、カーソルを動かすとオブジェクト移動
	//   ///

	//   ///
	//   /// 4.クリックを離した場合、その場にオブジェクトを設置
	//   ///
	//
	//   if (input_->IsMouseReleased(0) && isHoldingObject_) {
	//	isHoldingObject_ = false;
	//	grabbedObject_ = nullptr;
	//   }

	//   // デバッグ表示
	//   ImGui::Begin("hoge");

	// ImGui::Text("mouseWorldPos : x:%.1f, y:%.1f, z:%.1f", worldSpace.x, worldSpace.y, worldSpace.z);
	// ImGui::Text("rayDirection : x:%.1f, y:%.1f, z:%.1f", rayDirection.x, rayDirection.y, rayDirection.z);

	// if (grabbedObject_) {
	//	ImGui::Text("grabbedObjectPosition : x:%.1f, y:%.1f, z:%.1f", grabbedObject_->translate_.x, grabbedObject_->translate_.y, grabbedObject_->translate_.z);
	// }

	// ImGui::End();
#pragma endregion

#pragma region マウスのレイを描画したいザウルス
	//// マウスのスクリーン座標を取得
	//Vector2 mouseScreenPosition = input_->GetMousePosition();

	//// スクリーン座標 -> 正規化デバイス座標への変換
	//float x_ndc = (2.0f * mouseScreenPosition.x / WinApp::kWindowWidth_) - 1.0f;
	//float y_ndc = 1.0f - (2.0f * mouseScreenPosition.y / WinApp::kWindowHeight_);

	//// NDC -> ビュー空間への変換
	//Matrix4x4 inverseProjection = Inverse(SceneCamera_.matProjection_);
	//Vector4 clipSpace = Vector4(x_ndc, y_ndc, -1.0f, 1.0f);
	//
	///*Matrix4x4 * Vector4ができるようになったら修正（↓この1行だけでよくなる） */
	////Vector4 viewSpace = inverseProjection * clipSpace;

	//Matrix4x4 m1 = inverseProjection;
	//Vector4 v1 = clipSpace;
	//Vector4 viewSpace = {
	//    m1.m[0][0] * v1.x + m1.m[0][1] * v1.y + m1.m[0][2] * v1.z + m1.m[0][3] * v1.w,
	//    m1.m[1][0] * v1.x + m1.m[1][1] * v1.y + m1.m[1][2] * v1.z + m1.m[1][3] * v1.w,
	//    m1.m[2][0] * v1.x + m1.m[2][1] * v1.y + m1.m[2][2] * v1.z + m1.m[2][3] * v1.w,
	//    m1.m[3][0] * v1.x + m1.m[3][1] * v1.y + m1.m[3][2] * v1.z + m1.m[3][3] * v1.w,
	//};

	//// ビュー行列 -> ワールド空間への変換
	//Matrix4x4 inverseView = Inverse(SceneCamera_.matView_);

	///*Matrix4x4 * Vector4ができるようになったら修正（↓この1行だけでよくなる） */
	///*Vector4 worldSpace = inverseView * viewSpace;*/

	//Matrix4x4 m2 = inverseView;
	//Vector4 v2 = viewSpace;

	//Vector4 worldSpace = {
	//    m2.m[0][0] * v2.x + m2.m[0][1] * v2.y + m2.m[0][2] * v2.z + m2.m[0][3] * v2.w,
	//    m2.m[1][0] * v2.x + m2.m[1][1] * v2.y + m2.m[1][2] * v2.z + m2.m[1][3] * v2.w,
	//    m2.m[2][0] * v2.x + m2.m[2][1] * v2.y + m2.m[2][2] * v2.z + m2.m[2][3] * v2.w,
	//    m2.m[3][0] * v2.x + m2.m[3][1] * v2.y + m2.m[3][2] * v2.z + m2.m[3][3] * v2.w,
	//};

	//// レイの始点と方向
	//Vector3 rayOrigin = SceneCamera_.translate_;
	//Vector3 rayDiff = Vector3(worldSpace.x, worldSpace.y, worldSpace.z) - SceneCamera_.translate_;

	//// 描画用に終点を設定
	//Vector3 rayEnd = rayOrigin + rayDiff * 100.0f;

	//// マウスのレイを描画
	//LineDrawer::GetInstance()->RegisterPoint(rayOrigin, rayEnd);

	//ImGui::Begin("hoge");
	//
	//ImGui::DragFloat3("rayOrigin", &rayOrigin.x);
	//ImGui::DragFloat3("rayDiff", &rayDiff.x);
	//
	//ImGui::DragFloat3("rayEnd", &rayEnd.x);

	//ImGui::End();

#pragma endregion

#pragma region マウスのレイを描画
	// マウスの位置を取得
	Vector2 mousePos = input_->GetMousePosition();

	// NDCに変換
	float ndcX = (2.0f * mousePos.x / WinApp::kWindowWidth_) - 1.0f;
	float ndcY = 1.0f - (2.0f * mousePos.y / WinApp::kWindowHeight_);

	// レイのクリップ空間での開始点と終了点
	Vector4 rayStartNDC = Vector4(ndcX, ndcY, 0.0f, 1.0f);
	Vector4 rayEndNDC = Vector4(ndcX, ndcY, 1.0f, 1.0f);

	// ビュープロジェクションの逆行列を取得
	Matrix4x4 inverseViewProjectionMatrix = Inverse(SceneCamera_.GetViewProjection());

	// レイをワールド空間に変換
	Vector4 rayStartWorld = Transform(inverseViewProjectionMatrix, rayStartNDC);
	Vector4 rayEndWorld = Transform(inverseViewProjectionMatrix, rayEndNDC);

	// レイの方向を計算
	Vector3 rayDirection = Vector3

#pragma endregion
}

Vector4 GameScene::Transform(const Matrix4x4& mat, const Vector4& vec) {
	Vector4 result;
	result.x = mat.m[0][0] * vec.x + mat.m[0][1] * vec.y + mat.m[0][2] * vec.z + mat.m[0][3] * vec.w;
	result.y = mat.m[1][0] * vec.x + mat.m[1][1] * vec.y + mat.m[1][2] * vec.z + mat.m[1][3] * vec.w;
	result.z = mat.m[2][0] * vec.x + mat.m[2][1] * vec.y + mat.m[2][2] * vec.z + mat.m[2][3] * vec.w;
	result.w = mat.m[3][0] * vec.x + mat.m[3][1] * vec.y + mat.m[3][2] * vec.z + mat.m[3][3] * vec.w;
	return result;
}
