#include  "Resources/Shader/FullScreen.hlsli"
//#include "FullScreen.hlsli"

static const uint kNumVertices = 3;
static const float4 kPosition[kNumVertices] =
{
    { -1.0f, 1.0f, 0.0f, 1.0f }, // top left
    { 3.0f, 1.0f, 0.0f, 1.0f }, // top right
    { -1.0f, -3.0f, 0.0f, 1.0f }, // bottom left
};

static const float2 kUV[kNumVertices] =
{
    { 0.0f, 0.0f }, // top left
    { 2.0f, 0.0f }, // top right
    { 0.0f, 2.0f }, // bottom left
};

VertexOutput main(uint _id : SV_VertexID)
{
    VertexOutput output;
    output.position = kPosition[_id];
    output.uv = kUV[_id];
    return output;
}