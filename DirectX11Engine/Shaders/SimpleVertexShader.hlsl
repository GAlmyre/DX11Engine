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
    float3 Tangent : TANGENT;
    float3 Binormal : BINORMAL;
	float2 TexCoord : TEXCOORD;
    float2 ReturnTex : RETTEX;
};

VS_OUTPUT main( float4 pos : POSITION, float3 Normal : NORMAL, float3 Tangent : TANGENT, float3 Binormal : BINORMAL, float2 TexCoord : TEXCOORD )
{
    VS_OUTPUT Output;
	
    Output.Pos = mul(pos, WorldViewProj);
    
    Output.WorldPos = mul(pos, WorldMatrix);
    
    // Normal, tangent and binormal
    Output.Normal = mul(Normal, (float3x3)WorldMatrix);
    Output.Normal = normalize(Output.Normal);
    
    Output.Tangent = mul(Tangent, (float3x3) WorldMatrix);
    Output.Tangent = normalize(Output.Tangent);
    
    Output.Binormal = mul(Binormal, (float3x3) WorldMatrix);
    Output.Binormal = normalize(Output.Binormal);
    
    Output.TexCoord = TexCoord;
    Output.ReturnTex = TexCoord;
	
	return Output;
}