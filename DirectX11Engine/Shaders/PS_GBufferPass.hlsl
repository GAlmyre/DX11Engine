Texture2D Texture;
SamplerState ObjectSamplerState;

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float4 WorldPos : POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD;
    float2 ReturnTex : RETTEX;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    float3 FinalColor = Texture.Sample(ObjectSamplerState, input.TexCoord);
    
    return float4(saturate(FinalColor), 1.0f);    
}