#include "Resources/Shader/Sprite.hlsli"
//#include "Sprite.hlsli"

struct VertexShaderInput
{
    float4 position : POSITION0;
    float2 texcoord : TEXCOORD0;
};

VertexShaderOutput main(VertexShaderInput _input)
{
    VertexShaderOutput output;

    output.position = mul(_input.position, affineMat);
    output.texcoord = _input.texcoord;
    return output;
}

