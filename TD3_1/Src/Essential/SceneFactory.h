#pragma once

#include <Features/Scene/ISceneFactory.h>

class SceneFactory : public ISceneFactory
{
public:
    std::unique_ptr<BaseScene> CreateScene(const std::string& _name) override;

    std::string ShowDebugWindow() override;

};
