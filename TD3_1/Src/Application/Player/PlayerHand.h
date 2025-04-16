#pragma once

// Engine
#include <Features/Model/ObjectModel.h>

/// <summary>
/// カーソル位置に手を表示するだけ（後でアニメーションなども追加）
/// </summary>
class PlayerHand {
public:
	void Initialize(const Camera& camera);
	void Update(const Camera& camera);
	void Draw(const Camera& camera);

	// ドラッグ状態のセット
	void SetIsDragging(bool isDragging) { isDragging_ = isDragging; }

private:
	std::unique_ptr<ObjectModel> object_;
	uint32_t texture_;

	/// <summary>
	/// カーソル位置のワールド座標を計算
	/// </summary>
	/// <param name="camera"></param>
	/// <returns></returns>
	Vector3 CalclateCursorPosition(const Camera& camera);

	// クリックでの上昇アニメーション関連
	bool isDragging_;
	float defaultY_;
	float targetY_;
	float currentY_;
	float easingSpeed_;
};
