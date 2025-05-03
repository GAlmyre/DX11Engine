cbuffer cbPerObject
{
	// pre multiplied world view projection matrix
    float4x4 WorldViewProj;
    float4x4 WorldMatrix;
    float4x4 LightWorldViewProj;
};

struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
    float4 WorldPos : POSITION;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 Binormal : BINORMAL;
	float2 TexCoord : TEXCOORD;
    float4 LightViewPos : TEXCOORD1;
    float3 LightPos : TEXCOORD2;
};

VS_OUTPUT main( float4 pos : POSITION, float3 Normal : NORMAL, float3 Tangent : TANGENT, float3 Binormal : BINORMAL, float2 TexCoord : TEXCOORD )
{
    VS_OUTPUT Output;
    pos.w = 1.0f;
    
    Output.Pos = mul(pos, WorldViewProj);
    // Position of the vertex viewed by the light
    Output.LightViewPos = mul(pos, LightWorldViewProj);
    //Output.Pos = Output.LightViewPos;
    
    Output.WorldPos = mul(pos, WorldMatrix);
         
    Output.LightPos = /*LightPos.xyz - */Output.WorldPos.xyz;
    //Output.LightPos = normalize(LightPos);
    
    // Normal, tangent and binormal
    Output.Normal = mul(Normal, (float3x3)WorldMatrix);
    Output.Normal = normalize(Output.Normal);
    
    Output.Tangent = mul(Tangent, (float3x3) WorldMatrix);
    Output.Tangent = normalize(Output.Tangent);
    
    Output.Binormal = mul(Binormal, (float3x3) WorldMatrix);
    Output.Binormal = normalize(Output.Binormal);
    
    Output.TexCoord = TexCoord; 
	
	return Output;
}