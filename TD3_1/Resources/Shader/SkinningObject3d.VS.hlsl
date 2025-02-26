#include "Resources/Shader/Object3d.hlsli"
//#include "./Object3d.hlsli"

struct Well
{
    float4x4 skeletonSpaceMatrix;
    float4x4 skeletonSpaceInversedTransposeMatrix;
};

StructuredBuffer<Well> gMatrixPalette : register(t0);

struct Skinned
{
    float4 position;
    float3 normal;
};

cbuffer TransformationMatrix : register(b1)
{
    float4x4 World;
    float4x4 worldInverseTranspose;
};

struct VertexShaderInput
{
    float4 position : POSITION0;
    float3 normal : NORMAL0;
    float2 texcoord : TEXCOORD0;
    float4 weight : WEIGHT0;
    int4 index : INDEX0;
};
Skinned Skinnning(VertexShaderInput _input);

VertexShaderOutput main(VertexShaderInput _input)
{
    VertexShaderOutput output;
    Skinned skinned = Skinnning(_input);
    output.position = mul(skinned.position, mul(World, mul(matView, matProj)));
    output.worldPosition = mul(skinned.position, World).xyz;
    output.texcoord = _input.texcoord;
    output.normal = normalize(mul(skinned.normal, (float3x3) worldInverseTranspose));
    return output;
}

Skinned Skinnning(VertexShaderInput _input)
{
    Skinned skinned;

    int totalWeight = _input.weight.x + _input.weight.y + _input.weight.z + _input.weight.w;

    if (totalWeight == 0)
    {
        skinned.position = mul(_input.position, gMatrixPalette[_input.index.x].skeletonSpaceMatrix);
        skinned.normal = mul(_input.normal, (float3x3) gMatrixPalette[_input.index.x].skeletonSpaceInversedTransposeMatrix);
        return skinned;
    }

    skinned.position = mul(_input.position, gMatrixPalette[_input.index.x].skeletonSpaceMatrix) * _input.weight.x;
    skinned.position += mul(_input.position, gMatrixPalette[_input.index.y].skeletonSpaceMatrix) * _input.weight.y;
    skinned.position += mul(_input.position, gMatrixPalette[_input.index.z].skeletonSpaceMatrix) * _input.weight.z;
    skinned.position += mul(_input.position, gMatrixPalette[_input.index.w].skeletonSpaceMatrix) * _input.weight.w;
    skinned.position.w = 1.0f;

    skinned.normal = mul(_input.normal, (float3x3) gMatrixPalette[_input.index.x].skeletonSpaceInversedTransposeMatrix) * _input.weight.x;
    skinned.normal += mul(_input.normal, (float3x3) gMatrixPalette[_input.index.y].skeletonSpaceInversedTransposeMatrix) * _input.weight.y;
    skinned.normal += mul(_input.normal, (float3x3) gMatrixPalette[_input.index.z].skeletonSpaceInversedTransposeMatrix) * _input.weight.z;
    skinned.normal += mul(_input.normal, (float3x3) gMatrixPalette[_input.index.w].skeletonSpaceInversedTransposeMatrix) * _input.weight.w;
    skinned.normal = normalize(skinned.normal);

    return skinned;
}