#include "EdgeDetection.h"

#include <Core/DXCommon/DXCommon.h>
#include <Core/DXCommon/PSOManager/PSOManager.h>
#include <Core/DXCommon/SRVManager/SRVManager.h>
#include <Debug/Debug.h>



Microsoft::WRL::ComPtr<ID3D12PipelineState> EdgeDetection::pso_;
Microsoft::WRL::ComPtr<ID3D12PipelineState> EdgeDetection::initPso_;
Microsoft::WRL::ComPtr<ID3D12RootSignature> EdgeDetection::rootSignature_;
bool EdgeDetection::isCreated_ = false;

EdgeDetection::~EdgeDetection()
{
    if (pso_)
        pso_.Reset();

    if(initPso_)
        initPso_.Reset();

    if (rootSignature_)
        rootSignature_.Reset();
}

void EdgeDetection::Initialize(RenderTarget* _Rendertarget)
{
    dxCommon_ = DXCommon::GetInstance();

    computeCommandList_ = std::make_unique<CommandList>(dxCommon_->GetDevice(), CommandListType::Compute);


    CreatePipelineAndRootSignatrue();

    renderTarget_ = _Rendertarget;

    shadowMapSrvIndex_ = _Rendertarget->GetSRVindexofDSV();
    IDTextureSrvIndex_ = _Rendertarget->GetSRVindexofRTV();

    CreateMeshGenerationPipeline();

}

