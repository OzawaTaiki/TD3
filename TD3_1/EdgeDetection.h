#pragma once

#include <Math/Vector/Vector2.h>
#include <Core/DXCommon/RTV/RenderTexture.h>
#include <cstdint>

#include <d3d12.h>
#include <wrl.h>

class DXCommon;
class EdgeDetection
{
public:
    EdgeDetection() = default;
    ~EdgeDetection();

    void Initialize(RenderTarget* _Rendertarget);

    void Execute();

private:
    static bool isCreated_;

    static Microsoft::WRL::ComPtr<ID3D12PipelineState> pso_;
    static Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;

    DXCommon* dxCommon_;
    RenderTarget* renderTarget_;

    // CSに送るリソースたち
    uint32_t shadowMapSrvIndex_ = 0;
    uint32_t IDTextureSrvIndex_ = 0;

    const uint32_t kMaxEdgeCount = 16384;

    Microsoft::WRL::ComPtr<ID3D12Resource> outputEdgeResourceUAV_;
    Microsoft::WRL::ComPtr<ID3D12Resource> outputConuterResourceUAV_;

    uint32_t edgeResourceSRVIndex_ = 0;
    uint32_t counterResourceSRVIndex_ = 0;

    Microsoft::WRL::ComPtr<ID3D12Resource> inputBuffer_;

    static const uint32_t kMaxIDCount = 32;
    struct InputData
    {
        Vector2 dimensions;
        float threshold;
        uint32_t idCount;
        int32_t ID[kMaxIDCount];
    };

    InputData* inputData_ = nullptr;


    void Create();


};
