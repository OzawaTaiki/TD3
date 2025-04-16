#include "EnemyRouteManager.h"

EnemyRouteManager::~EnemyRouteManager()
{
    WriteJson();

    for (auto& pair : bezierCurves_)
    {
        delete pair.second;
    }

    bezierCurves_.clear();
}

void EnemyRouteManager::Initialize()
{
    jsonBinder_ = std::make_unique<JsonBinder>("EnemyRouteKeyMap", "Resources/data/Enemy/route/");

    std::vector<std::string> keys;
    jsonBinder_->GetVariableValue("BezierCurveKey", keys);

    for (const auto& key : keys)
    {
        bezierCurves_[key] = new BezierCurve3D(key);
        bezierCurves_[key]->Load();
    }

}

void EnemyRouteManager::RegisterBezierCurve(const std::string& name, BezierCurve3D* bezierCurve)
{
    if (bezierCurve == nullptr)     return;

    if (bezierCurves_.find(name) != bezierCurves_.end())
    {
        // 既に登録されている場合は、古いBezierCurveを削除
        delete bezierCurves_[name];
    }
    bezierCurves_[name] = bezierCurve;
}

std::vector<std::string> EnemyRouteManager::GetBezierCurveNames() const
{
    std::vector<std::string> names;
    for (const auto& pair : bezierCurves_)
    {
        names.push_back(pair.first);
    }
    return names;
}


BezierCurve3D* EnemyRouteManager::GetBezierCurve(const std::string& name) const
{
    auto it = bezierCurves_.find(name);
    if (it != bezierCurves_.end())
    {
        return it->second;
    }

    return nullptr;
}

void EnemyRouteManager::ShowDebugWindow()
{
#ifdef _DEBUG

    if (ImGui::Button("Create"))
    {
        ImGui::OpenPopup("CreateBezierCurve");
    }

    if (ImGui::BeginPopupModal("CreateBezierCurve"))
    {
        static char name[64] = "";
        ImGui::InputText("Name", name, sizeof(name));
        if (ImGui::Button("OK"))
        {
            bezierCurves_[name] = new BezierCurve3D(name);
            bezierCurves_[name]->Load();
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel"))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }


#endif // _DEBUG

}

void EnemyRouteManager::WriteJson()
{
    std::vector<std::string> keys;
    for (const auto& [name,pBezier] : bezierCurves_)
    {
        if (name.empty() || pBezier == nullptr)            continue;

        keys.push_back(name);
        pBezier->Save();
    }

    jsonBinder_->SendVariable("BezierCurveKey", keys);
    jsonBinder_->Save();


}
