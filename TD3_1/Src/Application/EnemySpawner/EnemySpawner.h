#pragma once

// Engine
#include <Features/Model/ObjectModel.h>

// Application
#include <Application/Enemy/Enemy.h>

class Camera;

class EnemySpawner
{
public:
	void Initialize(const Vector3& position, const Vector3 towerPosition);
	void Update();
	void Draw(const Camera* camera);

	void DrawImGui(const std::string& name);

private:
	std::vector<std::unique_ptr<Enemy>> enemies_;

	std::unique_ptr<ObjectModel> object_; // スポナーを可視化するためだけなので消してもよい

	Vector3 towetPositon_;

	float spawnInterval_ = 4.0f;
	float elapsedTime_ = 0.0f;
};

