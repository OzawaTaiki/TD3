#include "ResultOtohime.h"

void ResultOtohime::Initialize()
{
    otohimeModels_[0] = std::make_unique<ObjectModel>("Otohime0");
    otohimeModels_[0]->Initialize("cube/cube.obj");

    otohimeModels_[1] = std::make_unique<ObjectModel>("Otohime1");
    otohimeModels_[1]->Initialize("cube/cube.obj");

    otohimeModels_[2] = std::make_unique<ObjectModel>("Otohime2");
    otohimeModels_[2]->Initialize("cube/cube.obj");

    osaraModels_[0] = std::make_unique<ObjectModel>("Osara0");
    osaraModels_[0]->Initialize("teapot.obj");
    osaraModels_[1] = std::make_unique<ObjectModel>("Osara1");
    osaraModels_[1]->Initialize("teapot.obj");
    osaraModels_[2] = std::make_unique<ObjectModel>("Osara2");
    osaraModels_[2]->Initialize("teapot.obj");

    jsonBinder_ = std::make_unique<JsonBinder>("ResultOtohime", "Resources/Data/Result/");
    jsonBinder_->RegisterVariable("StandardPos", &standardPos_);
    jsonBinder_->RegisterVariable("MoveDistance", &moveDistance_);
    jsonBinder_->RegisterVariable("Speed", &speed_);
    jsonBinder_->RegisterVariable("OsaraOffset", &osaraOffset_);
    jsonBinder_->RegisterVariable("ObjectRange", &objectRange);

    for (size_t i = 0; i < otohimeModels_.size(); i++)
    {
        otohimeModels_[i]->translate_ = standardPos_ + objectRange * static_cast<float>(i);
        osaraModels_[i]->translate_ = standardPos_ + osaraOffset_ + objectRange * static_cast<float>(i);
    }

}

void ResultOtohime::Update()
{
    if (!active_)
        return;

    const Vector3 move = { -1,0,0 };

    // 移動処理
    for (auto& model : otohimeModels_) {
        model->translate_ += move * speed_;
    }
    for (auto& model : osaraModels_) {
        model->translate_ += move * speed_;
    }

    modvedDistanse_ += move * speed_;

    if (modvedDistanse_.x > moveDistance_.x)
    {
        active_ = false;
    }

    for (auto& model : otohimeModels_) {
        model->Update();
    }
    for (auto& model : osaraModels_) {
        model->Update();
    }
}

void ResultOtohime::Draw(const Camera* _camera)
{
    for (auto& model : otohimeModels_) {
        model->Draw(_camera,{1,1,1,1});
    }
    for (auto& model : osaraModels_) {
        model->Draw(_camera, { 1,1,1,1 });
    }
}

void ResultOtohime::DebugWindow()
{
#ifdef _DEBUG
    ImGui::PushID(this);
    ImGui::Begin("ResultOtohime");

    ImGui::DragFloat3("StandardPos", &standardPos_.x, 0.01f);
    ImGui::DragFloat3("MoveDistance", &moveDistance_.x, 0.01f);
    ImGui::DragFloat("Speed", &speed_, 0.01f);
    ImGui::DragFloat3("OsaraOffset", &osaraOffset_.x, 0.01f);
    ImGui::DragFloat3("ObjectRange", &objectRange.x, 0.01f);

    ImGui::Checkbox("Active", &active_);
    if(!active_)
    {
        for (size_t i = 0; i < otohimeModels_.size(); i++)
        {
            otohimeModels_[i]->translate_ = standardPos_ + objectRange * static_cast<float>(i);
            osaraModels_[i]->translate_ = standardPos_ + osaraOffset_ + objectRange * static_cast<float>(i);
        }
    }
    ImGui::End();

    ImGui::PopID();
#endif // _DEBUG
}
