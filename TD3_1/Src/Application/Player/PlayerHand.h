#pragma once

// Engine
#include <Features/Model/ObjectModel.h>

/// <summary>
/// カーソル位置に手を表示するだけ（後でアニメーションなども追加）
/// </summary>
class PlayerHand {
public:
	void Initialize();
	void Update(const Camera& camera);
	void Draw(const Camera& camera);

private:
	std::unique_ptr<ObjectModel> object_;
	uint32_t texture_;
};
