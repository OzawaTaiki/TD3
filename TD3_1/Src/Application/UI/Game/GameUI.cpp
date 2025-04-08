#include "GameUI.h"

// Engine
#include <Core/DXCommon/TextureManager/TextureManager.h>

void GameUI::Initialize()
{
	// 操作説明（文字）
	uint32_t textureTextGuide = TextureManager::GetInstance()->Load("game/ui/textGuide.png");
	spriteTextGuide_ = Sprite::Create("textGuide", textureTextGuide);
	spriteTextGuide_->Initialize();
	spriteTextGuide_->translate_ = { 250, 85 };

	// 打ち上げゲージ（文字）
	uint32_t textureTextGauge = TextureManager::GetInstance()->Load("game/ui/textGauge.png");
	spriteTextGauge_ = Sprite::Create("textGauge", textureTextGauge);
	spriteTextGauge_->Initialize();
	spriteTextGauge_->translate_ = { 1015, 45 };

	//// 打ち上げゲージ（内側 : ゲージ本体）
	//uint32_t textureGauge = TextureManager::GetInstance()->Load("game/ui/gauge.png");
	//spriteGauge_ = Sprite::Create("gauge", textureGauge);
	//spriteGauge_->translate_ = { 1015, 125 };

	//// 打ち上げゲージ（外側 : 枠）
	//uint32_t textureGaugeFrame = TextureManager::GetInstance()->Load("game/ui/gaugeFrame.png");
	//spriteGaugeFrame_ = Sprite::Create("gaugeFrame", textureGaugeFrame);
	//spriteGaugeFrame_->translate_ = { 1015, 125 };
}

void GameUI::Update()
{
	spriteTextGuide_->Update();
	spriteTextGauge_->Update();
	/*spriteGauge_->Update();
	spriteGaugeFrame_->Update();*/
}

void GameUI::Draw()
{
	Sprite::PreDraw();

	spriteTextGuide_->Draw();
	spriteTextGauge_->Draw();
	/*spriteGauge_->Draw();
	spriteGaugeFrame_->Draw();*/
}
