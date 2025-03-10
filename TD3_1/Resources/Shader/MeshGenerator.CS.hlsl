

// 入力
StructuredBuffer<float2> ContourPoints : register(t0);
Texture2D<float> DepthTexture : register(t1);
RWBuffer<uint> CounterBuffer : register(u0);
cbuffer Constants : register(b0)
{
    float4x4 inverseLightViewProj; // lightVPの逆行列
    float height; // 生成するメッシュの高さ
    uint numCountourPoints; // 輪郭の数
    float2 screenDimensions; // スクリーンのサイズ
};


struct DirectionalLight
{
    float4x4 lightVP;
    float4 color;
    float3 direction;
    float intensity;
    int isHalf;
};

struct PointLight
{
    float4 color;
    float3 position;
    float intensity;
    float radius;
    float decay;
    int isHalf;
};

struct SpotLight
{
    float4 color;
    float3 position;
    float intensity;
    float3 direction;
    float distance;
    float decay;
    float cosAngle;
    float cosFalloutStart;
    int isHalf;
};
static const int MAX_POINT_LIGHT = 32;
static const int MAX_SPOT_LIGHT = 16;
struct LightGroup
{
    DirectionalLight DL;
    PointLight PL[MAX_POINT_LIGHT];
    SpotLight SL[MAX_SPOT_LIGHT];
    int numPointLight;
    int numSpotLight;
};

cbuffer gLightGroup : register(b1)
{
    LightGroup lightGroup;
}
// 出力
struct VertexData
{
    float4 position;
    float3 nomal;
    float pad;
    float2 texcoord;
};
RWStructuredBuffer<VertexData> VertexBuffer : register(u1);
RWStructuredBuffer<uint> TriangleIndices : register(u2);

// スクリーン座標からワールド座標への変換
float3 ScreenToWorld(float2 screenPos, float depth)
{
    // NDC座標に変換
    float2 ndc = float2(
        (2.0f * screenPos.x / screenDimensions.x) - 1.0f,
        1.0f - (2.0f * screenPos.y / screenDimensions.y)
    );

    // ワールド座標に変換
    float4 ndcPoint = float4(ndc, depth, 1.0f);
    float4 worldPos = mul(ndcPoint, inverseLightViewProj);
    worldPos /= worldPos.w;

    float3 lightDir = lightGroup.DL.direction.xyz;
    lightDir = normalize(lightDir);

    float t = 0.0f;
    if (abs(lightDir.y) > 0.0001f)
    {
        t = -worldPos.y / lightDir.y;
    }
    else
    {
        // ライトが地面と平行な場合は、現在位置をそのまま使用
        worldPos.y = 0.0f;
        return worldPos.xyz;
    }

    // 6. 地面との交点を計算
    float3 groundIntersection = worldPos.xyz + lightDir * t;
    groundIntersection.y = 0.0f; // 数値誤差を防ぐために明示的に0に設定

    return groundIntersection;
}

