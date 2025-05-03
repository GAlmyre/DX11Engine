struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float4 DepthPos : TEXTURE0;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    float DepthValue;
    
    DepthValue = input.DepthPos.z / input.DepthPos.w;
    
    return float4(DepthValue, DepthValue, DepthValue, 1.0f);
}