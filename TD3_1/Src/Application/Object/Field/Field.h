#pragma once

// Engine
#include <Features/Model/ObjectModel.h>
#include <Features/Collision/Manager/CollisionManager.h>

// C++
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>
#include <iostream>

class Field {
public:
	// 基本的な関数
	void Initialize(const std::string& filename);
	void Update();
	void Draw(const Camera* _camera, const Vector4& _color);
	
	// CSVファイルからブロック情報を読んで配列に格納
	void LoadBlocksFromCSV(const std::string& filename);

private:
	/// <summary>
	/// ブロックオブジェクトの配列
	/// </summary>
	std::vector<std::unique_ptr<ObjectModel>> blocks_;

	/// <summary>
	/// ブロックオブジェクト用コライダーの配列
	/// </summary>
	std::vector<std::unique_ptr<AABBCollider>> colliders_;

	uint32_t texture_;

	// フィールドオブジェクト
	std::unique_ptr<ObjectModel> object_;
	uint32_t textureField_;
};
