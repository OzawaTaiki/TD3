#pragma once

// C++
#include <vector>
#include <memory>

// Application
#include <Application/PointLightObject/PointLightObject.h>

class PointLightObjectManager
{
public:
	void Initialize();
	void Update();
	void Draw(const Camera& camera);

	const std::vector<std::unique_ptr<PointLightObject>>& GetLights() const { return pointLightObjects_; }

	void AddLight();

private:
	std::vector<std::unique_ptr<PointLightObject>> pointLightObjects_;

	// JSONファイルへ保存
	void SaveToFile();
	// JSONファイルから読み込み
	void LoadFromFile();
};

