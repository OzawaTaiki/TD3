struct VertexShaderOutput
{
    float4 position : SV_Position;
    float2 texcoord : TEXCOORD0;
};

cbuffer constBuf0 : register(b0)
{
    float4x4 affineMat;
    float4x4 uvTransMat;
};