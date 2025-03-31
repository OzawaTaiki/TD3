#include "ShadowObjectGenerator.h"

ShadowObjectGenerator* ShadowObjectGenerator::GetInstance()
{
    static ShadowObjectGenerator instance;
    return &instance;
}

void ShadowObjectGenerator::GenerateShadowObject(std::initializer_list<uint32_t> _id)
{
    // マルチスレッドでの処理を行うべき
    for (auto id : _id)
    {

    }
}


