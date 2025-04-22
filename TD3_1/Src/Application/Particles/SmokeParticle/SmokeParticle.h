#pragma once

#include <Features/Model/Model.h>

/// <summary>
/// タワーが敵に攻撃された際に発生させるパーティクル
/// </summary>
class SmokeParticle
{
public:
	void Initialize();
	void Emit(const Vector3& position);

private:
	// 生成するパーティクル数
	const uint32_t kEmitCount = 10;
	// 使用するモデル（一度読み込む必要があるため）
	Model* model_;
	// 使用するテクスチャ
	uint32_t textureHandle_;
};

