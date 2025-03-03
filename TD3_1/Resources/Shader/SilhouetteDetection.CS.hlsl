struct VertexData
{
    float4 position;
    float2 texcoord;
    float3 normal;
};

struct VertexIndex
{
    uint index;
};

struct SilhouetteEdge
{
    uint v1; // 頂点インデックス
    uint v2; // 頂点インデックス
    uint isSilhouette; // シルエットエッジかどうか
};

struct ConstData
{
    float3 lightDir;
    uint indexCount;
    uint vertexCount;
    uint debug_FrameCount;
};

struct EdgeInfo
{
    uint triangleA; // このエッジを持つ三角形１のインデックス
    uint triangleB; // このエッジを持つ三角形２のインデックス (存在すれば)
    uint isBoundary; // 境界エッジかどうか
    uint processed; // 処理済みかどうか
};

// 新しい構造体: エッジの頂点ペア情報
struct EdgeVertexPair
{
    uint v1;
    uint v2;
};

StructuredBuffer<VertexData> gVertices : register(t0); // モデルの頂点データ
StructuredBuffer<VertexIndex> gIndices : register(t1); // モデルのインデックスデータ

RWStructuredBuffer<EdgeInfo> gEdgeInfo : register(u0); // エッジ情報
RWStructuredBuffer<uint> gCounters : register(u1); // カウンター [0]:エッジ数, [1]:シルエット数
RWStructuredBuffer<SilhouetteEdge> gSilhouetteEdges : register(u2); // シルエットエッジ
RWStructuredBuffer<EdgeVertexPair> gEdgeVertexPairs : register(u3); // エッジの頂点ペア情報

cbuffer gConstData : register(b0)
{
    ConstData constData;
}

// エッジハッシュキーを生成する関数
uint GetEdgeKey(uint v1, uint v2)
{
    // v1とv2を正規化（小さい方を先に）
    uint minVertex = min(v1, v2);
    uint maxVertex = max(v1, v2);

    // 簡易的なハッシュキー生成
    return minVertex * constData.vertexCount + maxVertex;
}

float3 CalculateTriangleNormal(uint triangleIndex)
{
    uint i0 = gIndices[triangleIndex * 3].index;
    uint i1 = gIndices[triangleIndex * 3 + 1].index;
    uint i2 = gIndices[triangleIndex * 3 + 2].index;

    float3 p0 = gVertices[i0].position.xyz;
    float3 p1 = gVertices[i1].position.xyz;
    float3 p2 = gVertices[i2].position.xyz;

    float3 edge1 = p1 - p0;
    float3 edge2 = p2 - p0;

    return normalize(cross(edge1, edge2));
}

// エッジ情報を生成する関数
// エッジ情報を生成する関数
[numthreads(256, 1, 1)]
void CS_CreateEdgeInfo(uint3 DTid : SV_DispatchThreadID)
{
    uint triangleID = DTid.x; // スレッドIDを三角形のインデックスとして使用
    if (triangleID >= constData.indexCount / 3)
    {
        return;
    }
    uint indexBase = triangleID * 3;

    for (uint i = 0; i < 3; i++)
    {
        uint index1 = gIndices[indexBase + i].index;
        uint index2 = gIndices[indexBase + (i + 1) % 3].index;

        // 頂点インデックスを正規化（小さい順に）
        uint minVertex = min(index1, index2);
        uint maxVertex = max(index1, index2);

        // エッジキーを計算
        uint edgeKey = GetEdgeKey(minVertex, maxVertex);

        uint originalTriangleA;
        InterlockedCompareExchange(gEdgeInfo[edgeKey].triangleA, 0xFFFFFFFF, triangleID, originalTriangleA);

        // 初めて見つけたエッジの場合（まだ未割り当て = 0xFFFFFFFF）
        if (originalTriangleA == 0xFFFFFFFF)
        {
            // エッジカウンターをインクリメント
            uint edgeCounter;
            InterlockedAdd(gCounters[0], 1, edgeCounter);

            // エッジの頂点ペア情報を保存
            gEdgeVertexPairs[edgeCounter].v1 = minVertex;
            gEdgeVertexPairs[edgeCounter].v2 = maxVertex;

            // エッジ情報を設定
            gEdgeInfo[edgeKey].triangleA = triangleID;
            gEdgeInfo[edgeKey].triangleB = 0xFFFFFFFF;
            gEdgeInfo[edgeKey].isBoundary = 1;
        }
        // すでに登録されているエッジの場合
        else if (originalTriangleA != triangleID)
        {
            // 共有エッジなので、境界ではない
            gEdgeInfo[edgeKey].triangleB = triangleID;
            gEdgeInfo[edgeKey].isBoundary = 0;
        }
    }
}

// シルエットエッジを検出する関数
[numthreads(256, 1, 1)]
void CS_DetectSilhouette(uint3 DTid : SV_DispatchThreadID)
{
    uint edgeID = DTid.x;

    if (edgeID >= gCounters[0])
    {
        return;
    }

    // エッジの頂点ペア情報を直接取得
    EdgeVertexPair vertexPair = gEdgeVertexPairs[edgeID];
    uint v1 = vertexPair.v1;
    uint v2 = vertexPair.v2;

    // エッジキーを計算
    uint edgeKey = GetEdgeKey(v1, v2);

    // エッジ情報を取得
    EdgeInfo edgeInfo = gEdgeInfo[edgeKey];

    // すでに処理済みのエッジはスキップ
    if (edgeInfo.processed != 0)
    {
        return;
    }

    bool isSilhouette = false;

    if (edgeInfo.isBoundary != 0)
    {
        // 境界エッジに関連する唯一の三角形の法線を取得
        float3 normal = CalculateTriangleNormal(edgeInfo.triangleA);

        // 光源方向と法線のドット積で表裏判定
        float dotProduct = dot(normal, constData.lightDir);

        // 裏向きなら、このエッジはシルエット
        isSilhouette = (dotProduct < 0.0f);
        // 光源とポリゴンがほぼ平行の場合も考慮
        if (abs(dotProduct) < 0.01f)
        {
        // 光源とほぼ平行な面の場合、エッジをシルエットとして扱う
            isSilhouette = true;
        }
    }
    else
    {
        // 両方の三角形の法線を取得
        float3 normalA = CalculateTriangleNormal(edgeInfo.triangleA);
        float3 normalB = CalculateTriangleNormal(edgeInfo.triangleB);

        // 両方の法線と光源方向のドット積
        float dotProductA = dot(normalA, constData.lightDir);
        float dotProductB = dot(normalB, constData.lightDir);

        // 一方が表で一方が裏ならシルエット
        isSilhouette = (sign(dotProductA) != sign(dotProductB));
    }
    InterlockedAdd(gCounters[2], 1);

    // エッジを処理済みとマーク
    gEdgeInfo[edgeKey].processed = 1;

    // シルエットとして検出された場合、出力に追加
    if (isSilhouette)
    {
        InterlockedAdd(gCounters[3], 1);

        uint silhouetteID;
        InterlockedAdd(gCounters[1], 1, silhouetteID);

        // シルエットエッジとして記録
        gSilhouetteEdges[silhouetteID].v1 = v1;
        gSilhouetteEdges[silhouetteID].v2 = v2;
        gSilhouetteEdges[silhouetteID].isSilhouette = 1;
    }
}