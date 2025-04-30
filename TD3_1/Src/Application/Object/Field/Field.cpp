#include "Field.h"

// Engine
#include <Core/DXCommon/TextureManager/TextureManager.h>

void Field::Initialize(const std::string& filename) {
	// CSVからマップデータの読み込み
	LoadBlocksFromCSV(filename);

	texture_ = TextureManager::GetInstance()->Load("cube.jpg");

	object_ = std::make_unique<ObjectModel>("field");
	object_->Initialize("field/field2.obj");
	object_->translate_ = { -2.5f, 2.0f, 0.0f };
	object_->euler_ = { 0.0f, std::numbers::pi_v<float> / 2.0f , 0.0f };
	textureField_ = TextureManager::GetInstance()->Load("game/field.png");
}

void Field::Update() {
	// 全てのブロックを更新
	for (auto& block : blocks_) {
		block->Update();
	}

	// 全てのコライダーを登録
	for (size_t i = 0; i < colliders_.size(); i++) {
		CollisionManager::GetInstance()->RegisterCollider(colliders_[i].get());
	}

	object_->Update();

#ifdef _DEBUG
	ImGui::Begin("field");
	ImGui::DragFloat3("translate", &object_->translate_.x);
	ImGui::DragFloat3("scale", &object_->scale_.x);
	ImGui::DragFloat3("rotate", &object_->euler_.x, 0.01f);
	ImGui::End();
#endif
}

void Field::Draw(const Camera* _camera, const Vector4& _color) {
	// 全てのブロックを描画
	/*for (auto& block : blocks_) {
		block->Draw(_camera, texture_, _color);
	}*/

	object_->Draw(_camera, textureField_ ,{ 1, 1, 1, 1 });
}

void Field::LoadBlocksFromCSV(const std::string& filename) { 
	std::ifstream file("Resources/Maps/" + filename); 
	if (!file) {
		std::cerr << "Failed to open CSV file: " << filename << std::endl;
		return;
	}

	std::string line;
	while (std::getline(file, line)) {
		// '{'と'}'を除去する
		line.erase(std::remove(line.begin(), line.end(), '{'), line.end());
		line.erase(std::remove(line.begin(), line.end(), '}'), line.end());

		// fを除去する
		line.erase(std::remove(line.begin(), line.end(), 'f'), line.end());

		std::stringstream ss(line);
		uint32_t blockType;
		float x, y, z;
		float scaleX, scaleY, scaleZ;
		char comma;

		// CSVのブロックデータのフォーマットを解析
		if (!(ss >> blockType >> comma >> x >> comma >> y >> comma >> z >> comma >> scaleX >> comma >> scaleY >> comma >> scaleZ)) {
			std::cerr << "Invalid CSV format: " << line << std::endl;
			continue;
		}

		// ブロックのインスタンスを生成
		auto block = std::make_unique<ObjectModel>("Block_" + std::to_string(blocks_.size()));
		block->Initialize("Cube/cube.obj");
		block->translate_ = Vector3(x, y + 1, z);
		block->scale_ = Vector3(scaleX, scaleZ, scaleY); // blenderに合わせてYとZを入れ替え

		// ブロック用コライダーを生成
		auto collider = std::make_unique<AABBCollider>("WallCollider_" + std::to_string(colliders_.size()));
		collider->SetLayer("Wall");
		collider->SetCollisionLayer("none");
		collider->SetMinMax(block->GetMin(), block->GetMax());
		collider->SetWorldTransform(block->GetWorldTransform());

		CollisionManager::GetInstance()->RegisterCollider(collider.get());

		// 配列へ追加
		blocks_.push_back(std::move(block));
		colliders_.push_back(std::move(collider));
	}

	file.close();
}
