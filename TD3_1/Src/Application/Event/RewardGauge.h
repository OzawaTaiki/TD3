#pragma once

#include <Features/Event/EventListener.h>
#include <Features/UI/UISprite.h>
#include <Features/Json/JsonBinder.h>

#include <Application/Event/RewardEventData.h>

#include <memory>

struct RewardGaugeData
{
    uint32_t count = 0;
    RewardItem item = RewardItem::None;
    bool isGiven = false;

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

    void Reset();

    // Eventを発行
    void EmitEvent();

private:

    void ImGui();

    void InitJsonBinder();

    void SaveJsonBinder();

    std::unique_ptr<UISprite> gauge_ = nullptr;
    std::unique_ptr<UISprite> gaugeFrame_ = nullptr;

    Vector2 defaultGaugeSize_ = { 0, 0 }; // ゲージのデフォルトサイズ

    uint32_t count_ = 0; // ゲージのカウント

    // 報酬を受け取るのに必要なカウント
    uint32_t rewardCooldown = 0;

    // カウントとそのカウントでの報酬アイテム
    std::vector<RewardGaugeData> rewardGaugeData_;
    std::vector<RewardGaugeData>::iterator nextRewardGaugeData_;

    std::unique_ptr<JsonBinder> jsonBinder_;

    uint32_t gaugeMaxSoundHandle_ = 0; // サウンドハンドル
    uint32_t gaugeUpSoundHandle_ = 0; // サウンドハンドル
    float gaugeMaxSoundVolume_ = 0.5f; // デフォルトのボリュームを設定
    float gaugeUpSoundVolume_ = 0.5f; // デフォルトのボリュームを設定

};