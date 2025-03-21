

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
    // 中心点は持たない 上面と底面の点が三つの法線を持つ
    // 三つの法線->その点が属する面の法線
    uint vertexCount = numCountourPoints * 2 * 3;
    uint vertexOffset = 0;

    // カウンターの初期化
    CounterBuffer[0] = vertexCount; // 頂点数を記録
    uint i = 0;


    for (i = 0; i < numCountourPoints; i++)
    {
        // 底面の頂点
        float depth = DepthTexture[ContourPoints[i]];
        float3 bottomPos = ScreenToWorld(ContourPoints[i], depth);
        uint bottomIndex = i;

        VertexData bottomVertex;
        bottomVertex.position = float4(bottomPos, 1.0f);
        bottomVertex.nomal = float3(0.0f, -1.0f, 0.0f);
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

        vertexOffset += 2;
    }

    uint indexOffset = 0;

    uint numTriangles = numCountourPoints - 2;

    // 底面の三角形を生成
    for (i = 0; i < numTriangles; i++)
    {
        TriangleIndices[indexOffset++] = 0;
        TriangleIndices[indexOffset++] = i + 1;
        TriangleIndices[indexOffset++] = i + 2;
    }

    //上面の三角形を生成
    for (i = 0; i < numTriangles; i++)
    {
        TriangleIndices[indexOffset++] = i + numCountourPoints + 2;
        TriangleIndices[indexOffset++] = i + numCountourPoints + 1;
        TriangleIndices[indexOffset++] = numCountourPoints;
    }

    // 側面の三角形を生成
    // 頂点は反時計回りに並んでいる
    for (i = 0; i < numCountourPoints; i++)
    {
        uint lb = i;
        uint rb = (lb + 1) % vertexCount;
        uint lt = i + numCountourPoints;
        uint rt = (lt + 1) % vertexCount;

        float3 v0 = VertexBuffer[lb].position.xyz;
        float3 v1 = VertexBuffer[rb].position.xyz;
        float3 v2 = VertexBuffer[lt].position.xyz;

        float3 edge0 = normalize(v1 - v0); // 左下から右下へのベクトル 右方向のベクトル
        float3 edge1 = normalize(v2 - v0); // 左下から左上へのベクトル 上方向のベクトル

        float3 crossVector = normalize(cross(edge1, edge0));

        uint copyVertexOffset = vertexOffset;

        VertexData triangleVertex = VertexBuffer[lb];
        triangleVertex.nomal = normalize(crossVector);

        VertexBuffer[copyVertexOffset++] = triangleVertex;

        triangleVertex.position = VertexBuffer[rb].position;
        VertexBuffer[copyVertexOffset++] = triangleVertex;

        triangleVertex.position = VertexBuffer[lt].position;
        VertexBuffer[copyVertexOffset++] = triangleVertex;

        triangleVertex.position = VertexBuffer[rt].position;
        VertexBuffer[copyVertexOffset++] = triangleVertex;


        TriangleIndices[indexOffset++] = vertexOffset;
        TriangleIndices[indexOffset++] = vertexOffset + 2;
        TriangleIndices[indexOffset++] = vertexOffset + 1;

        TriangleIndices[indexOffset++] = vertexOffset + 2;
        TriangleIndices[indexOffset++] = vertexOffset + 3;
        TriangleIndices[indexOffset++] = vertexOffset + 1;

        vertexOffset = copyVertexOffset;

    }



    // 生成した三角形の数を記録
    CounterBuffer[1] = indexOffset / 3; // 三角形数
    // インデックスの総数を記録
    CounterBuffer[2] = indexOffset;
}