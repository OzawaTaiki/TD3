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

struct PixelShaderOutput
{
    float4 data : SV_TARGET0;
};

cbuffer gID : register(b2)
{
    uint id;
}

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

    float r = (id & 0xFF) / 255.0; // 下位8bit
    float g = ((id >> 8) & 0xFF) / 255.0; // 中位8bit
    float b = ((id >> 16) & 0xFF) / 255.0; // 上位8bit

    float a = 1.0f;
    output.data = float4(r, g, b, a);

    return output;
}