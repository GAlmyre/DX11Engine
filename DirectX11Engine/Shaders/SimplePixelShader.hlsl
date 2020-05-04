Texture2D Texture;
SamplerState ObjectSamplerState;

struct Light
{
    float3 Dir;
    float3 Pos;
    float Range;
    float3 Attenuation;
    float4 Ambient;
    float4 Diffuse;
};

cbuffer cbPerFrame
{
	// The directional light of our scene
    Light CurrentLight;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float4 WorldPos : POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    input.Normal = normalize(input.Normal);
    
    float4 Diffuse = Texture.Sample(ObjectSamplerState, input.TexCoord);
    
    float3 FinalColor = float3(0.0f, 0.0f, 0.0f);
    
    // distance between light and worldPos
    float3 LightVector = CurrentLight.Pos - input.WorldPos;
    float LightDistance = length(LightVector);
    
    float3 FinalAmbient = Diffuse * CurrentLight.Ambient;
    
    // The light is too far
    if (LightDistance > CurrentLight.Range)
    {
        return float4(FinalAmbient, Diffuse.a);
    }     
        
    // normalize
    LightVector /= LightDistance;
    
    float LightQuantity = dot(LightVector, input.Normal);
    
    if (LightQuantity > 0.0f)
    {
        FinalColor += LightQuantity * Diffuse * CurrentLight.Diffuse;
        // falloff
        FinalColor /= CurrentLight.Attenuation[0] + (CurrentLight.Attenuation[1] * LightDistance) + CurrentLight.Attenuation[2] * (LightDistance * LightDistance);
    }
        
    FinalColor = saturate(FinalColor + FinalAmbient);
    
    return float4(FinalColor, Diffuse.a);
}