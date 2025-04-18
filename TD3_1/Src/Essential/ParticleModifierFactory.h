#pragma once

#include <Features/Effect/Modifier/IPaticleMoifierFactory.h>

class ParticleModifierFactory : public IParticleMoifierFactory
{

public:
    ParticleModifierFactory() = default;
    ~ParticleModifierFactory() override = default;

    std::unique_ptr<ParticleModifier> CreateModifier(const std::string _name) override;
};