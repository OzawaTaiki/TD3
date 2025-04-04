struct VertexShaderOutput
{
    float4 position : SV_Position;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
    float3 worldPosition : Position0;
    float4 shadowPos : TEXCOORD1;
};

cbuffer Camera : register(b0)
{
    float4x4 matView;
    float4x4 matProj;
    float3 worldPosition;
}

struct DirectionalLight
{
    float4x4 lightVP;

    float4 color;

    float3 direction;
    float intensity;

    int isHalf;
    int castShadow;
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

    float4x4 lightVP[6];
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

    int castShadow;
    float3 pad;

    float4x4 lightVP;
};
static const int MAX_POINT_LIGHT = 32;
static const int MAX_SPOT_LIGHT = 32;
cbuffer gLightGroup : register(b3)
{
    DirectionalLight DL;
    PointLight PL[MAX_POINT_LIGHT];
    SpotLight SL[MAX_SPOT_LIGHT];
    int numPointLight;
    int numSpotLight;

}