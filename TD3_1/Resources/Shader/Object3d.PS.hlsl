#include "Resources/Shader/Object3d.hlsli"
//#include "Object3d.hlsli"


cbuffer gMaterial : register(b1)
{
    float4x4 unTransform;
    float shininess;
    int enableLighting;
};

//cbuffer gTexVisibility : register(b1)
//{
//    float isVisible;
//};

cbuffer gColor : register(b2)
{
    float4 materialColor;
}


struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

Texture2D<float> gShadowMap : register(t1);
SamplerComparisonState gShadowSampler : register(s1);

TextureCube<float4> gCubeMap : register(t2);
SamplerState gCubeMapSampler : register(s2);

float3 CalculateDirectionalLighting(VertexShaderOutput _input, float3 _toEye, float4 _textureColor);
float3 CalculatePointLighting(VertexShaderOutput _input, PointLight _PL, int _lightIndex, float3 _toEye, float4 _textureColor);
float3 CalculateSpotLighting(VertexShaderOutput _input, SpotLight _SL, float3 _toEye, float4 _textureColor);

float3 CalculateLightingWithMultiplePointLights(VertexShaderOutput _input, float3 _toEye, float4 _textureColor);
float3 CalculateLightingWithMultipleSpotLights(VertexShaderOutput _input, float3 _toEye, float4 _textureColor);

float ComputeShadow(float4 shadowCoord)
{
    shadowCoord.xyz /= shadowCoord.w;
    shadowCoord.x = shadowCoord.x * 0.5 + 0.5;
    shadowCoord.y = -shadowCoord.y * 0.5 + 0.5;

    // 現在のピクセルの深度値
    float currentDepth = shadowCoord.z;

    // シャドウマップに記録された深度値
    float closestDepth = gShadowMap.Sample(gSampler, shadowCoord.xy).r;

    // 深度比較による影の判定
    float shadow = (currentDepth > closestDepth + 0.001f) ? DL.shadowFactor : 1.0f;
    return shadow;
}
float ComputePointLightShadow(int lightIndex, float3 worldPos, PointLight _PL)
{
    // 安全性のチェック
    if (lightIndex < 0 || lightIndex >= MAX_POINT_LIGHT || !_PL.castShadow)
        return 1.0f;

    // ライト位置から現在のワールド座標へのベクトル計算
    float3 lightToWorldVec = worldPos - _PL.position;

    // キューブマップの適切な面を選択
    float3 absVec = abs(lightToWorldVec);
    float maxComponent = max(max(absVec.x, absVec.y), absVec.z);
    int faceIndex = 0;

    if (maxComponent == absVec.x)
        faceIndex = lightToWorldVec.x > 0 ? 0 : 1;
    else if (maxComponent == absVec.y)
        faceIndex = lightToWorldVec.y > 0 ? 2 : 3;
    else
        faceIndex = lightToWorldVec.z > 0 ? 4 : 5;

    // シャドウ計算のロジック
    float currentDepth = length(lightToWorldVec) / _PL.radius;

    // 対応するライトのシャドウマップをサンプリング
    float closestDepth = gCubeMap.Sample(
        gCubeMapSampler,
        lightToWorldVec
    ).r;

    // シャドウバイアスを考慮
    float bias = 0.005;
    float shadow = currentDepth > closestDepth + bias ? _PL.shadowFactor : 1.0;


    return shadow;
}



PixelShaderOutput main(VertexShaderOutput _input)
{
    PixelShaderOutput output;
    output.color = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 textureColor;

    float4 transformedUV = mul(float4(_input.texcoord, 0.0f, 1.0f), unTransform);
    textureColor = materialColor * gTexture.Sample(gSampler, transformedUV.xy);

    float3 toEye = normalize(worldPosition - _input.worldPosition);

    float shadowFactor = ComputeShadow(_input.shadowPos);

    // シャドウファクターを適用したライティング
    float3 directionalLight = CalculateDirectionalLighting(_input, toEye, textureColor) * shadowFactor;
    float3 pointLight = CalculateLightingWithMultiplePointLights(_input, toEye, textureColor);
    float3 spotLightcColor = CalculateLightingWithMultipleSpotLights(_input, toEye, textureColor) * shadowFactor;



    if (enableLighting != 0)
    {
        output.color.rgb = directionalLight + pointLight + spotLightcColor;
        output.color.a = materialColor.a * textureColor.a;
    }
    else
        output.color = materialColor * textureColor;

    if (textureColor.a == 0.0 ||
        output.color.a == 0.0)
    {
        discard;
    }

    return output;
}

