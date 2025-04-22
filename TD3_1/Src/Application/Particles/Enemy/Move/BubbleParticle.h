#pragma once

// Engine
#include <Features/Model/Model.h>

class BubbleParticle
{
public:
	void Initialize();
	void Emit(const Vector3& position);

private:
	// 生成するパーティクル数
	const uint32_t kEmitCount = 1;
	// 使用するモデル（一度読み込む必要があるため）
	Model* model_;
	// 使用するテクスチャ
	uint32_t textureHandle_;
};

