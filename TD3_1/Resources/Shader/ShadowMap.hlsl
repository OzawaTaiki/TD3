#include "Resources/Shader/Object3d.hlsli"


cbuffer TransformationMatrix : register(b1)
{
    float4x4 World;
    float4x4 worldInverseTranspose;
};

struct VertexShaderInput
{
    float4 position : POSITION0;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
};

cbuffer gMaterial : register(b1)
{
    float4x4 unTransform;
    float shininess;
    int enableLighting;
};

cbuffer gColor : register(b2)
{
    float4 materialColor;
}


struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

VertexShaderOutput ShadowMapVS(VertexShaderInput _input)
{

    VertexShaderOutput output;
    output.position = mul(_input.position, mul(World, DL.lightVP));
    output.texcoord = _input.texcoord;
    output.normal = _input.normal;
    output.worldPosition = mul(_input.position, World).xyz;
    output.shadowPos = mul(float4(output.worldPosition, 1.0f), DL.lightVP);


    return output;
}



PixelShaderOutput ShadowMapPS(VertexShaderOutput _input)
{
    PixelShaderOutput output;
    output.color = float4(0.5f, 0.5f, 0.5f, 1.0f);

    return output;
}