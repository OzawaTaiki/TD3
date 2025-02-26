#include "Resources/Shader/LineDrawer.hlsli"
//#include "LineDrawer.hlsli"

float4 main(VertexShaderOutput _input) : SV_TARGET
{
    return _input.color;
}