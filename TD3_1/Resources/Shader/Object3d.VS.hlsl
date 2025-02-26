#include "Resources/Shader/Object3d.hlsli"
//#include "Object3d.hlsli"

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




VertexShaderOutput main(VertexShaderInput _input)
{
    VertexShaderOutput output;
    output.position = mul(_input.position, mul(World, mul(matView, matProj)));
    output.texcoord = _input.texcoord;
    output.normal = normalize(mul(_input.normal, (float3x3) worldInverseTranspose));
    output.worldPosition = mul(_input.position, World).xyz;
    output.shadowPos = mul(mul(_input.position, World), DL.lightVP);
    return output;
}
