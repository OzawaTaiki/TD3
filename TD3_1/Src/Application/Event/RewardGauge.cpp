#include "RewardGauge.h"
#include <Features/Event/EventManager.h>
#include <System/Audio/Audio.h>

RewardGauge::RewardGauge()
{
    // イベントリスナーの登録
    EventManager::GetInstance()->AddEventListener("EnemyLaunchKill", this);
#ifdef _DEBUG
    EventManager::GetInstance()->AddEventListener("ResetEnemyManager", this);
#endif // _DEBUG

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

    InitJsonBinder();

    nextRewardGaugeData_ = rewardGaugeData_.begin();
    rewardCooldown = nextRewardGaugeData_->count;

    gaugeMaxSoundHandle_ = Audio::GetInstance()->SoundLoadWave("Resources/audio/gaugeMax.wav");
    gaugeUpSoundHandle_ = Audio::GetInstance()->SoundLoadWave("Resources/audio/gaugeUp.wav");

}

void RewardGauge::Update()
{
    ImGui();

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
        Audio::GetInstance()->SoundPlay(gaugeUpSoundHandle_, gaugeMaxSoundVolume_); // ゲージが上がったときのサウンドを再生
    }

#ifdef _DEBUG
    if (_event.GetEventType() == "ResetEnemyManager")
    {
        Reset();
    }
#endif // _DEBUG

}

void RewardGauge::Reset()
{
    count_ = 0;
    gauge_->SetSize({ 0, defaultGaugeSize_.y }); // ゲージのサイズを初期化

    nextRewardGaugeData_ = rewardGaugeData_.begin();
    rewardCooldown = nextRewardGaugeData_->count;
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

    gauge_->SetSize({ 0, defaultGaugeSize_.y });

    ReawardEventData eventData(nextRewardGaugeData_->item, count);

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

    Audio::GetInstance()->SoundPlay(gaugeMaxSoundHandle_, gaugeMaxSoundVolume_); // ゲージが最大になったときのサウンドを再生
    EventManager::GetInstance()->DispatchEvent(GameEvent("GiveReward", &eventData));
}

void RewardGauge::ImGui()
{
#ifdef _DEBUG
    ImGui::Begin("Data Editor");

    // 新しいアイテムの追加用の一時変数
    static int newCount = 0;
    static int newRewardIndex = 0;

    // 新規データの入力UI
    ImGui::Text("Add New Data");
    ImGui::InputInt("Count##new", &newCount);
    ImGui::Combo("Reward##new", &newRewardIndex, rewardNames, IM_ARRAYSIZE(rewardNames));

    if (ImGui::Button("Add")) {
        RewardGaugeData newData;
        newData.count = newCount;
        newData.item = static_cast<RewardItem>(newRewardIndex);
        newData.isGiven = false;
        rewardGaugeData_.push_back(newData);

        // countの値でソート
        std::sort(rewardGaugeData_.begin(), rewardGaugeData_.end(),
            [](const RewardGaugeData& _a, const RewardGaugeData& _b) {
                return _a.count < _b.count;
            });
        newCount = 0;
        newRewardIndex = 0;

        nextRewardGaugeData_ = rewardGaugeData_.begin();
    }

    ImGui::Separator();

    // 既存データの表示と編集
    ImGui::Text("Data List");

    for (size_t i = 0; i < rewardGaugeData_.size(); i++) {
        ImGui::PushID(static_cast<int>(i));

        char label[32];
        sprintf_s(label, "Item %zu: Count=%d", i, rewardGaugeData_[i].count);

        if (ImGui::TreeNode(label)) {
            RewardGaugeData& item = rewardGaugeData_[i];

            // 編集用UI
            int previousCount = item.count;
            ImGui::InputInt("Count", reinterpret_cast<int*>(&item.count));

            // countが変更された場合、再ソートを行う
            if (previousCount != item.count) {
                std::sort(rewardGaugeData_.begin(), rewardGaugeData_.end(),
                    [](const RewardGaugeData& _a, const RewardGaugeData& _b){
                        return _a.count < _b.count;
                    });
            }

            int currentRewardIndex = static_cast<int>(item.item);
            if (ImGui::Combo("Reward", &currentRewardIndex, rewardNames, IM_ARRAYSIZE(rewardNames))) {
                item.item = static_cast<RewardItem>(currentRewardIndex);
            }

            // 削除ボタン
            if (ImGui::Button("Delete")) {
                rewardGaugeData_.erase(rewardGaugeData_.begin() + i);
                nextRewardGaugeData_ = rewardGaugeData_.begin();
                ImGui::TreePop();
                ImGui::PopID();
                break;  // ループを抜ける（削除後のインデックスが無効になるため）
            }

            ImGui::TreePop();
        }

        ImGui::PopID();
    }

    // データの合計表示
    ImGui::Separator();
    ImGui::Text("Total: %zu items", rewardGaugeData_.size());

    ImGui::SameLine();
    if (ImGui::Button("Save")) {
        // JSONに保存
        SaveJsonBinder();
    }


    ImGui::End();
#endif // _DEBUG
}

void RewardGauge::InitJsonBinder()
{
    jsonBinder_ = std::make_unique<JsonBinder>("RewardGauge", "Resources/Data/");

    std::vector<uint32_t> rewardGaugeCount;
    std::vector<uint32_t> rewardGaugeItem;
    jsonBinder_->GetVariableValue("RewardCount", rewardGaugeCount);
    jsonBinder_->GetVariableValue("RewardItem", rewardGaugeItem);

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

void RewardGauge::SaveJsonBinder()
{
    std::vector<uint32_t> rewardGaugeCount;
    std::vector<uint32_t> rewardGaugeItem;
    for (const auto& data : rewardGaugeData_)
    {
        rewardGaugeCount.push_back(data.count);
        rewardGaugeItem.push_back(static_cast<uint32_t>(data.item));
    }
    jsonBinder_->SendVariable("RewardCount", rewardGaugeCount);
    jsonBinder_->SendVariable("RewardItem", rewardGaugeItem);

    jsonBinder_->Save();
}
