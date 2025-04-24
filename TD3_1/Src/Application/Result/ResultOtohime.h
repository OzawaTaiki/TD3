#pragma once

#include <Features/Model/ObjectModel.h>
#include <Features/Json/JsonBinder.h>


class ResultOtohime
{
public:
    ResultOtohime() = default;
    ~ResultOtohime() = default;

    void Initialize();
    void Update();
    void Draw(const Camera* _camera);

    void SetActive(bool _active) { active_ = _active; }


    void DebugWindow();
private:
    bool active_ = false;

    std::array<std::unique_ptr<ObjectModel>, 3> otohimeModels_ = {};
    std::array<std::unique_ptr<ObjectModel>, 3> osaraModels_ = {};

    Vector3 standardPos_ = { 0, 0, 0 }; // 基準座標
    Vector3 osaraOffset_ = { 0, 0, 0 }; // お皿のオフセット 下にいくつか
    Vector3 objectRange = { 0,0,0 };    // 隣同士の間隔

    // 移動する距離
    Vector3 moveDistance_ = { 0, 0, 0 };
    float speed_ = 0.0f; // 移動速度

    Vector3 modvedDistanse_ = { 0,0,0 };

    std::unique_ptr<JsonBinder> jsonBinder_ = nullptr;



};