float3 CalculateDirectionalLighting(VertexShaderOutput _input, float3 _toEye, float4 _textureColor)
{
    if (DL.intensity <= 0.0f)
        return float3(0.0f, 0.0f, 0.0f);

    float3 HalfVector = normalize(-DL.direction + _toEye);
    float specularPow = pow(saturate(dot(normalize(_input.normal), HalfVector)), shininess);
    float NdotL = dot(normalize(_input.normal), -DL.direction);
    float cos = saturate(NdotL);
    if (DL.isHalf != 0)
    {
        cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
    }
    float3 diffuse = materialColor.rgb * _textureColor.rgb * DL.color.rgb * cos * DL.intensity;
    float3 specular = DL.color.rgb * DL.intensity * specularPow * float3(1.0f, 1.0f, 1.0f);

    return diffuse + specular;
}

float3 CalculatePointLighting(VertexShaderOutput _input,PointLight _PL, int _lightIndex,float3 _toEye, float4 _textureColor)
{
    if (_PL.intensity <= 0.0f)
        return float3(0.0f, 0.0f, 0.0f);

    float3 direction = normalize(_input.worldPosition - _PL.position);
    float3 HalfVector = normalize(-direction + _toEye);
    float specularPow = pow(saturate(dot(normalize(_input.normal), HalfVector)), shininess);
    float NdotL = dot(normalize(_input.normal), -direction);
    float cos = saturate(NdotL);
    if (_PL.isHalf != 0)
    {
        cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
    }
    float distance = length(_PL.position - _input.worldPosition);
    float factor = pow(saturate(-distance / _PL.radius + 1.0f), _PL.decay);

    float shadowFactor = ComputePointLightShadow(_lightIndex, _input.worldPosition, _PL);

    float3 diffuse = materialColor.rgb * _textureColor.rgb * _PL.color.rgb * cos * _PL.intensity * factor * shadowFactor;
    float3 specular = _PL.color.rgb * _PL.intensity * specularPow * float3(1.0f, 1.0f, 1.0f) * factor * shadowFactor;

    return diffuse + specular;

}

float3 CalculateSpotLighting(VertexShaderOutput _input, SpotLight _SL, float3 _toEye, float4 _textureColor)
{
    if (_SL.intensity <= 0.0f)
        return float3(0.0f, 0.0f, 0.0f);


    float3 direction = normalize(_input.worldPosition - _SL.position);
    float3 HalfVector = normalize(-direction + _toEye);
    float specularPow = pow(saturate(dot(normalize(_input.normal), HalfVector)), shininess);

    float NdotL = dot(normalize(_input.normal), -direction);
    float cos = saturate(NdotL);
    if (_SL.isHalf != 0)
    {
        cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
    }

    float distance = length(_SL.position - _input.worldPosition);
    float factor = pow(saturate(-distance / _SL.distance + 1.0f), _SL.decay);

    float cosAngle = dot(direction, normalize(_SL.direction));
    float falloffFactor = 1.0f;
    if (cosAngle < _SL.cosFalloutStart)
    {
        falloffFactor = saturate((cosAngle - _SL.cosAngle) / (_SL.cosFalloutStart - _SL.cosAngle));
    }


    float3 diffuse = materialColor.rgb * _textureColor.rgb * _SL.color.rgb * cos * _SL.intensity * factor * falloffFactor;
    float3 specular = _SL.color.rgb * _SL.intensity * specularPow * float3(1.0f, 1.0f, 1.0f) * factor * falloffFactor;

    return diffuse + specular;

}

float3 CalculateLightingWithMultiplePointLights(VertexShaderOutput _input, float3 _toEye, float4 _textureColor)
{
    float3 lighting = float3(0.0f, 0.0f, 0.0f);
    for (int i = 0; i < numPointLight; i++)
    {
        lighting += CalculatePointLighting(_input, PL[i], i, _toEye, _textureColor);
    }
    return lighting;
}

float3 CalculateLightingWithMultipleSpotLights(VertexShaderOutput _input, float3 _toEye, float4 _textureColor)
{
    float3 lighting = float3(0.0f, 0.0f, 0.0f);
    for (int i = 0; i < numSpotLight; i++)
    {
        lighting += CalculateSpotLighting(_input, SL[i], _toEye, _textureColor);
    }
    return lighting;
}
