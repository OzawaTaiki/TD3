#include "Fade.h"

// C++
#include <algorithm>

// Engine
#include <Core/WinApp/WinApp.h>
#include <Core/DXCommon/TextureManager/TextureManager.h>

void Fade::Initialize()
{
	uint32_t textureHandle = TextureManager::GetInstance()->Load("white.png");
	sprite_ = Sprite::Create("fade", textureHandle, Vector2{0.0f, 0.0f});
	sprite_->Initialize();
	sprite_->SetSize(Vector2{ static_cast<float>(WinApp::kWindowWidth_), static_cast<float>(WinApp::kWindowHeight_)});
}

void Fade::Update()
{
	sprite_->Update();

	// フェード状態による分岐
	switch (status_) {
	case Status::None:
		// 何もしない
		break;
	case Status::FadeIn:
		// フェードイン中

		// 1フレーム分の秒数をカウントアップ
		counter_ += 1.0f / 60.0f;
		// フェード継続時間に達したら打ち止め
		if (counter_ >= duration_) {
			counter_ = duration_;
		}
		// 1.0fから0.0fの間で、経過時間がフェード継続時間に近づくほどアルファ値を小さくする
		sprite_->SetColor(Vector4(0.0f, 0.0f, 0.0f, std::clamp(1.0f - counter_ / duration_, 0.0f, 1.0f)));

		break;
	case Status::FadeOut:
		// フェードアウト中

		// 1フレーム分の秒数をカウントアップ
		counter_ += 1.0f / 60.0f;
		// フェード継続時間に達したら打ち止め
		if (counter_ >= duration_) {
			counter_ = duration_;
		}
		// 0.0fから1.0fの間で、経過時間がフェード継続時間に近づくほどアルファ値を大きくする
		sprite_->SetColor(Vector4(0.0f, 0.0f, 0.0f, std::clamp(counter_ / duration_, 0.0f, 1.0f)));

		break;
	}
}

void Fade::Draw()
{
	if (status_ == Status::None) {
		return;
	}

	Sprite::PreDraw();
	sprite_->Draw();
}

void Fade::Start(Status status, float duration)
{
	status_ = status;
	duration_ = duration;
	counter_ = 0.0f;
}

void Fade::Stop()
{
	status_ = Status::None;
}

bool Fade::IsFinished() const
{
	// フェード状態による分岐
	switch (status_) {
	case Status::FadeIn:
	case Status::FadeOut:
		if (counter_ >= duration_) {
			return true;
		} else {
			return false;
		}
	}

	return true;
}