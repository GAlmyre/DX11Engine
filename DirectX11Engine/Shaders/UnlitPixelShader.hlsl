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
    float3 FinalColor;
    if (CurrentMaterial.bUseAlbedoTexture == 0)
    {
        FinalColor = (float3)CurrentMaterial.DiffuseColor;
    }
    else
    {
        FinalColor = (float3)Texture.Sample(ObjectSamplerState, input.TexCoord);
    }
        
    return float4(saturate(FinalColor), 1.0f);    
}