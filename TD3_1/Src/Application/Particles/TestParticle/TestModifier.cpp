#include "TestModifier.h"

void TestModifier::Apply(Particle* _particle, float _deltaTime) {
	if (_particle == nullptr) {
		return;
	}

	float speed_ = 2.0f;
	speed_ *= 4.0f;

	_particle->SetSpeed(speed_);
}