#include "ScoreSprites.h"

void ScoreSprites::Initialize()
{
    sensekiText_ = std::make_unique<UISprite>();
    sensekiText_->Initialize("sensekiText");

    utiageCountText_ = std::make_unique<UISprite>();
    utiageCountText_->Initialize("utiageCountText");

    waveCountText_ = std::make_unique<UISprite>();
    waveCountText_->Initialize("waveCountText");

    otohimeCountText = std::make_unique<UISprite>();
    otohimeCountText->Initialize("otohimeCountText");

    backGroud_ = std::make_unique<UISprite>();
    backGroud_->Initialize("ResultBackGround");

    jsonBinder_ = std::make_unique<JsonBinder>("scoreSprites", "Resources/Data/Result/");
    jsonBinder_->RegisterVariable("StandardPos", &uiStandardPos_);

}

void ScoreSprites::Update()
{
    backGroud_->Update();
    sensekiText_->Update();
    utiageCountText_->Update();
    waveCountText_->Update();
    otohimeCountText->Update();

    for (auto& sprite : scoreSprites_) {
        sprite->Update();
    }
}

void ScoreSprites::Draw()
{
    Sprite::PreDraw();

    backGroud_->Draw();
    sensekiText_->Draw();
    utiageCountText_->Draw();
    waveCountText_->Draw();
    otohimeCountText->Draw();
    for (auto& sprite : scoreSprites_) {
        sprite->Draw();
    }
}
