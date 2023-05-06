cbuffer cbPerObject
{
	// pre multiplied world view projection matrix
    float4x4 WorldViewProj;
    float4x4 WorldMatrix;
};

struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
    float4 WorldPos : POSITION;
    float3 Normal : NORMAL;
	float2 TexCoord : TEXCOORD;
    float2 ReturnTex : RETTEX;
};

VS_OUTPUT main( float4 pos : POSITION, float3 Normal : NORMAL, float2 TexCoord : TEXCOORD )
{
    VS_OUTPUT Output;
	
    Output.Pos = mul(pos, WorldViewProj);
    
    Output.WorldPos = mul(pos, WorldMatrix);
    Output.Normal = mul(Normal, WorldMatrix);
    
    Output.TexCoord = TexCoord;
    Output.ReturnTex = TexCoord;
	
	return Output;
}