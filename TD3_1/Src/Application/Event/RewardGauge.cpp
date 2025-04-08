#include "RewardGauge.h"
#include <Features/Event/EventManager.h>

RewardGauge::RewardGauge()
{
    // イベントリスナーの登録
    EventManager::GetInstance()->AddEventListener("EnemyLaunchKill", this);
}

RewardGauge::~RewardGauge()
{
}

void RewardGauge::Initialize()
{
    gauge_ = std::make_unique<UISprite>();
    gauge_->Initialize("RewardGauge");

    gaugeFrame_ = std::make_unique<UISprite>();
    gaugeFrame_->Initialize("RewardGaugeFrame");

    count_ = 0;

    RewardGaugeData data = { .count = 1, .item = RewardItem::MovableObject, .isGiven = false };
    RewardGaugeData data2 = { .count = 3, .item = RewardItem::MovableObject, .isGiven = false };
    RewardGaugeData data3 = { .count = 5, .item = RewardItem::MovableObject, .isGiven = false };
    rewardGaugeData_.push_back(data);
    rewardGaugeData_.push_back(data2);
    rewardGaugeData_.push_back(data3);

    nextRewardGaugeData_ = rewardGaugeData_.begin();

    InitJsonBinder();
}

void RewardGauge::Update()
{
    gauge_->Update();
    gaugeFrame_->Update();
}

void RewardGauge::Draw()
{
    gaugeFrame_->Draw();
    gauge_->Draw();
}

void RewardGauge::OnEvent(const GameEvent& _event)
{   // イベントの種類を確認
    // 敵を倒したときに報酬ゲージを更新
    if (_event.GetEventType() == "EnemyLaunchKill")
    {
        ++count_;
        gauge_->SetSize({ static_cast<float>(count_), 1.0f });
    }
}

void RewardGauge::EmitEvent()
{
    if (nextRewardGaugeData_ == rewardGaugeData_.end())
        return;


    EventManager::GetInstance()->DispatchEvent(GameEvent("GiveReward", nullptr));
}

void RewardGauge::InitJsonBinder()
{
    jsonBinder_ = std::make_unique<JsonBinder>("RewardGauge", "Resources/Data/");

    std::vector<uint32_t> rewardGaugeCount;
    std::vector<uint32_t> rewardGaugeItem;
    jsonBinder_->RegisterVariable("RewardCount", &rewardGaugeCount);
    jsonBinder_->RegisterVariable("RewardItem", &rewardGaugeItem);

    if (rewardGaugeCount.size() < rewardGaugeItem.size())
    {
        throw std::runtime_error("RewardGaugeの報酬アイテムの数がカウントの数より多いです。");
        return;
    }

    size_t i = 0;
    for (; i < rewardGaugeCount.size(); ++i)
    {
        RewardGaugeData data;
        data.count = rewardGaugeCount[i];
        data.isGiven = false;

        if (i > rewardGaugeItem.size()) // 報酬アイテムが設定されていなかったら
            data.item = static_cast<RewardItem>(0);
        else
            data.item = static_cast<RewardItem>(rewardGaugeItem[i]);

        rewardGaugeData_.push_back(data);
    }
}
