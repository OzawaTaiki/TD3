#pragma once

#include <Math/Vector/Vector2.h>
#include <Math/Matrix/Matrix4x4.h>
#include <Features/Model/Mesh/Mesh.h>
#include <Features/Light/System/LightingSystem.h>
#include <Core/DXCommon/RTV/RenderTexture.h>
#include <Core/DXCommon/CmdList/CommandList.h>


#include <initializer_list>
#include <cstdint>

#include <d3d12.h>
#include <wrl.h>

enum class EdgeShape
{
    Triangle,
    Rectangle,
    Circle,
    Other
};

class DXCommon;
class EdgeDetection
{
public:
    EdgeDetection() = default;
    ~EdgeDetection();

    void Initialize(RenderTarget* _Rendertarget);

    void Execute(std::initializer_list<uint32_t> _ids);

    void ProcessContourPoints();
    std::unique_ptr<Mesh> GenerateMeshFromContourPoints(const Matrix4x4& inverseLightViewProj, float height, const Vector3& _centroid);

    EdgeShape IdentifyShape();
    std::vector<Vector2> FindCorners(const std::vector<Vector2>& contourPoints);


private:
    static bool isCreated_;

    static Microsoft::WRL::ComPtr<ID3D12PipelineState> pso_;
    static Microsoft::WRL::ComPtr<ID3D12PipelineState> initPso_;
    static Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;

    DXCommon* dxCommon_;
    RenderTarget* renderTarget_;

    // CSに送るリソースたち
    uint32_t shadowMapSrvIndex_ = 0;
    uint32_t IDTextureSrvIndex_ = 0;

    const uint32_t kMaxEdgeCount = 4096;

    Microsoft::WRL::ComPtr<ID3D12Resource> outputEdgeResourceUAV_;
    Microsoft::WRL::ComPtr<ID3D12Resource> outputConuterResourceUAV_;
    Microsoft::WRL::ComPtr<ID3D12Resource> debugTextureUAV_;

    uint32_t edgeResourceSRVIndex_ = 0;
    uint32_t counterResourceSRVIndex_ = 0;
    uint32_t debugTextureSRVIndex_ = 0;

    Microsoft::WRL::ComPtr<ID3D12Resource> inputBuffer_;

    struct InputData
    {
        Vector2 dimensions;
        float threshold;
        uint32_t frameCount;
    };

    InputData* inputData_ = nullptr;

    Microsoft::WRL::ComPtr<ID3D12Resource> idBuffer_;
    static const uint32_t kMaxIDCount = 16;
    struct alignas(16)  IDData
    {
        int32_t ID[kMaxIDCount];
    };

    IDData* idData_ = nullptr;

    std::unique_ptr<CommandList> computeCommandList_ = nullptr;

    std::vector<Vector2> contourPoints_;


    // メッシュ生成関連
    struct OriginalVertexData
    {
        Vector4 position;
        Vector3 normal;
        float pad;
        Vector2 texcoord;
    };

    Microsoft::WRL::ComPtr<ID3D12RootSignature> meshGenRootSignature_;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> meshGenPSO_;

    struct MeshGenConstants
    {
        Matrix4x4 inverseLightViewProj;
        float height;
        uint32_t numCountourPoints;
        Vector2 screenDimensions;
    };

    // メッシュ生成データ
    Microsoft::WRL::ComPtr<ID3D12Resource> meshConstantsBuffer_;
    MeshGenConstants* meshConstants_ = nullptr;

    // 輪郭点バッファ
    Microsoft::WRL::ComPtr<ID3D12Resource> contourPointsBuffer_;
    uint32_t contourPointsSize_ = 0;
    uint32_t contourPointsSRVIndex_ = 0;

    // メッシュデータバッファ
    Microsoft::WRL::ComPtr<ID3D12Resource> meshCounterBuffer_;
    Microsoft::WRL::ComPtr<ID3D12Resource> meshVertexBuffer_;
    Microsoft::WRL::ComPtr<ID3D12Resource> meshIndexBuffer_;
    uint32_t meshVertexBufferSize_ = 0;
    uint32_t meshIndexBufferSize_ = 0;
    uint32_t meshCounterSRVIndex_ = 0;
    uint32_t meshVertexSRVIndex_ = 0;
    uint32_t meshIndexSRVIndex_ = 0;


    std::unique_ptr<Mesh> GenerateMesh(const Vector3& _centroid);
    void RecalculateNormals(std::vector<VertexData>& vertices, const std::vector<uint32_t>& indices);

    void CreatePipelineAndRootSignatrue();
    void CreateMeshGenerationPipeline();


    void GetContourPoints(std::vector<Vector2>& _contourPoints);
    void OrderContourPoints(const std::vector<Vector2>& unsortedPoints, std::vector<Vector2>& sortedPoints);
    float PerpendicularDistance(const Vector2& point, const Vector2& lineStart, const Vector2& lineEnd);
    void DouglasPeucker(const std::vector<Vector2>& points, float epsilon, std::vector<Vector2>& result);
};
