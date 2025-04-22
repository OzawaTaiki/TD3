struct VSInput
{
    float4 position : POSITION0;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
};

struct VSOutput
{
    float4 position : SV_Position;
};

struct GSOutput
{
    float4 Position : SV_POSITION; // クリップ空間の座標
    float4 WorldPos : TEXCOORD0; // ワールド座標（距離計算用）
    uint RTIndex : SV_RenderTargetArrayIndex; // レンダーターゲット配列インデックス
};

struct PSOutput
{
    float4 data : SV_TARGET0;
};

cbuffer TransformationMatrix : register(b0)
{
    float4x4 World;
    float4x4 worldInverseTranspose;
};


struct PointLight
{

    float4 color;

    float3 position;
    float intensity;

    float radius;
    float decay;
    int isHalf;
    int castShadow;

    float shadowFactor;
    float3 pad;

    float4x4 lightVP[6];
};

cbuffer gLightGroup : register(b1)
{
    PointLight PL;
};

cbuffer id : register(b2)
{
    uint id;
}

VSOutput VSmain(VSInput _input)
{
    VSOutput output;

    output.position = mul(_input.position, World);

    return output;
}

[maxvertexcount(18)] // 三角形 × 6面 = 最大18頂点
void GSmain(triangle VSOutput input[3], inout TriangleStream<GSOutput> TriStream)
{
    // 6つの面（キューブマップの各方向）に対して処理
    for (uint face = 0; face < 6; face++)
    {
        // 三角形の3頂点を処理
        for (uint v = 0; v < 3; v++)
        {
            GSOutput output;

            // ワールド座標を保存（距離計算用）
            output.WorldPos = input[v].position; // 既にワールド座標

            // ライトの視点でのクリップ空間座標に変換
            output.Position = mul(input[v].position, PL.lightVP[face]);

            // どのキューブマップの面にレンダリングするかを指定
            output.RTIndex = face;

            TriStream.Append(output);
        }

        // 三角形ストリップの終了
        TriStream.RestartStrip();
    }
}

PSOutput PSmain(GSOutput input)
{
    PSOutput output;

    float r = (id & 0xFF) / 255.0; // 下位8bit
    float g = ((id >> 8) & 0xFF) / 255.0; // 中位8bit
    float b = ((id >> 16) & 0xFF) / 255.0; // 上位8bit

    float a = 1.0f;
    output.data = float4(r, g, b, a);

    return output;
}
