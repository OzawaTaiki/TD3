#pragma once

#include <Features/Event/EventListener.h>
#include <Features/UI/UISprite.h>
#include <Features/Json/JsonBinder.h>

#include <Application/Event/RewardEventData.h>

#include <memory>

struct RewardGaugeData
{
    uint32_t count;
    RewardItem item;
    bool isGiven;
};

class RewardGauge : public iEventListener
{
public:

    RewardGauge();
    ~RewardGauge() override;

    void Initialize();

    void Update();

    void Draw();

    void OnEvent(const GameEvent& _event) override;

    // Eventを発行
    void EmitEvent();

private:

    void InitJsonBinder();

    std::unique_ptr<UISprite> gauge_ = nullptr;
    std::unique_ptr<UISprite> gaugeFrame_ = nullptr;

    uint32_t count_ = 0; // ゲージのカウント

    // カウントとそのカウントでの報酬アイテム
    std::vector<RewardGaugeData> rewardGaugeData_;
    std::vector<RewardGaugeData>::iterator nextRewardGaugeData_;

    std::unique_ptr<JsonBinder> jsonBinder_;


};