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

    defaultGaugeSize_ = gauge_->GetSize(); // ゲージのデフォルトサイズを取得
    gauge_->SetSize({ 0, defaultGaugeSize_.y }); // ゲージのサイズを初期化

    gaugeFrame_ = std::make_unique<UISprite>();
    gaugeFrame_->Initialize("RewardGaugeFrame");

    count_ = 0;

    RewardGaugeData data2 = { .count = 3, .item = RewardItem::MovableObject, .isGiven = false };
    RewardGaugeData data3 = { .count = 6, .item = RewardItem::MovableObject, .isGiven = false };
    rewardGaugeData_.push_back(data2);
    rewardGaugeData_.push_back(data3);

    nextRewardGaugeData_ = rewardGaugeData_.begin();
    rewardCooldown = nextRewardGaugeData_->count;

    InitJsonBinder();
}

void RewardGauge::Update()
{
    gauge_->Update();
    gaugeFrame_->Update();

    EmitEvent();
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
        if (nextRewardGaugeData_ == rewardGaugeData_.end())
        {
            gauge_->SetSize(defaultGaugeSize_);
            return;
        }

        ++count_;

        float ratio = static_cast<float>( nextRewardGaugeData_->count - count_) / static_cast<float>(rewardCooldown);
        float scaledX = defaultGaugeSize_.x * (1.0f - ratio);
        gauge_->SetSize({ scaledX, defaultGaugeSize_.y });
    }
}

void RewardGauge::EmitEvent()
{
    // 最後のアイテムを受け取っていたらリターン
    if (nextRewardGaugeData_ == rewardGaugeData_.end())
        return;

    // 次の報酬を受け取れないならリターン
    if (count_ < nextRewardGaugeData_->count)
        return;

    // 報酬を受け取る
    nextRewardGaugeData_->isGiven = true;

    uint32_t count = nextRewardGaugeData_->count;

    // 次の報酬を受け取るためにイテレータを進める
    // エラーが出たら嫌なので念のため分岐
    if (nextRewardGaugeData_ + 1 != rewardGaugeData_.end())
    {
        ++nextRewardGaugeData_;
        rewardCooldown = nextRewardGaugeData_->count - count; // 次の報酬までのカウントを更新
    }
    else
    {
        nextRewardGaugeData_ = rewardGaugeData_.end();
        rewardCooldown = 0;
    }

    gauge_->SetSize({ 0, defaultGaugeSize_.y });

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
