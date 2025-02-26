struct VertexShaderOutput
{
    float4 position : SV_Position;
    float4 color : COLOR0;
};

cbuffer constBuf0 : register(b0)
{
    float4x4 vp;
	float4 color;
}