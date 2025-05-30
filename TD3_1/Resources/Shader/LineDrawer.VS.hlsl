#include "Resources/Shader/LineDrawer.hlsli"
//#include "LineDrawer.hlsli"

struct VSInput
{
	float4 position : POSITION0;
    float4 color : COLOR0;
};

VertexShaderOutput main(VSInput _input)
{
	VertexShaderOutput output;
	output.position = mul(_input.position, vp);
    output.color = _input.color;
	return output;
}