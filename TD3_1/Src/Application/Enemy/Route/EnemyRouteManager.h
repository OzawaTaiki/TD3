#pragma once
#include <Math/BezierCurve3D.h>
#include <Features/Json/JsonBinder.h>

#include <map>
#include <string>

class EnemyRouteManager
{
public:

    EnemyRouteManager() = default;
    ~EnemyRouteManager();

    void Initialize();


    void RegisterBezierCurve(const std::string& name, BezierCurve3D* bezierCurve);

    std::map<std::string, BezierCurve3D*>& GetBezierCurves() { return bezierCurves_; }
    std::vector<std::string> GetBezierCurveNames() const;
    BezierCurve3D* GetBezierCurve(const std::string& name) const;

    void ShowDebugWindow();

private:

    // mapのキーをJsonに書き出す
    void WriteJson();

    std::map<std::string, BezierCurve3D*> bezierCurves_;


    std::unique_ptr<JsonBinder> jsonBinder_;

};