#pragma once

#include <Features/UI/UISprite.h>

#include <Features/Json/JsonBinder.h>

class ScoreSprites
{
public:
    ScoreSprites() = default;
    ~ScoreSprites() = default;

    void Initialize();
    void Update();
    void Draw();

private:

    std::unique_ptr<UISprite> backGroud_ = nullptr;

    std::unique_ptr<UISprite> sensekiText_ = nullptr;
    std::unique_ptr<UISprite> utiageCountText_= nullptr;
    std::unique_ptr<UISprite> waveCountText_ = nullptr;
    std::unique_ptr<UISprite> otohimeCountText = nullptr;

    std::vector<std::unique_ptr<UISprite>> scoreSprites_ = {};

    std::vector<Vector2> uiStandardPos_ = {};

    std::unique_ptr<JsonBinder> jsonBinder_ = nullptr;

};