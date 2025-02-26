#include "Resources/Shader/Sprite.hlsli"
//#include "Sprite.hlsli"

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

cbuffer gColor : register(b1)
{
    float4 color;
}

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

PixelShaderOutput main(VertexShaderOutput _input)
{
    PixelShaderOutput output;
    float4 transformedUV = mul(float4(_input.texcoord, 0.0f, 1.0f), uvTransMat);
    output.color = gTexture.Sample(gSampler, transformedUV.xy) * color;
    return output;
}