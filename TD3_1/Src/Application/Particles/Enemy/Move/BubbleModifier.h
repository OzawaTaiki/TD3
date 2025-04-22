#pragma once

// Engine
#include <Features/Effect/Modifier/ParticleModifier.h>

class BubbleModifier : public ParticleModifier
{
public:
	BubbleModifier() = default;
	~BubbleModifier() override = default;

	void Apply(Particle* _particle, float _deltaTime) override;

private:

};