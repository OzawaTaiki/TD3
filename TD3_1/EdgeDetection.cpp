#include "EdgeDetection.h"

#include <Core/DXCommon/DXCommon.h>
#include <Core/DXCommon/PSOManager/PSOManager.h>
#include <Core/DXCommon/SRVManager/SRVManager.h>
#include <Debug/Debug.h>



Microsoft::WRL::ComPtr<ID3D12PipelineState> EdgeDetection::pso_;
Microsoft::WRL::ComPtr<ID3D12RootSignature> EdgeDetection::rootSignature_;
bool EdgeDetection::isCreated_ = false;

EdgeDetection::~EdgeDetection()
{
    if (pso_)
        pso_.Reset();

    if (rootSignature_)
        rootSignature_.Reset();
}

void EdgeDetection::Initialize(RenderTarget* _Rendertarget)
{
    dxCommon_ = DXCommon::GetInstance();

    Create();

    renderTarget_ = _Rendertarget;

    shadowMapSrvIndex_ = _Rendertarget->GetSRVindexofDSV();
    IDTextureSrvIndex_ = _Rendertarget->GetSRVindexofRTV();

}

void EdgeDetection::Execute()
{
    if (!isCreated_)
        Create();
    auto commandList = dxCommon_->GetCommandList();
    commandList->SetPipelineState(pso_.Get());
    commandList->SetComputeRootSignature(rootSignature_.Get());

    SRVManager::GetInstance()->PreDraw();
    inputData_->ID[0]++;

    renderTarget_->ChangeRTVState( D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    renderTarget_->ChangeDSVState( D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

    commandList->SetComputeRootDescriptorTable(0, SRVManager::GetInstance()->GetGPUSRVDescriptorHandle(shadowMapSrvIndex_));
    commandList->SetComputeRootDescriptorTable(1, SRVManager::GetInstance()->GetGPUSRVDescriptorHandle(IDTextureSrvIndex_));
    commandList->SetComputeRootDescriptorTable(2, SRVManager::GetInstance()->GetGPUSRVDescriptorHandle(edgeResourceSRVIndex_));
    commandList->SetComputeRootDescriptorTable(3, SRVManager::GetInstance()->GetGPUSRVDescriptorHandle(counterResourceSRVIndex_));
    commandList->SetComputeRootConstantBufferView(4, inputBuffer_->GetGPUVirtualAddress());

    uint32_t dispatchX = (4096 + 15) / 16; // 切り上げ除算で256
    uint32_t dispatchY = (4096 + 15) / 16; // 切り上げ除算で256
    commandList->Dispatch(dispatchX, dispatchY, 1);



    renderTarget_->ChangeRTVState( D3D12_RESOURCE_STATE_RENDER_TARGET);
    renderTarget_->ChangeDSVState( D3D12_RESOURCE_STATE_DEPTH_WRITE);

}

void EdgeDetection::Create()
{
    isCreated_ = true;
    HRESULT hr = S_FALSE;


    D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
    descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    D3D12_DESCRIPTOR_RANGE descriptorRanges[4] = {};
    // gMatrixPalette
    descriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descriptorRanges[0].NumDescriptors = 1;
    descriptorRanges[0].BaseShaderRegister = 0;
    descriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // inputVertex
    descriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descriptorRanges[1].NumDescriptors = 1;
    descriptorRanges[1].BaseShaderRegister = 1;
    descriptorRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // gInfluence
    descriptorRanges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    descriptorRanges[2].NumDescriptors = 1;
    descriptorRanges[2].BaseShaderRegister = 0;
    descriptorRanges[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // outputVertex
    descriptorRanges[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    descriptorRanges[3].NumDescriptors = 1;
    descriptorRanges[3].BaseShaderRegister = 1;
    descriptorRanges[3].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


    D3D12_ROOT_PARAMETER rootParameters[5] = {};

    // ShadowMap
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[0].DescriptorTable.pDescriptorRanges = &descriptorRanges[0];
    rootParameters[0].DescriptorTable.NumDescriptorRanges = 1;

    // IDTexture
    rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[1].DescriptorTable.pDescriptorRanges = &descriptorRanges[1];
    rootParameters[1].DescriptorTable.NumDescriptorRanges = 1;

    // EdgeBuff
    rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[2].DescriptorTable.pDescriptorRanges = &descriptorRanges[2];
    rootParameters[2].DescriptorTable.NumDescriptorRanges = 1;

    // CounterBuff
    rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[3].DescriptorTable.pDescriptorRanges = &descriptorRanges[3];
    rootParameters[3].DescriptorTable.NumDescriptorRanges = 1;

    // Constants
    rootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[4].Descriptor.ShaderRegister = 0;


    descriptionRootSignature.pParameters = rootParameters;
    descriptionRootSignature.NumParameters = _countof(rootParameters);


    //シリアライズしてバイナリする
    Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
    hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
    if (FAILED(hr))
    {
        Debug::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
        assert(false);
    }
    hr = dxCommon_->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature_));
    assert(SUCCEEDED(hr));


    // ComputeShaderの設定
    Microsoft::WRL::ComPtr<IDxcBlob> computeShaderBlob = nullptr;
    // ComputeShaderのコンパイル
    computeShaderBlob = PSOManager::GetInstance()->ComplieShader(L"EdgeDetectoin.CS.hlsl", L"cs_6_0");
    assert(computeShaderBlob != nullptr);

    D3D12_COMPUTE_PIPELINE_STATE_DESC computePsoDesc = {};
    computePsoDesc.pRootSignature = rootSignature_.Get();
    computePsoDesc.CS = { computeShaderBlob->GetBufferPointer(), computeShaderBlob->GetBufferSize() };

    hr = dxCommon_->GetDevice()->CreateComputePipelineState(&computePsoDesc, IID_PPV_ARGS(&pso_));
    assert(SUCCEEDED(hr));


    inputBuffer_ = dxCommon_->CreateBufferResource(sizeof(InputData));
    inputBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&inputData_));

    inputData_->dimensions.x = 4096;
    inputData_->dimensions.y = 4096;
    inputData_->threshold = 0.5f;
    inputData_->idCount = 1;
    for (int i = 0; i < kMaxIDCount; i++)
        inputData_->ID[i] = -1;

    inputData_->ID[0] = 0;



    outputConuterResourceUAV_ = dxCommon_->CreateUAVBufferResource(sizeof(uint32_t) * 16);
    outputEdgeResourceUAV_ = dxCommon_->CreateUAVBufferResource(sizeof(uint32_t) * kMaxEdgeCount);

    edgeResourceSRVIndex_ = SRVManager::GetInstance()->Allocate();
    counterResourceSRVIndex_ = SRVManager::GetInstance()->Allocate();

    SRVManager::GetInstance()->CreateSRVForUAV(edgeResourceSRVIndex_, outputEdgeResourceUAV_.Get(), kMaxEdgeCount, sizeof(uint32_t));
    SRVManager::GetInstance()->CreateSRVForUAV(counterResourceSRVIndex_, outputConuterResourceUAV_.Get(), 16, sizeof(uint32_t));

}
