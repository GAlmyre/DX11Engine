Texture2D Texture;
SamplerState ObjectSamplerState;

struct Material
{
    float3 AmbientColor;
    float3 DiffuseColor;
    float3 SpecularColor;
    float SpecExponent;
};

struct PointLight
{
    float3 Position;
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
    float3 Attenuation;
    float Range;
};

struct DirectionalLight
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
    float3 Dir;
};

cbuffer cbPerFrame
{
    // The directional light of our scene
    DirectionalLight Sun;
    PointLight Light;
    float3 CamPosition;
    float LightsCount;
};

cbuffer cbPerObject
{
    // The directional light of our scene
    Material CurrentMaterial;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float4 WorldPos : POSITION;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 Binormal : BINORMAL;
    float2 TexCoord : TEXCOORD;
    float2 ReturnTex : RETTEX;
};

float4 main(PS_INPUT input) : SV_TARGET
{  
    float3 RemappedNormal = (input.Normal * 0.5) + 0.5;

    float3 FinalColor = RemappedNormal;
    return float4(saturate(FinalColor), 1.0f);    
}