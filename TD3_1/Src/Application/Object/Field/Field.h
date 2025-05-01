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
	
private:
	// フィールドオブジェクト
	std::unique_ptr<ObjectModel> object_;
	uint32_t textureField_;
};
