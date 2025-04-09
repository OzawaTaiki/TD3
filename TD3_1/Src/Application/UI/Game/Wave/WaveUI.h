#pragma once

#include <Features/Event/EventListener.h>
#include <Features/UI/UISprite.h>

#include <memory>
#include <vector>

class WaveUI : public iEventListener
{
public:
    WaveUI();
    ~WaveUI();

    void Initialize(uint32_t _waveCount);
    void Update();
    void Draw();
    void OnEvent(const GameEvent& _event) override;

private:

    std::unique_ptr<UISprite> waveUI_;

    uint32_t soundHandle_ = 0;
    float soundVolume_ = 0.5f;

    int currentWave_ = 0;
    std::vector<uint32_t> textureHandle_;

};