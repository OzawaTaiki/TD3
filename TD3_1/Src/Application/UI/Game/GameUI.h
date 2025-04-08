#pragma once

// Engine
#include <Features/Sprite/Sprite.h>

class GameUI
{
public:
	void Initialize();
	void Update();
	void Draw();

private:
	std::unique_ptr<Sprite> spriteTextGuide_; // 操作説明（文字）
	std::unique_ptr<Sprite> spriteTextGauge_; // 打ち上げゲージ（文字）
	//std::unique_ptr<Sprite> spriteGauge_; // 打ち上げゲージ（内側 : ゲージ本体）
	//std::unique_ptr<Sprite> spriteGaugeFrame_; // 打ち上げゲージ（外側 : 枠）
};

