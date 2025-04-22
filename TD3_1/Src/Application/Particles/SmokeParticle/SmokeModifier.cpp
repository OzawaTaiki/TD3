#define NOMINMAX
#include "SmokeModifier.h"

// C++
#include <algorithm>

// Enghine
#include <Math/Easing.h>
#include <Math/Random/RandomGenerator.h>

float SmokeModifier::deceleration_ = 0.8f; // 初期値

void SmokeModifier::Apply(Particle* _particle, float _deltaTime)
{
    if (_particle == nullptr)
        return;

    // 減速率を適用
    float speed = _particle->GetSpeed();
    speed *= (1.0f - deceleration_ * _deltaTime);
    _particle->SetSpeed(speed);

    // lifeTimeに応じて徐々に透明にする
    float maxLifeTime = _particle->GetLifeTime();
    float currentTime = _particle->GetCurrentTime();

    float t = currentTime / maxLifeTime;
    t = std::clamp(t, 0.0f, 1.0f);

    float easedAlpha = Easing::EaseInQuart(t);

    Vector4 newColor = _particle->GetColor();
    newColor.w = 1.0f - easedAlpha;
    _particle->SetColor(newColor);
}