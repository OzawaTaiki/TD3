#include "BubbleModifier.h"

// C++
#include <algorithm>

// Enghine
#include <Math/Easing.h>
#include <Math/Random/RandomGenerator.h>

void BubbleModifier::Apply(Particle* _particle, float _deltaTime)
{
	if (_particle == nullptr)
		return;

	// lifeTimeに応じて徐々に小さくする
	float maxLifeTime = _particle->GetLifeTime();
	float currentTime = _particle->GetCurrentTime();

	float t = currentTime / maxLifeTime;
	t = std::clamp(t, 0.0f, 1.0f);

	float easedScale = Easing::EaseInExpo(t);

	Vector3 newScale = _particle->GetScale() * (1.0f - easedScale);
	_particle->SetScale(newScale);

	// わずかに上昇させながらsin波で左右に揺らす
	Vector3 position = _particle->GetPosition();

	float upwardVelocity = 0.2f;
	position.y += upwardVelocity * _deltaTime;

	float amplitude = RandomGenerator::GetInstance()->GetRandValue(0.01f, 0.02f); // 振れ幅
	float frequency = RandomGenerator::GetInstance()->GetRandValue(4.0f, 6.0f); // 1周の速さ
	position.x += amplitude * std::sin(frequency * currentTime);

	_particle->SetPosition(position);
}
