#pragma once

// Engine
#include <Features/Effect/Modifier/ParticleModifier.h>

class TestModifier : public ParticleModifier {
public:
	TestModifier() = default;
	~TestModifier() override = default;

	void Apply(Particle* _particle, float _deltaTime) override;

private:

};
