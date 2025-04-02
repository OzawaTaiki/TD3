#include "EnemySpawner.h"

// Externals
#include "imgui.h"

void EnemySpawner::Initialize(const Vector3& position, const Vector3 towerPosition)
{
	object_ = std::make_unique<ObjectModel>("enemySpawner");
	object_->Initialize("Cube/cube.obj");
	object_->translate_ = position;

	towetPositon_ = towerPosition;
}

void EnemySpawner::Update()
{
	// 自動スポーン
	elapsedTime_ += 1.0f / 60.0f;
	if (elapsedTime_ > spawnInterval_) {
		auto enemy = std::make_unique<NormalEnemy>();
		enemy->Initialize(this->object_->translate_); // スポナー位置からスポーン
		enemy->SetTarget(towetPositon_);              // ターゲットにタワーを設定
		enemies_.push_back(std::move(enemy));

		elapsedTime_ = 0.0f;
	}


	for (auto& enemy : enemies_) {
		enemy->Update();
	}
	// 死亡した敵を削除
	enemies_.erase(std::remove_if(enemies_.begin(), enemies_.end(), 
		[](const std::unique_ptr<Enemy>& enemy) 
		{ return enemy->IsDead(); 
		}), 
		enemies_.end());

#ifdef _DEBUG
	object_->Update();

	//ImGui::Begin("enemySpawner");
	//ImGui::DragFloat3("translate", &object_->translate_.x, 0.01f);

	//if (ImGui::Button("Spawn")) {
	//	auto enemy = std::make_unique<NormalEnemy>();
	//	enemy->Initialize(this->object_->translate_); // スポナー位置からスポーン
	//	enemy->SetTarget(towetPositon_); // ターゲットにタワーを設定
	//	enemies_.push_back(std::move(enemy));
	//}

	//ImGui::End();
#endif 
}

void EnemySpawner::Draw(const Camera* camera)
{
	for (auto& enemy : enemies_) {
		enemy->Draw(camera);
	}

#ifdef _DEBUG
	object_->Draw(camera, { 1, 1, 1, 1 });
#endif 
}

void EnemySpawner::DrawImGui(const std::string& name) {
	ImGui::Begin(name.c_str());
	ImGui::DragFloat3("translate", &object_->translate_.x, 0.01f);

	if (ImGui::Button("Spawn")) {
		auto enemy = std::make_unique<NormalEnemy>();
		enemy->Initialize(this->object_->translate_); // スポナー位置からスポーン
		enemy->SetTarget(towetPositon_);              // ターゲットにタワーを設定
		enemies_.push_back(std::move(enemy));
	}

	ImGui::End();
}