void EdgeDetection::Execute()
{
    if (!isCreated_)
        CreatePipelineAndRootSignatrue();

    computeCommandList_->Reset(initPso_.Get());
    auto commandList = computeCommandList_->GetCommandList();
    commandList->SetComputeRootSignature(rootSignature_.Get());

    SRVManager::GetInstance()->PreDraw(commandList);

    inputData_->frameCount++;
    idData_->ID[0] = 2;
    //idData_->ID[1] = 0;



    renderTarget_->ChangeRTVState(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    renderTarget_->ChangeDSVState(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

    dxCommon_->ExecuteCommandList();

    renderTarget_->ChangeRTVState(commandList, D3D12_RESOURCE_STATE_COMMON);
    renderTarget_->ChangeDSVState(commandList, D3D12_RESOURCE_STATE_COMMON);



    commandList->SetComputeRootDescriptorTable(0, SRVManager::GetInstance()->GetGPUSRVDescriptorHandle(shadowMapSrvIndex_));
    commandList->SetComputeRootDescriptorTable(1, SRVManager::GetInstance()->GetGPUSRVDescriptorHandle(IDTextureSrvIndex_));
    commandList->SetComputeRootDescriptorTable(2, SRVManager::GetInstance()->GetGPUSRVDescriptorHandle(edgeResourceSRVIndex_));
    commandList->SetComputeRootDescriptorTable(3, SRVManager::GetInstance()->GetGPUSRVDescriptorHandle(counterResourceSRVIndex_));
    commandList->SetComputeRootConstantBufferView(4, inputBuffer_->GetGPUVirtualAddress());
    commandList->SetComputeRootConstantBufferView(5, idBuffer_->GetGPUVirtualAddress());
    commandList->Dispatch(1, 1, 1);


    commandList->SetPipelineState(pso_.Get());

    uint32_t dispatchX = uint32_t((4096 + 15) / 16); // 切り上げ除算で256
    uint32_t dispatchY = uint32_t((4096 + 15) / 16); // 切り上げ除算で256

    commandList->Dispatch(dispatchX, dispatchY, 1);

    computeCommandList_->Execute(true);


    renderTarget_->ChangeRTVState( D3D12_RESOURCE_STATE_RENDER_TARGET);
    renderTarget_->ChangeDSVState( D3D12_RESOURCE_STATE_DEPTH_WRITE);

}

void EdgeDetection::ProcessContourPoints()
{
    // 輪郭点を取得
    std::vector<Vector2> contourPoints;
    GetContourPoints(contourPoints);

    if (contourPoints.empty()) {
        return;
    }

    // 輪郭点を順序付け
    std::vector<Vector2> orderedPoints;
    OrderContourPoints(contourPoints, orderedPoints);

    // ダグラス・ポイカー法で点を削減
    float epsilon = 2.0f; // 許容誤差（適宜調整）
    DouglasPeucker(orderedPoints, epsilon, contourPoints_);


}

std::unique_ptr<Mesh> EdgeDetection::GenerateMeshFromContourPoints(const Matrix4x4& inverseLightViewProj, float height)
{
    // 輪郭点データを取得
    ProcessContourPoints();
    if (contourPoints_.empty() || contourPoints_.size() < 3) {
        return nullptr; // 少なくとも3点が必要
    }

    IdentifyShape();

    // 1. コンピュートシェーダー用のバッファを準備

    // 輪郭点数を更新
    uint32_t numPoints = static_cast<uint32_t>(contourPoints_.size());
    meshConstants_->numCountourPoints = numPoints;
    meshConstants_->height = height;
    meshConstants_->inverseLightViewProj = inverseLightViewProj;

    // 輪郭点バッファを作成・更新
    if (!contourPointsBuffer_ || contourPointsSize_ < numPoints) {
        contourPointsBuffer_ = dxCommon_->CreateBufferResource(sizeof(Vector2) * numPoints);
        contourPointsSize_ = numPoints;

        // SRVを作成
        SRVManager::GetInstance()->CreateSRVForStructureBuffer(
            contourPointsSRVIndex_,
            contourPointsBuffer_.Get(),
            numPoints,
            sizeof(Vector2)
        );
    }

    // 輪郭点データをアップロード
    void* mappedContourPoints = nullptr;
    contourPointsBuffer_->Map(0, nullptr, &mappedContourPoints);
    memcpy(mappedContourPoints, contourPoints_.data(), sizeof(Vector2) * numPoints);
    contourPointsBuffer_->Unmap(0, nullptr);

    // 頂点バッファを作成・更新（必要に応じて）
    uint32_t maxVertices = numPoints * 2 + 2; // 底面 + 上面 + 中心点2つ
    if (!meshVertexBuffer_ || meshVertexBufferSize_ < maxVertices) {
        meshVertexBuffer_ = dxCommon_->CreateUAVBufferResource(sizeof(OriginalVertexData) * maxVertices);
        meshVertexBufferSize_ = maxVertices;

        // UAVを作成
        D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
        uavDesc.Format = DXGI_FORMAT_UNKNOWN;
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
        uavDesc.Buffer.FirstElement = 0;
        uavDesc.Buffer.NumElements = maxVertices;
        uavDesc.Buffer.StructureByteStride = sizeof(OriginalVertexData);
        uavDesc.Buffer.CounterOffsetInBytes = 0;
        uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

        SRVManager::GetInstance()->CreateUAV(meshVertexSRVIndex_, meshVertexBuffer_.Get(), &uavDesc);
    }

    // インデックスバッファを作成・更新（必要に応じて）
    uint32_t maxTriangles = numPoints * 4; // 側面 + 底面上面
    uint32_t indexCount = maxTriangles * 3; // 三角形あたり3インデックス

    if (!meshIndexBuffer_ || meshIndexBufferSize_ < indexCount) {
        meshIndexBuffer_ = dxCommon_->CreateUAVBufferResource(sizeof(uint32_t) * indexCount);
        meshIndexBufferSize_ = indexCount;

        // UAVを作成
        D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
        uavDesc.Format = DXGI_FORMAT_UNKNOWN;
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
        uavDesc.Buffer.FirstElement = 0;
        uavDesc.Buffer.NumElements = indexCount;
        uavDesc.Buffer.StructureByteStride = sizeof(uint32_t); // 単一のuintとして扱う
        uavDesc.Buffer.CounterOffsetInBytes = 0;
        uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

        SRVManager::GetInstance()->CreateUAV(meshIndexSRVIndex_, meshIndexBuffer_.Get(), &uavDesc);
    }

    // 2. コンピュートシェーダーを実行
    computeCommandList_->Reset(meshGenPSO_.Get());
    auto commandList = computeCommandList_->GetCommandList();

    // ディスクリプタヒープを設定
    SRVManager::GetInstance()->PreDraw(commandList);

    // シェーダーリソースをセット
    commandList->SetComputeRootSignature(meshGenRootSignature_.Get());
    commandList->SetComputeRootDescriptorTable(0, SRVManager::GetInstance()->GetGPUSRVDescriptorHandle(contourPointsSRVIndex_));
    commandList->SetComputeRootDescriptorTable(1, SRVManager::GetInstance()->GetGPUSRVDescriptorHandle(shadowMapSrvIndex_));
    commandList->SetComputeRootDescriptorTable(2, SRVManager::GetInstance()->GetGPUSRVDescriptorHandle(meshCounterSRVIndex_));
    commandList->SetComputeRootDescriptorTable(3, SRVManager::GetInstance()->GetGPUSRVDescriptorHandle(meshVertexSRVIndex_));
    commandList->SetComputeRootDescriptorTable(4, SRVManager::GetInstance()->GetGPUSRVDescriptorHandle(meshIndexSRVIndex_));
    commandList->SetComputeRootConstantBufferView(5, meshConstantsBuffer_->GetGPUVirtualAddress());
    LightingSystem::GetInstance()->QueueComputeCommand(commandList, 6);

    // ディスパッチ（1スレッドのみ）
    commandList->Dispatch(1, 1, 1);

    // 3. 実行完了を待機
    computeCommandList_->Execute(true);

    return GenerateMesh();
}

EdgeShape EdgeDetection::IdentifyShape()
{
    // 中心点と面積を計算
    Vector2 centroid(0, 0);
    for (const auto& point : contourPoints_)
    {
        centroid += point;
    }
    centroid /= static_cast<float>(contourPoints_.size());

    // 面積計算（多角形の面積）
    float area = 0.0f;
    for (size_t i = 0; i < contourPoints_.size(); i++)
    {
        const Vector2& p1 = contourPoints_[i];
        const Vector2& p2 = contourPoints_[(i + 1) % contourPoints_.size()];
        area += (p1.x * p2.y - p2.x * p1.y);
    }
    area = std::abs(area) / 2.0f;

    // 最大距離と最小距離（中心からの）
    float maxDist = 0.0f;
    float minDist = FLT_MAX;
    float avgDist = 0.0f;

    for (const auto& point : contourPoints_)
    {
        float dist = (point - centroid).Length();
        maxDist = (std::max)(maxDist, dist);
        minDist = (std::min)(minDist, dist);
        avgDist += dist;
    }
    avgDist /= contourPoints_.size();

    // 円形度（Circularity）: 4π * area / perimeter^2
    float perimeter = 0.0f;
    for (size_t i = 0; i < contourPoints_.size(); i++)
    {
        const Vector2& p1 = contourPoints_[i];
        const Vector2& p2 = contourPoints_[(i + 1) % contourPoints_.size()];
        perimeter += (p2 - p1).Length();
    }

    float PI = 3.14159265359f;
    float circularity = (4.0f * PI * area) / (perimeter * perimeter);

    // 形状判定
    // 1. 円判定 (circularity が 1に近い、かつ maxDist/minDist が1に近い)
    bool isCircle = (circularity > 0.6f) && ((maxDist / minDist) < 1.2f);

    // 2. 長方形判定 (面積と外接矩形の面積の比)
    float minX = FLT_MAX, minY = FLT_MAX;
    float maxX = -FLT_MAX, maxY = -FLT_MAX;

    for (const auto& point : contourPoints_)
    {
        minX = (std::min)(minX, point.x);
        minY = (std::min)(minY, point.y);
        maxX = (std::max)(maxX, point.x);
        maxY = (std::max)(maxY, point.y);
    }

    float boundingBoxArea = (maxX - minX) * (maxY - minY);
    float rectangularity = area / boundingBoxArea;

    bool isRectangle = (rectangularity > 0.8f) && !isCircle;

    // 3. 三角形判定（頂点数が少なく、角の数が3に近い）
    std::vector<Vector2> corners = FindCorners(contourPoints_);
    bool isTriangle = (corners.size() == 3 || (corners.size() >= 3 && corners.size() <= 5));

    // 分類結果
    if (isCircle)
    {
        Debug::Log("Shape classified as: Circle\n");
        return EdgeShape::Circle;
    }
    else if (isRectangle)
    {
        Debug::Log("Shape classified as: Rectangle\n");
        return EdgeShape::Rectangle;
    }
    else if (isTriangle)
    {
        Debug::Log("Shape classified as: Triangle\n");
        return EdgeShape::Triangle;
    }
    else
    {
        Debug::Log("Shape classified as: Other\n");
        return EdgeShape::Other;
    }

}

std::vector<Vector2> EdgeDetection::FindCorners(const std::vector<Vector2>& contourPoints)
{
    std::vector<Vector2> corners;

    // ダグラス・ポイカー法で単純化
    std::vector<Vector2> simplifiedContour;
    float epsilon = 5.0f; // 単純化の程度
    DouglasPeucker(contourPoints, epsilon, simplifiedContour);

    return simplifiedContour; // 単純化した点群が角に相当
}

std::unique_ptr<Mesh> EdgeDetection::GenerateMesh()
{
    std::vector<VertexData> vertices = {};
    std::vector<uint32_t> indices = {};


    // カウンターデータを読み取り、頂点数とインデックス数を取得
    auto counterReadback = dxCommon_->CreateReadbackResources(sizeof(uint32_t) * 8);

    // コマンドリストをリセット
    computeCommandList_->Reset();
    auto commandList = computeCommandList_->GetCommandList();

    // ディスクリプタヒープを設定
    SRVManager::GetInstance()->PreDraw(commandList);

    // カウンターバッファからリードバックバッファにコピー
    commandList->CopyResource(counterReadback.Get(), meshCounterBuffer_.Get());

    // コマンドを実行して完了を待機
    computeCommandList_->Execute(true);

    // カウンターデータを読み取り
    uint32_t vertexCount = 0;
    uint32_t triangleCount = 0;
    uint32_t indexCount = 0;

    {
        D3D12_RANGE readRange = { 0, sizeof(uint32_t) * 4 };
        uint32_t* mappedData = nullptr;
        counterReadback->Map(0, &readRange, reinterpret_cast<void**>(&mappedData));

        vertexCount = mappedData[0];
        triangleCount = mappedData[1];
        indexCount = mappedData[2];

        counterReadback->Unmap(0, nullptr);
    }

    // 頂点データを読み取るためのリードバックバッファ
    auto vertexReadback = dxCommon_->CreateReadbackResources(sizeof(OriginalVertexData) * vertexCount);

    // インデックスデータを読み取るためのリードバックバッファ
    auto indexReadback = dxCommon_->CreateReadbackResources(sizeof(uint32_t) * indexCount);

    // コマンドリストをリセット
    computeCommandList_->Reset();
    commandList = computeCommandList_->GetCommandList();

    // ディスクリプタヒープを設定
    SRVManager::GetInstance()->PreDraw(commandList);

    // バッファからリードバックバッファにコピー
    commandList->CopyResource(vertexReadback.Get(), meshVertexBuffer_.Get());
    commandList->CopyResource(indexReadback.Get(), meshIndexBuffer_.Get());

    // コマンドを実行して完了を待機
    computeCommandList_->Execute(true);

    // 頂点データを読み取り
    std::vector<OriginalVertexData> originalVertices(vertexCount);
    {
        D3D12_RANGE readRange = { 0, sizeof(OriginalVertexData) * vertexCount };
        OriginalVertexData* mappedData = nullptr;
        vertexReadback->Map(0, &readRange, reinterpret_cast<void**>(&mappedData));

        memcpy(originalVertices.data(), mappedData, sizeof(OriginalVertexData) * vertexCount);

        vertexReadback->Unmap(0, nullptr);
    }

    // オリジナル構造体から必要な構造体へ変換
    vertices.resize(vertexCount);
    for (uint32_t i = 0; i < vertexCount; i++) {
        vertices[i].position = originalVertices[i].position;
        vertices[i].texcoord = originalVertices[i].texcoord;
        vertices[i].normal = originalVertices[i].normal;
    }

    // インデックスデータを読み取り
    indices.resize(indexCount);
    {
        D3D12_RANGE readRange = { 0, sizeof(uint32_t) * indexCount };
        uint32_t* mappedData = nullptr;
        indexReadback->Map(0, &readRange, reinterpret_cast<void**>(&mappedData));

        memcpy(indices.data(), mappedData, sizeof(uint32_t) * indexCount);

        indexReadback->Unmap(0, nullptr);
    }


    std::unique_ptr<Mesh> mesh = std::make_unique<Mesh>();

    mesh->Initialize(vertices, indices);

    return std::move(mesh);
}

void EdgeDetection::GetContourPoints(std::vector<Vector2>& _contourPoints)
{
    // 1. カウンターの値を取得するためのリードバックバッファを作成
    auto counterReadback = dxCommon_->CreateReadbackResources(sizeof(uint32_t) * 16);

    // 2. コマンドリストを作成・リセット（既にコンピュートコマンドリストがあれば利用可能）
    computeCommandList_->Reset();
    auto commandList = computeCommandList_->GetCommandList();

    // 3. カウンターバッファからリードバックバッファにコピー
    commandList->CopyResource(counterReadback.Get(), outputConuterResourceUAV_.Get());

    // 4. コマンドを実行して完了を待機
    computeCommandList_->Execute(true);

    // 5. カウンターデータを読み取り
    uint32_t numEdges = 0;
    {
        D3D12_RANGE readRange = { 0, sizeof(uint32_t) };
        void* mappedData = nullptr;
        counterReadback->Map(0, &readRange, &mappedData);

        numEdges = *static_cast<uint32_t*>(mappedData);
        numEdges = (std::min)(numEdges, kMaxEdgeCount); // 最大値を制限

        counterReadback->Unmap(0, nullptr);
    }

    if (numEdges == 0) {
        return; // 輪郭点がない
    }

    // 6. エッジデータ用のリードバックバッファを作成
    auto edgeReadback = dxCommon_->CreateReadbackResources(sizeof(uint32_t) * kMaxEdgeCount);

    computeCommandList_->Reset();
    // 7. エッジバッファからリードバックバッファにコピー
    commandList->CopyResource(edgeReadback.Get(), outputEdgeResourceUAV_.Get());

    // 8. コマンドを実行して完了を待機
    computeCommandList_->Execute(true);

    // 9. エッジデータを読み取り
    std::vector<uint32_t> edgeIndices(numEdges);
    {
        D3D12_RANGE readRange = { 0, sizeof(uint32_t) * numEdges };
        void* mappedData = nullptr;
        edgeReadback->Map(0, &readRange, &mappedData);

        memcpy(edgeIndices.data(), mappedData, sizeof(uint32_t) * numEdges);

        edgeReadback->Unmap(0, nullptr);
    }

    // 10. インデックスから2D座標に変換
    _contourPoints.clear();
    _contourPoints.reserve(numEdges);

    for (uint32_t i = 0; i < numEdges; i++) {
        uint32_t index = edgeIndices[i];
        uint32_t x = index % static_cast<uint32_t>(inputData_->dimensions.x);
        uint32_t y = index / static_cast<uint32_t>(inputData_->dimensions.x);

        _contourPoints.push_back({ static_cast<float>(x), static_cast<float>(y) });
    }
}

void EdgeDetection::OrderContourPoints(const std::vector<Vector2>& unsortedPoints, std::vector<Vector2>& sortedPoints)
{
    if (unsortedPoints.empty()) {
        return;
    }

    // 重心を計算
    Vector2 centroid = { 0.0f, 0.0f };
    for (const auto& p : unsortedPoints) {
        centroid.x += p.x;
        centroid.y += p.y;
    }
    centroid.x /= unsortedPoints.size();
    centroid.y /= unsortedPoints.size();

    // 点をコピー
    sortedPoints = unsortedPoints;

    // 重心からの角度でソート
    std::sort(sortedPoints.begin(), sortedPoints.end(),
        [&centroid](const Vector2& a, const Vector2& b) {
            float angleA = std::atan2(a.y - centroid.y, a.x - centroid.x);
            float angleB = std::atan2(b.y - centroid.y, b.x - centroid.x);
            return angleA < angleB;
        });
}

float EdgeDetection::PerpendicularDistance(const Vector2& point, const Vector2& lineStart, const Vector2& lineEnd)
{
    float dx = lineEnd.x - lineStart.x;
    float dy = lineEnd.y - lineStart.y;

    // 線の長さの二乗
    float lineLengthSq = dx * dx + dy * dy;

    if (lineLengthSq == 0.0f) {
        // 直線の長さがゼロの場合（開始点と終了点が同じ）
        float diffX = point.x - lineStart.x;
        float diffY = point.y - lineStart.y;
        return std::sqrtf(diffX * diffX + diffY * diffY);
    }

    // 点と直線の垂直距離を計算
    float t = ((point.x - lineStart.x) * dx + (point.y - lineStart.y) * dy) / lineLengthSq;

    if (t < 0.0f) {
        // 点が開始点の外側にある
        float diffX = point.x - lineStart.x;
        float diffY = point.y - lineStart.y;
        return std::sqrtf(diffX * diffX + diffY * diffY);
    }

    if (t > 1.0f) {
        // 点が終了点の外側にある
        float diffX = point.x - lineEnd.x;
        float diffY = point.y - lineEnd.y;
        return std::sqrtf(diffX * diffX + diffY * diffY);
    }

    // 点から直線上の最近接点を計算
    float closestX = lineStart.x + t * dx;
    float closestY = lineStart.y + t * dy;

    float diffX = point.x - closestX;
    float diffY = point.y - closestY;

    return std::sqrtf(diffX * diffX + diffY * diffY);
}

void EdgeDetection::DouglasPeucker(const std::vector<Vector2>& points, float epsilon, std::vector<Vector2>& result)
{
    if (points.size() <= 2) {
        // 点が2つ以下なら全部保持
        result = points;
        return;
    }

    // 最初と最後の点を結ぶ直線からの最大距離を持つ点を見つける
    float maxDistance = 0.0f;
    size_t maxIndex = 0;

    for (size_t i = 1; i < points.size() - 1; i++) {
        float distance = PerpendicularDistance(points[i], points.front(), points.back());

        if (distance > maxDistance) {
            maxDistance = distance;
            maxIndex = i;
        }
    }

    // 最大距離がしきい値より大きい場合、分割して再帰
    if (maxDistance > epsilon) {
        std::vector<Vector2> firstPart(points.begin(), points.begin() + maxIndex + 1);
        std::vector<Vector2> secondPart(points.begin() + maxIndex, points.end());

        std::vector<Vector2> resultFirst, resultSecond;
        DouglasPeucker(firstPart, epsilon, resultFirst);
        DouglasPeucker(secondPart, epsilon, resultSecond);

        // 結果を結合（重複する点を除く）
        result = resultFirst;
        result.pop_back(); // 重複を避けるため、最後の点を削除
        result.insert(result.end(), resultSecond.begin(), resultSecond.end());
    }
    else {
        // しきい値以下なら、開始点と終了点のみを保持
        result.clear();
        result.push_back(points.front());
        result.push_back(points.back());
    }
}

void EdgeDetection::CreatePipelineAndRootSignatrue()
{
    isCreated_ = true;
    HRESULT hr = S_FALSE;


    D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
    descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    D3D12_DESCRIPTOR_RANGE descriptorRanges[4] = {};
    // ShadowMap
    descriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descriptorRanges[0].NumDescriptors = 1;
    descriptorRanges[0].BaseShaderRegister = 0;
    descriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // IDTexture
    descriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descriptorRanges[1].NumDescriptors = 1;
    descriptorRanges[1].BaseShaderRegister = 1;
    descriptorRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // EdgeBuff
    descriptorRanges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    descriptorRanges[2].NumDescriptors = 1;
    descriptorRanges[2].BaseShaderRegister = 0;
    descriptorRanges[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // CounterBuff
    descriptorRanges[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    descriptorRanges[3].NumDescriptors = 1;
    descriptorRanges[3].BaseShaderRegister = 1;
    descriptorRanges[3].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


    D3D12_ROOT_PARAMETER rootParameters[6] = {};

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

    // idData
    rootParameters[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[5].Descriptor.ShaderRegister = 1;


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

    idBuffer_ = dxCommon_->CreateBufferResource(sizeof(IDData));
    idBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&idData_));
    for (int i = 0; i < kMaxIDCount; i++)
        idData_->ID[i] = -1;
    idData_->ID[0] = 2;

    inputBuffer_ = dxCommon_->CreateBufferResource(sizeof(InputData));
    inputBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&inputData_));

    inputData_->dimensions.x = 4096;
    inputData_->dimensions.y = 4096;
    inputData_->threshold = 0.5f;
    inputData_->frameCount = 0;



    outputConuterResourceUAV_ = dxCommon_->CreateUAVBufferResource(sizeof(uint32_t) * 16);
    outputEdgeResourceUAV_ = dxCommon_->CreateUAVBufferResource(sizeof(uint32_t) * kMaxEdgeCount);

    edgeResourceSRVIndex_ = SRVManager::GetInstance()->Allocate();
    counterResourceSRVIndex_ = SRVManager::GetInstance()->Allocate();

    SRVManager::GetInstance()->CreateSRVForUAV(edgeResourceSRVIndex_, outputEdgeResourceUAV_.Get(), kMaxEdgeCount, sizeof(uint32_t));
    SRVManager::GetInstance()->CreateSRVForUAV(counterResourceSRVIndex_, outputConuterResourceUAV_.Get(), 16, sizeof(uint32_t));

    computeShaderBlob = PSOManager::GetInstance()->ComplieShader(L"EdgeDetectoin.CS.hlsl", L"cs_6_0",L"InitializeCounter");
    assert(computeShaderBlob != nullptr);
    computePsoDesc.pRootSignature = rootSignature_.Get();
    computePsoDesc.CS = { computeShaderBlob->GetBufferPointer(), computeShaderBlob->GetBufferSize() };
    hr = dxCommon_->GetDevice()->CreateComputePipelineState(&computePsoDesc, IID_PPV_ARGS(&initPso_));
}

void EdgeDetection::CreateMeshGenerationPipeline()
{
    HRESULT hr = S_FALSE;

    // ルートシグネチャの設定
    D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
    descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    D3D12_DESCRIPTOR_RANGE descriptorRanges[5] = {};
    // 輪郭点データ (SRV)
    descriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descriptorRanges[0].NumDescriptors = 1;
    descriptorRanges[0].BaseShaderRegister = 0;
    descriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // depthTexture (SRV)
    descriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descriptorRanges[1].NumDescriptors = 1;
    descriptorRanges[1].BaseShaderRegister = 1;
    descriptorRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


    // カウンターバッファ (UAV)
    descriptorRanges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    descriptorRanges[2].NumDescriptors = 1;
    descriptorRanges[2].BaseShaderRegister = 0;
    descriptorRanges[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // 頂点バッファ (UAV)
    descriptorRanges[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    descriptorRanges[3].NumDescriptors = 1;
    descriptorRanges[3].BaseShaderRegister = 1;
    descriptorRanges[3].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // インデックスバッファ (UAV)
    descriptorRanges[4].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    descriptorRanges[4].NumDescriptors = 1;
    descriptorRanges[4].BaseShaderRegister = 2;
    descriptorRanges[4].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    D3D12_ROOT_PARAMETER rootParameters[7] = {};

    // 輪郭点データ
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[0].DescriptorTable.pDescriptorRanges = &descriptorRanges[0];
    rootParameters[0].DescriptorTable.NumDescriptorRanges = 1;

    // depthTexture
    rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[1].DescriptorTable.pDescriptorRanges = &descriptorRanges[1];
    rootParameters[1].DescriptorTable.NumDescriptorRanges = 1;

    // カウンターバッファ
    rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[2].DescriptorTable.pDescriptorRanges = &descriptorRanges[2];
    rootParameters[2].DescriptorTable.NumDescriptorRanges = 1;

    // 頂点バッファ
    rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[3].DescriptorTable.pDescriptorRanges = &descriptorRanges[3];
    rootParameters[3].DescriptorTable.NumDescriptorRanges = 1;

    // インデックスバッファ
    rootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[4].DescriptorTable.pDescriptorRanges = &descriptorRanges[4];
    rootParameters[4].DescriptorTable.NumDescriptorRanges = 1;

    // 定数バッファ
    rootParameters[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[5].Descriptor.ShaderRegister = 0;

    //
    rootParameters[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[6].Descriptor.ShaderRegister = 1;

    descriptionRootSignature.pParameters = rootParameters;
    descriptionRootSignature.NumParameters = _countof(rootParameters);

    // シリアライズしてバイナリする
    Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
    hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
    if (FAILED(hr))
    {
        Debug::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
        assert(false);
    }
    hr = dxCommon_->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&meshGenRootSignature_));
    assert(SUCCEEDED(hr));

    // コンピュートシェーダーの設定
    Microsoft::WRL::ComPtr<IDxcBlob> computeShaderBlob = nullptr;
    // ComputeShaderのコンパイル
    computeShaderBlob = PSOManager::GetInstance()->ComplieShader(L"MeshGenerator.CS.hlsl", L"cs_6_0");
    assert(computeShaderBlob != nullptr);

    D3D12_COMPUTE_PIPELINE_STATE_DESC computePsoDesc = {};
    computePsoDesc.pRootSignature = meshGenRootSignature_.Get();
    computePsoDesc.CS = { computeShaderBlob->GetBufferPointer(), computeShaderBlob->GetBufferSize() };

    hr = dxCommon_->GetDevice()->CreateComputePipelineState(&computePsoDesc, IID_PPV_ARGS(&meshGenPSO_));
    assert(SUCCEEDED(hr));

    // メッシュ生成用の定数バッファを作成
    meshConstantsBuffer_ = dxCommon_->CreateBufferResource(sizeof(MeshGenConstants));
    meshConstantsBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&meshConstants_));

    // ベースの設定
    meshConstants_->height = 10.0f; // デフォルトの高さ
    meshConstants_->numCountourPoints = 0; // 実行時に設定
    meshConstants_->screenDimensions = Vector2(inputData_->dimensions.x, inputData_->dimensions.y);
    // inverseLightViewProjは実行時に設定

    // メッシュデータ用のバッファを作成
    // カウンターバッファ
    meshCounterBuffer_ = dxCommon_->CreateUAVBufferResource(sizeof(uint32_t) * 8);

    // 頂点・インデックス用のSRVインデックスを確保
    meshCounterSRVIndex_ = SRVManager::GetInstance()->Allocate();
    meshVertexSRVIndex_ = SRVManager::GetInstance()->Allocate();
    meshIndexSRVIndex_ = SRVManager::GetInstance()->Allocate();
    contourPointsSRVIndex_ = SRVManager::GetInstance()->Allocate();

    // カウンターのSRVを作成
    SRVManager::GetInstance()->CreateSRVForUAV(meshCounterSRVIndex_, meshCounterBuffer_.Get(), 8, sizeof(uint32_t));
}
