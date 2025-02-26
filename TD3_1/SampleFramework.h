#pragma once

#include <Framework/Framework.h>

class SampleFramework : public Framework
{
public:
    SampleFramework() = default;
    ~SampleFramework() = default;

    void Initialize() override;
    void Update() override;
    void Draw() override;

    void Finalize() override;

private:

};
