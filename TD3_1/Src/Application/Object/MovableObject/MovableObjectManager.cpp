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
#include <System/Audio/Audio.h>

// Externals
#include <imgui.h>

// Application
#include <Application/Event/RewardEventData.h>
#include <Application/Enemy/EnemyAttackInfo.h>

MovableObjectManager::~MovableObjectManager()
{
    // イベントリスナーの登録解除
    EventManager::GetInstance()->RemoveEventListener("GiveReward", this);
    EventManager::GetInstance()->RemoveEventListener("EnemyAttack", this);


#ifdef _DEBUG
    EventManager::GetInstance()->RemoveEventListener("ResetEnemyManager", this);
#endif // _DEBUG
}

void MovableObjectManager::Initialize(const Camera& camera) {
	input_ = Input::GetInstance();

	texture_ = TextureManager::GetInstance()->Load("game/player/objectBox.png");

	AddMovableObject({ 0, 1, -6 });

    Audio* audio = Audio::GetInstance();

    haveSoundHandle_ = audio->SoundLoadWave("Resources/audio/have.wav");
    putSoundHandle_ = audio->SoundLoadWave("Resources/audio/put.wav");

    // イベントリスナーの登録
    EventManager::GetInstance()->AddEventListener("GiveReward", this);
    EventManager::GetInstance()->AddEventListener("EnemyAttack", this);

#ifdef _DEBUG
    EventManager::GetInstance()->AddEventListener("ResetEnemyManager", this);
#endif // _DEBUG
}

void MovableObjectManager::Update(const Camera& camera)
{
	// オブジェクト更新
	for (size_t i = 0; i < objects_.size(); i++) {
		objects_[i]->Update();

		// ドラッグ中のオブジェクトと敵は判定を取らないように
		if (isDragging_) {
			objects_[i]->SetLayerMask("enemy");
			objects_[i]->SetLayerMask("enemy_forward");
		} else {
			objects_[i]->ExcludeLayerMask("enemy");
			objects_[i]->ExcludeLayerMask("enemy_forward");
		}
	}

	// オブジェクトをドラッグアンドドロップで動かす処理
	/*HandleObjectDragAndDrop(camera);*/
}

void MovableObjectManager::Draw(const Camera& camera)
{
	// オブジェクト描画
	for (const auto& object : objects_) {
		object->Draw(camera);
	}
}

void MovableObjectManager::DrawShadow(const Camera& camera)
{
    // オブジェクト影描画
    for (const auto& object : objects_) {
        object->DrawShadow(camera);
    }
}

void MovableObjectManager::AddMovableObject(const Vector3& position)
{
	// オブジェクトを生成（一旦箱型オブジェクトだけ）
	auto object = std::make_unique<BoxObject>();
	object->Initialize(objectHp_);
	object->SetTranslate(position);

	// 配列に追加
	objects_.push_back(std::move(object));
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
	else if (_event.GetEventType() == "EnemyAttack")
	{
        EnemyAttackInfo* attackInfo = static_cast<EnemyAttackInfo*>(_event.GetData());
        // 攻撃対象のオブジェクト名を取得
        std::string targetObjectName = attackInfo->name;
        // 攻撃対象のオブジェクトを検索

        for (auto it = objects_.begin(); it != objects_.end(); ++it) {
            if ((*it)->GetCollider()->GetName() == targetObjectName) {
                // 攻撃対象のオブジェクトにダメージを与える
                (*it)->Damage(attackInfo->name, attackInfo->damage);
                // HPが0以下になったらオブジェクトを削除
                if ((*it)->IsDead()) {
                    // オブジェクトを削除
                    it = objects_.erase(it);
                    // 削除したのでループを抜ける
                    break;
                }
                break;
            }
        }
	}

#ifdef _DEBUG
	if (_event.GetEventType() == "ResetEnemyManager") {
		// 敵マネージャーのリセット処理
		objects_.clear();
		AddMovableObject({ 0, 1, -6 });
	}
#endif // ?DEBUG
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

	// マウスレイとオブジェクトのコライダーとの衝突判定
	RayCastHit hit;
	MovableObject* hitObject = nullptr;
	for (size_t i = 0; i < objects_.size(); i++) {
		if (RayCollisionManager::GetInstance()->RayCast(mouseRay, objects_[i]->GetCollider(), hit)) {
			hitObject = objects_[i].get();
		}
	}

	// 左クリックした瞬間
	if (input_->IsMouseTriggered(0)) {
		if (hitObject) {
			// 衝突したオブジェクトが動かせない場合には処理をスキップ
			if (!hitObject->CanMove()) {
				return;
			}

			isDragging_ = true;
			draggingObject_ = hitObject;
			//dragStartHeight_ = draggingObject_->GetTranslate().y; // 掴んだ際の高さを記録
			dragStartHeight_ = 1.0f;
			dragOffset_ = draggingObject_->GetTranslate() - hit.point; // マウスとオブジェクトのオフセット計算

			// 掴んだ際に少し上昇させる処理
			targetY_ = dragStartHeight_ + 2.0f; // 上昇する値の設定
			currentY_ = dragStartHeight_;

            Audio::GetInstance()->SoundPlay(haveSoundHandle_,haveSoundVolume_); // サウンドを再生
		}
	}

	// ドラッグ中のオブジェクト位置を更新
	if (isDragging_) {
		if (draggingObject_) {
			// 掴んだ際に少し上昇させる処理
			currentY_ += (targetY_ - currentY_) * 0.2f;

			// マウスレイとオブジェクトの初期高さの平面との交点を求める
			Vector3 intersection;
			if (IntersectRayWithPlane(mouseRay, Vector3(0, 1, 0), dragStartHeight_, intersection)) {
				draggingObject_->SetTranslate(
				{
					intersection.x + dragOffset_.x,
					currentY_,
					intersection.z + dragOffset_.z}
				);
			}
		}
	}

	// 左クリックを離したら終了
	if (input_->IsMouseReleased(0)) {
		if (isDragging_) {
			targetY_ = dragStartHeight_;
			draggingObject_->SetTranslateY(dragStartHeight_); // 離した際に元の高さへ戻るように

			Audio::GetInstance()->SoundPlay(putSoundHandle_, putSoundVolume_); // サウンドを再生
		}
		isDragging_ = false;
		draggingObject_ = nullptr; // ドラッグ中オブジェクトをクリア

	}

#ifdef _DEBUG
	ImGui::Begin("movableObject");

	if (ImGui::Button("AddObject")) {
		AddMovableObject({ 0, 1, -6 });
	}
	for (size_t i = 0; i < objects_.size(); ++i) {
		// オブジェクト番号表示
		std::string labelNum = "Object[" + std::to_string(i) + "]";
		ImGui::Text(labelNum.c_str());

		// オブジェクト座標表示（非活性化していじれないように）
		Vector3 translate = objects_[i]->GetTranslate();
		ImGui::BeginDisabled(true); ImGui::DragFloat3("", &translate.x); ImGui::EndDisabled(); ImGui::SameLine();

		// オブジェクト削除ボタン
		std::string labelDel = "Delete##" + std::to_string(i);
		if (ImGui::Button(labelDel.c_str())) {
			objects_.erase(objects_.begin() + i);
		}
	}
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
