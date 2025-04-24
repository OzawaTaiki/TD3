#pragma once

// Engine
#include <Features/Model/ObjectModel.h>
#include <Features/Json/JsonBinder.h>


#include <vector>
#include <map>
#include <string>
#include <memory>

class TitleBackModel
{
public:
    TitleBackModel() = default;
    ~TitleBackModel() = default;

    void Initialize();
    void Update();
    void Draw(const Camera* _camera);
    void DebugWindow();

private:

    void Save();

    std::map<std::string, std::unique_ptr<ObjectModel>> models_;


    std::unique_ptr<JsonBinder> jsonBinder_ = nullptr;


    struct loadData
    {
        std::string name;
        std::string filePath;
        std::string textureDirPath;
        std::string texturePath;
        Vector3 translate;
        Vector3 scale;
        Vector3 euler;
        Vector4 color;
        uint32_t textureHandle = 0;
    };
    std::map<std::string, loadData> modelData_;

    void LoadModelData(const std::string& _name);

};