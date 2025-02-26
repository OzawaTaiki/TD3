#include "Resources/Shader/Particle.hlsli"

struct ParticleForGPU
{
    float4x4 matWorld;
    float4 color;
};

StructuredBuffer<ParticleForGPU> gParticle : register(t0);

cbuffer gViewProjection : register(b0)
{
    float4x4 matView;
    float4x4 matProj;
    float3 worldPosition;
};

struct VertexShaderInput
{
    float4 position : POSITION0;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
};

VertexShaderOutput main(VertexShaderInput _input, uint instanceID : SV_InstanceID)
{
    VertexShaderOutput output;
    output.position = mul(_input.position, mul(gParticle[instanceID].matWorld, mul(matView, matProj)));
    output.texcoord = _input.texcoord;
    output.color = gParticle[instanceID].color;
    return output;
}



