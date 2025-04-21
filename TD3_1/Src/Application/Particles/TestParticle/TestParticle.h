#pragma once

// Engine
#include <Features/Effect/Manager/ParticleSystem.h>
#include <Features/Model/Model.h>

/// <summary>
/// テスト用パーティクル
/// </summary>
class TestParticle {
public:
	void Initialize();
	void Emit();
	void Update();

private:
	// 生成するパーティクル数
	const uint32_t kEmitCount = 10;
	// 使用するモデル（一度読み込む必要があるため）
	Model* model_;
	// 使用するテクスチャ
	uint32_t textureHandle_;
};
