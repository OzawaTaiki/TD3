#include "SHADOWOBJECT.H"

uint32_t ShadowObject::id_ = 20;

void ShadowObject::Initialize(std::unique_ptr<Mesh> _mesh)
{
    oModel_ = std::make_unique<ObjectModel>("plane");
    oModel_->Initialize(std::move(_mesh));
}

void ShadowObject::Update()
{
    oModel_->Update();
}

void ShadowObject::Draw(const Camera* _camera)
{
    oModel_->Draw(_camera, 0, { 1,1,1,1 });
}

