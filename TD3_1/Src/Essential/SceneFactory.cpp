#include "SceneFactory.h"

#include "SampleScene.h"
#include <Features/Scene/ParticleTestScene.h>

#include <Scene/GameScene.h>

std::unique_ptr<BaseScene> SceneFactory::CreateScene(const std::string& _name)
{
    if (_name == "Sample")
    {
        return std::make_unique<SampleScene>();
    }
    else if (_name == "ParticleTest")
    {
        return std::make_unique<ParticleTestScene>();
    }
    else if (_name == "Game")
    {
        return std::make_unique<GameScene>();
    }


    assert("Scene Not Found");

    return nullptr;
}

std::string SceneFactory::ShowDebugWindow()
{
#ifdef _DEBUG

    ImGui::SeparatorText("Scene Factory");

    if (ImGui::Button("Sample"))
    {
        return "Sample";
    }
    if (ImGui::Button("ParticleTest"))
    {
        return "ParticleTest";
    }
    if (ImGui::Button("Game"))
    {
        return "Game";
    }


#endif // _DEBUG
    return "";

}