[numthreads(1, 1, 1)]
void main()
{
    // 頂点数の計算
    uint vertexCount = numCountourPoints * 2 + 2; // 底面 + 上面 + 中心点2つ

    // カウンターの初期化
    CounterBuffer[0] = vertexCount; // 頂点数を記録


    // 底面と上面の中心点の計算
    float3 bottomCenter = float3(0, 0, 0);

    uint i= 0;

    // すべての輪郭点をスキャンして中心点を計算
    for (i= 0; i < numCountourPoints; i++)
    {
        float depth = DepthTexture[ContourPoints[i]];
        float3 bottomPos = ScreenToWorld(ContourPoints[i], depth);
        bottomCenter += bottomPos;
    }

    // 中心点を平均化
    bottomCenter /= numCountourPoints;
    float3 topCenter = bottomCenter + float3(0.0f, height, 0.0f);

    // 中心点を頂点バッファに格納
    // 底面中心点
    uint bottomCenterIndex = numCountourPoints * 2;
    VertexData bottomCenterVertex;
    bottomCenterVertex.position = float4(bottomCenter, 1.0f);
    bottomCenterVertex.nomal = float3(0.0f, 1.0f, 0.0f);
    bottomCenterVertex.pad = 0.0f;
    bottomCenterVertex.texcoord = float2(0.5f, 0.5f); // 中心点のUV
    VertexBuffer[bottomCenterIndex] = bottomCenterVertex;

    // 上面中心点
    uint topCenterIndex = numCountourPoints * 2 + 1;
    VertexData topCenterVertex;
    topCenterVertex.position = float4(topCenter, 1.0f);
    topCenterVertex.nomal = float3(0.0f, 1.0f, 0.0f);
    topCenterVertex.pad = 0.0f;
    topCenterVertex.texcoord = float2(0.5f, 0.5f); // 中心点のUV
    VertexBuffer[topCenterIndex] = topCenterVertex;

    // 底面と上面の頂点を生成
    for (i= 0; i < numCountourPoints; i++)
    {
        // 底面の頂点
        float depth = DepthTexture[ContourPoints[i]];
        float3 bottomPos = ScreenToWorld(ContourPoints[i], depth);
        uint bottomIndex = i;

        VertexData bottomVertex;
        bottomVertex.position = float4(bottomPos, 1.0f);
        bottomVertex.nomal = float3(0.0f, 1.0f, 0.0f);
        bottomVertex.pad = 0.0f;
        bottomVertex.texcoord = float2(ContourPoints[i]) / screenDimensions; // 正規化したUV

        VertexBuffer[bottomIndex] = bottomVertex;

        // 上面の頂点
        float3 topPos = bottomPos + float3(0.0f, height, 0.0f);
        uint topIndex = i + numCountourPoints;

        VertexData topVertex;
        topVertex.position = float4(topPos, 1.0f);
        topVertex.nomal = float3(0.0f, 1.0f, 0.0f);
        topVertex.pad = 0.0f;
        topVertex.texcoord = float2(ContourPoints[i]) / screenDimensions; // 同じUV

        VertexBuffer[topIndex] = topVertex;
    }

    // インデックスバッファの生成
    uint indexOffset = 0;

    // 側面の三角形を生成
    for (i= 0; i < numCountourPoints; i++)
    {
        uint next = (i + 1) % numCountourPoints;

        // 底面インデックス
        uint b0 = i;
        uint b1 = next;

        // 上面インデックス
        uint t0 = i + numCountourPoints;
        uint t1 = next + numCountourPoints;

        // 1つ目の三角形（底辺→上辺→底辺）
        TriangleIndices[indexOffset++] = b0;
        TriangleIndices[indexOffset++] = t0;
        TriangleIndices[indexOffset++] = b1;

        // 2つ目の三角形（底辺→上辺→上辺）
        TriangleIndices[indexOffset++] = b1;
        TriangleIndices[indexOffset++] = t0;
        TriangleIndices[indexOffset++] = t1;
    }

    // 底面の三角形を生成（中心点から扇形）
    for (i= 0; i < numCountourPoints; i++)
    {
        uint next = (i + 1) % numCountourPoints;

        TriangleIndices[indexOffset++] = bottomCenterIndex;
        TriangleIndices[indexOffset++] = i;
        TriangleIndices[indexOffset++] = next;
    }

    // 上面の三角形を生成（中心点から扇形）
    for (i= 0; i < numCountourPoints; i++)
    {
        uint next = (i + 1) % numCountourPoints;
        uint current = i + numCountourPoints;
        uint nextTop = next + numCountourPoints;

        TriangleIndices[indexOffset++] = topCenterIndex;
        TriangleIndices[indexOffset++] = nextTop;
        TriangleIndices[indexOffset++] = current;
    }

    // 生成した三角形の数を記録
    CounterBuffer[1] = indexOffset / 3; // 三角形数
    // インデックスの総数を記録
    CounterBuffer[2] = indexOffset;
}