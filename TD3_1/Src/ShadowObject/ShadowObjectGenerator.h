#pragma once

#include <ShadowObject/detection/EdgeDetection.h>

#include <initializer_list>
#include <cstdint>

class ShadowObjectGenerator
{
public:

    static ShadowObjectGenerator* GetInstance();

    void GenerateShadowObject(std::initializer_list<uint32_t> _id);


private:

    std::unique_ptr< EdgeDetection> edgeDetection_ = nullptr;


private:
    ShadowObjectGenerator() = default;
    ~ShadowObjectGenerator() = default;
    ShadowObjectGenerator(const ShadowObjectGenerator&) = delete;
    ShadowObjectGenerator& operator=(const ShadowObjectGenerator&) = delete;

};
