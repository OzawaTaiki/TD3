#include "WaveUI.h"

#include <Features/Event/EventManager.h>
#include <Core/DXCommon/TextureManager/TextureManager.h>
#include <System/Audio/Audio.h>

#include <Application/UI/Game/Wave/WaveChangeData.h>

WaveUI::WaveUI()
{
    // イベントリスナーの登録
    EventManager::GetInstance()->AddEventListener("WaveStart", this);
}

WaveUI::~WaveUI()
{
     EventManager::GetInstance()->RemoveEventListener("WaveStart", this);
}

void WaveUI::Initialize(uint32_t _waveCount)
{
    waveUI_ = std::make_unique<UISprite>();
    waveUI_->Initialize("WaveUI");
    // テクスチャの読み込み

    TextureManager* textureManager = TextureManager::GetInstance();

    std::string defaultDirpath = "game/ui/wave";
    for (uint32_t i = 1; i <= _waveCount; ++i)
    {
        // テクスチャの読み込み
        std::string texturePath = defaultDirpath + std::to_string(i) + ".png";
        uint32_t textureHandle = textureManager->Load(texturePath);
        textureHandle_.push_back(textureHandle);
    }

    // 初期状態では最初のウェーブを表示
    currentWave_ = 0;
    waveUI_->SetTextureHandle(textureHandle_[currentWave_]);

    soundHandle_ = Audio::GetInstance()->SoundLoadWave("Resources/audio/nextWave.wav");
    soundVolume_ = 0.5f; // デフォルトのボリュームを設定
}

void WaveUI::Update()
{
    waveUI_->Update();
}

void WaveUI::Draw()
{
    waveUI_->Draw();
}

void WaveUI::OnEvent(const GameEvent& _event)
{
    // イベントの種類を確認
    if (_event.GetEventType() == "WaveStart") {

        WaveChangeData* eventData = static_cast<WaveChangeData*>(_event.GetData());
        // ウェーブのインデックスを取得
        currentWave_ = eventData->waveNumber;
        // テクスチャを変更
        waveUI_->SetTextureHandle(textureHandle_[currentWave_]);

        Audio::GetInstance()->SoundPlay(soundHandle_, soundVolume_); // サウンドを再生
    }
}
