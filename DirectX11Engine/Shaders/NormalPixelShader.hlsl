Texture2D Texture : register(t0);
Texture2D NormalMap : register(t1);
Texture2D SpecularMap : register(t2);
SamplerState ObjectSamplerState;

struct Material
{
    float4 AmbientColor;
    float4 DiffuseColor;
    float4 SpecularColor;
    float SpecExponent;
    int bUseAlbedoTexture;
    int bUseNormalMap;
    int bUseSpecularMap;
};

cbuffer cbPerObject
{
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
    float3 BumpNormal;
    if (CurrentMaterial.bUseNormalMap == 0)
    {
        BumpNormal = normalize(input.Normal);
    }
    else
    {
        float4 BumpMap = NormalMap.Sample(ObjectSamplerState, input.TexCoord);
        BumpMap = (BumpMap * 2.0f) - 1.0f;
        BumpNormal = (BumpMap.x * input.Tangent) + (BumpMap.y * input.Binormal) + (BumpMap.z * input.Normal);
        BumpNormal = normalize(BumpNormal);
    }
    float3 RemappedNormal = (BumpNormal * 0.5) + 0.5;
    
    return float4(RemappedNormal, 1.0f);
}