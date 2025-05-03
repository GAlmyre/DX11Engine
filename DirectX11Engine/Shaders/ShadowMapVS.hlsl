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
    float4 DepthPos : TEXTURE0;
};

VS_OUTPUT main( float4 pos : POSITION )
{
    VS_OUTPUT Output;
    pos.w = 1.0f;
    Output.Pos = mul(pos, WorldViewProj);
    
    Output.DepthPos = Output.Pos;
	
	return Output;
}