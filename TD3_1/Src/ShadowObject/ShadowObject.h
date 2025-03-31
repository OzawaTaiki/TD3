#pragma once
#pragma once

#include <Features/Model/ObjectModel.h>

#include <cstdint>

class ShadowObject
{
public:

    ShadowObject() = default;
    ~ShadowObject() = default;

    void Initialize(std::unique_ptr<Mesh> _mesh);

    void Update();

    void Draw(const Camera* _camera);

private:

    static uint32_t id_;

    std::unique_ptr<ObjectModel> oModel_ = nullptr;



};
