#include "SmokeParticle.h"

// Engine
#include <Features/Effect/Manager/ParticleSystem.h>
#include <Core/DXCommon/TextureManager/TextureManager.h>
#include <Math/Random/RandomGenerator.h>

void SmokeParticle::Initialize()
{
	// 使用するモデル読み込み
	model_ = Model::CreateFromFile("plane/plane.gltf");
	// 使用するテクスチャ読み込み
	textureHandle_ = TextureManager::GetInstance()->Load("particle/smoke.png");
}

void SmokeParticle::Emit(const Vector3& position)
{
	auto rand = RandomGenerator::GetInstance();

	///
	///	指定数のパーティクルを生成
	/// 
	std::vector<Particle*> particles(kEmitCount);

	for (uint32_t i = 0; i < kEmitCount; i++) {
		///
		///	各パラメーターの設定
		///

		// 初期化データ
		ParticleInitParam initParam;

		// 有効時間
		initParam.lifeTime = 1.0f;
		initParam.isInfiniteLife = false;
		// サイズ
		initParam.size = { 1.0f, 1.0f, 1.0f };
		// 回転
		initParam.rotate = { 0.0f, 0.0f, 0.0f };
		// 位置
		initParam.position = position;
		// スピード
		initParam.speed = 4.0f;
		// 方向
		initParam.direction = rand->GetRandValue({ -1.0f, -1.0f, -1.0f }, { 1.0f, 1.0f, 1.0f });
		// 加速度、重力
		initParam.acceleration = { 0.0f, 0.0f, 0.0f };
		// 色
		initParam.color = { 1.0f, 1.0f, 1.0f, 1.0f };
		// ビルボード
		initParam.isBillboard = { true, true, true };

		///
		///	パーティクルを生成してコンテナに追加
		/// 
		Particle* particle = new Particle();
		particle->Initialize(initParam);

		particles[i] = particle;
	}

	///
	///	描画簡易設定
	/// 
	ParticleRenderSettings settings;
	settings.blendMode = BlendMode::Normal;
	settings.cullBack = false;

	///
	///	使用するモデル名
	/// 
	std::string useModelName = "plane/plane.gltf";

	///
	///	モディファイア
	/// 

	std::vector<std::string> modifiers;
	modifiers.push_back("SmokeModifier");

	///
	///	生成したパーティクルを登録
	/// 

	ParticleSystem::GetInstance()->AddParticles(useModelName, particles, settings, textureHandle_, modifiers);
}