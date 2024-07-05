Texture2D Texture : register(t0);
Texture2D NormalMap : register(t1);

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
    PointLight Lights[5];
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

float3 AmbientLighting(float4 LightAmbient)
{
    return /*CurrentMaterial.AmbientColor **/ LightAmbient;
}

float3 DiffuseLighting(float3 N, float3 L, float4 LightDiffuse, float2 TexCoord)
{
    float DiffuseTerm = saturate(dot(N, L));
	return Texture.Sample(ObjectSamplerState, TexCoord) * LightDiffuse * DiffuseTerm;
}

float3 SpecularLighting(float3 N, float3 L, float3 V, float4 LightSpecular)
{
    float SpecularTerm = 0;
    
    if (dot(N, L) > 0)
    {
        // half vector
        float3 H = normalize(L + V);
        // 64 = shininess
        SpecularTerm = pow(clamp(dot(N, H), 0, 1), CurrentMaterial.SpecExponent);
    }
    
    return LightSpecular * SpecularTerm;
}

float3 CalculateDirectional(DirectionalLight Light, float3 Normal, float3 ViewDir, float2 TexCoord)
{ 
    float3 LightDir = normalize(-Light.Dir);
    
    float3 Ambient = AmbientLighting(Light.Ambient);
    float3 Diffuse = DiffuseLighting(Normal, LightDir, Light.Diffuse, TexCoord);
    float3 Specular = SpecularLighting(Normal, LightDir, ViewDir, Light.Specular);
    
    return Ambient + Diffuse + Specular;
}

float3 CalculatePointLight(PointLight Light, float3 WorldPosition, float3 Normal, float3 ViewDir, float2 TexCoord)
{
    float3 LightDir = normalize(Light.Position - WorldPosition);
    float Distance = length(Light.Position - WorldPosition);
    float Attenuation = 1.0 / (Light.Attenuation.x + Light.Attenuation.y * Distance + Light.Attenuation.z * (Distance * Distance));

    
    float3 Ambient = AmbientLighting(Light.Ambient) * Attenuation;
    float3 Diffuse = DiffuseLighting(Normal, LightDir, Light.Diffuse, TexCoord) * Attenuation;
    float3 Specular = SpecularLighting(Normal, LightDir, ViewDir, Light.Specular) * Attenuation;
    
    return Ambient + Diffuse + Specular;
}

float4 main(PS_INPUT input) : SV_TARGET
{  
    float3 V = normalize(CamPosition - input.WorldPos.xyz);

    float4 TextureColor = Texture.Sample(ObjectSamplerState, input.TexCoord);
    
    // Normal mapping
    float4 BumpMap = NormalMap.Sample(ObjectSamplerState, input.TexCoord);
    BumpMap = (BumpMap * 2.0f) - 1.0f;
    float3 BumpNormal = (BumpMap.x * input.Tangent) + (BumpMap.y * input.Binormal) + (BumpMap.z * input.Normal);
    BumpNormal = normalize(BumpNormal);
    
    // Uncomment to see without bumpmapping
    //BumpNormal = normalize(input.Normal);
    
    if (TextureColor.a < 0.01)
        discard;
    
    float3 FinalColor = TextureColor * CalculateDirectional(Sun, BumpNormal, V, input.TexCoord);
    
    for (int i = 0; i < LightsCount; i++)
    {
        FinalColor += TextureColor * CalculatePointLight(Lights[i], input.WorldPos.xyz, BumpNormal, V, input.TexCoord);
    }
    
    return float4(saturate(FinalColor), 1.0f);
}