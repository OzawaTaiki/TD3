#pragma once

// Engine
#include <Features/Effect/Modifier/ParticleModifier.h>

class SmokeModifier : public ParticleModifier
{
public:
	SmokeModifier() = default;
	~SmokeModifier() override = default;

	void Apply(Particle* _particle, float _deltaTime) override;

private:
	static float deceleration_;
};

