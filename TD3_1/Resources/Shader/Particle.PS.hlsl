#include "Resources/Shader/Particle.hlsli"

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

PixelShaderOutput main(VertexShaderOutput _input)
{
    PixelShaderOutput output;
    float4 textureColor = gTexture.Sample(gSampler, _input.texcoord);

    //画像の有無

    output.color =  textureColor * _input.color;
    if (output.color.a == 0.0)
    {
        discard;
    }

    return output;
}