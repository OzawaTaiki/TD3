#include "Field.h"

void Field::Initialize(const std::string& filename) {
	// CSVからマップデータの読み込み
	LoadBlocksFromCSV(filename);
}

void Field::Update() {
	// 全てのブロックを更新
	for (auto& block : blocks_) {
		block->Update();
	}

	// 全てのコライダーを登録
	for (size_t i = 0; i < colliders_.size(); i++) {
		/*CollisionManager::GetInstance()->RegisterCollider(colliders_[i].get());*/
	}
}

void Field::Draw(const Camera* _camera, const Vector4& _color) {
	// 全てのブロックを描画
	for (auto& block : blocks_) {
		block->Draw(_camera, _color);
	}
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
		char comma;

		// CSVのブロックデータのフォーマットを解析
		if (!(ss >> blockType >> comma >> x >> comma >> y >> comma >> z)) {
			std::cerr << "Invalid CSV format: " << line << std::endl;
			continue;
		}

		// ブロックのインスタンスを生成
		auto block = std::make_unique<ObjectModel>("Block_" + std::to_string(blocks_.size()));
		block->Initialize("Cube/cube.obj");
		block->translate_ = Vector3(x, y + 1, z);

		// ブロック用コライダーを生成
		auto collider = std::make_unique<AABBCollider>("BlockCollider_" + std::to_string(colliders_.size()));
		collider->SetLayer("Block");
		collider->SetMinMax(block->GetMin(), block->GetMax());
		collider->SetWorldTransform(block->GetWorldTransform());

		CollisionManager::GetInstance()->RegisterCollider(collider.get());

		// 配列へ追加
		blocks_.push_back(std::move(block));
		colliders_.push_back(std::move(collider));
	}

	file.close();
}
