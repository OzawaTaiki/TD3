#include "ResultModels.h"

#include <Application/Result/ResultData.h>
#include <Core/DXCommon/TextureManager/TextureManager.h>

void ResultModels::Initialize()
{
    jsonBinder_ = std::make_unique<JsonBinder>("ResultModels", "Resources/Data/Title/Models/");

    auto result = ResultData::GetInstance()->GetGameResult();

    std::vector<std::string> modelNames;

    jsonBinder_->GetVariableValue("modelNames", modelNames);
    jsonBinder_->GetVariableValue("clearModelNames", clearmodelNames_);
    jsonBinder_->GetVariableValue("gameOverModelNames", gameOvermodelNames_);

    if (result == GameResult::Clear)
    {
        for (auto& name : clearmodelNames_)
        {
            LoadModelData(name, 0);
        }
    }
    else if (result == GameResult::GameOver)
    {
        for (auto& name : gameOvermodelNames_)
        {
            LoadModelData(name, 1);
        }
    }


}

void ResultModels::Update()
{
    for (auto& [name, model] : models_)
    {
        model->translate_ = modelData_[name].translate;
        model->scale_ = modelData_[name].scale;
        model->euler_ = modelData_[name].euler;

        model->Update();
    }
}

void ResultModels::Draw(const Camera* _camera)
{
    for (auto& [name, model] : models_)
    {
        model->Draw(_camera, modelData_[name].textureHandle, modelData_[name].color);
    }
}

void ResultModels::DebugWindow()
{
#ifdef _DEBUG

    ImGui::PushID(this);
    ImGui::Begin("Models");
    {
        if (ImGui::Button("Save"))        Save();

        if (ImGui::Button("Create"))      ImGui::OpenPopup("CreateModel");

        if (ImGui::BeginPopupModal("CreateModel"))
        {
            static char name[32] = "NewModel";
            static const char* result[128] = {"Clear","GameOver"};
            static int resultIndex = 0;
            ImGui::InputText("Name", name, sizeof(name));
            ImGui::Combo("Result", &resultIndex, result, IM_ARRAYSIZE(result));
            if (ImGui::Button("OK"))
            {
                LoadModelData(name, resultIndex);

                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel"))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        static char filePath[256] = "";

        if (ImGui::BeginTabBar("ModelTabs"))
        {
            std::vector<std::string> deleteMark;
            for (auto& [name, model] : models_)
            {
                if (ImGui::BeginTabItem(name.c_str()))
                {
                    ImGui::PushID(name.c_str());
                    ImGui::ColorEdit4("Color", &modelData_[name].color.x);
                    ImGui::DragFloat3("Translate", &modelData_[name].translate.x, 0.01f);
                    ImGui::DragFloat3("Scale", &modelData_[name].scale.x, 0.01f);
                    ImGui::DragFloat3("Euler", &modelData_[name].euler.x, 0.01f);
                    ImGui::InputText("FilePath", filePath, sizeof(filePath));
                    static char texturePath[256] = "";
                    static char textureDirPath[256] = "Resources/images/";
                    ImGui::InputText("TextureDirPath", textureDirPath, 256);
                    ImGui::InputText("TexturePath", texturePath, 256);
                    if (ImGui::Button("Apply"))
                    {
                        model->Initialize(filePath);
                        modelData_[name].filePath = filePath;

                        modelData_[name].textureHandle = TextureManager::GetInstance()->Load(texturePath, textureDirPath);
                        modelData_[name].texturePath = texturePath;
                        modelData_[name].textureDirPath = textureDirPath;

                        strcpy_s(filePath, 256, "");
                        strcpy_s(texturePath, 256, "");
                        strcpy_s(textureDirPath, 256, "Resources/images/");
                    }

                    if (ImGui::Button("Delete"))
                    {
                        deleteMark.push_back(name);
                    }
                    ImGui::PopID();
                    ImGui::EndTabItem();
                }
            }
            ImGui::EndTabBar();

            if (!deleteMark.empty())
            {
                for (auto& name : deleteMark)
                {
                    models_.erase(name);
                    modelData_.erase(name);
                }
            }
        }
    }
    ImGui::End();

    ImGui::PopID();

#endif // _DEBUG
}

void ResultModels::Save()
{
    std::vector<std::string> modelNames;

    jsonBinder_->SendVariable("modelNames", modelData_.size());
    for (auto& [name, data] : modelData_)
    {
        modelNames.push_back(name);

        jsonBinder_->SendVariable(name + "_filePath", data.filePath);
        jsonBinder_->SendVariable(name + "_texturePath", data.texturePath);
        jsonBinder_->SendVariable(name + "_textureDirPath", data.textureDirPath);
        jsonBinder_->SendVariable(name + "_translate", data.translate);
        jsonBinder_->SendVariable(name + "_scale", data.scale);
        jsonBinder_->SendVariable(name + "_euler", data.euler);
        jsonBinder_->SendVariable(name + "_color", data.color);
    }

    jsonBinder_->SendVariable("modelNames", modelNames);

    jsonBinder_->Save();
}


void ResultModels::LoadModelData(const std::string& _name, int _result)
{
    loadData data;

    jsonBinder_->GetVariableValue(_name + "_filePath", data.filePath);
    jsonBinder_->GetVariableValue(_name + "_texturePath", data.texturePath);
    jsonBinder_->GetVariableValue(_name + "_textureDirPath", data.textureDirPath);
    jsonBinder_->GetVariableValue(_name + "_translate", data.translate);
    jsonBinder_->GetVariableValue(_name + "_scale", data.scale);
    jsonBinder_->GetVariableValue(_name + "_euler", data.euler);
    jsonBinder_->GetVariableValue(_name + "_color", data.color);

    if (data.textureDirPath.empty() || data.texturePath.empty())
    {
        data.textureDirPath = "Resources/images/";
        data.texturePath = "uvChecker.png";
    }

    data.name = _name;
    data.textureHandle = TextureManager::GetInstance()->Load(data.texturePath, data.textureDirPath);
    // 保存用に保持
    modelData_[_name] = data;
    if (_result == 0)
    {
        auto it = std::find(clearmodelNames_.begin(), clearmodelNames_.end(), _name);
        if (it == clearmodelNames_.end())
        {
            // 見つからなかったとき追加
            clearmodelNames_.push_back(_name);
        }

    }
    else
    {
        auto it = std::find(gameOvermodelNames_.begin(), gameOvermodelNames_.end(), _name);
        if (it == gameOvermodelNames_.end())
        {
            // 見つからなかったとき追加
            gameOvermodelNames_.push_back(_name);
        }
    }



    // モデルの読み込み
    auto model = std::make_unique<ObjectModel>(_name);
    if (data.filePath.empty())
    {
        data.filePath = "cube/cube.obj";
        data.color = { 1, 1, 1, 1 };
        data.scale = { 1, 1, 1 };
    }
    model->Initialize(data.filePath);
    model->translate_ = data.translate;
    model->scale_ = data.scale;
    model->euler_ = data.euler;

    models_[_name] = std::move(model);

}
