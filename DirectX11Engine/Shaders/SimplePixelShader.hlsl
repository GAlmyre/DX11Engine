Texture2D Texture       : register(t0);
Texture2D NormalMap     : register(t1);
Texture2D SpecularMap   : register(t2);
Texture2D ShadowDepth   : register(t3);

SamplerState ObjectSamplerState : register(s0);
SamplerState SampleTypeClamp : register(s1);

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
    float3 Position;
};

cbuffer cbPerFrame
{
    // The directional light of our scene
    DirectionalLight Sun;
    PointLight Lights[5];
    float3 CamPosition;
    int LightsCount;
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
    float4 LightViewPos : TEXCOORD1;
    float3 LightPos : TEXCOORD2;
};

float3 AmbientLighting(float4 LightAmbient, float3 PixelColor)
{
    return PixelColor * (float3)LightAmbient;
}

float3 DiffuseLighting(float3 N, float3 L, float4 LightDiffuse, float2 TexCoord, float3 PixelColor)
{
    float DiffuseTerm = saturate(dot(N, L));
    return PixelColor * (float3)LightDiffuse * DiffuseTerm;

}

float3 SpecularLighting(float3 N, float3 L, float3 V, float4 LightSpecular, float SpecularMapValue, float3 PixelColor)
{
    float SpecularTerm = 0;
    
    if (dot(N, L) > 0)
    {
        // half vector
        float3 H = normalize(L + V);
        // 64 = shininess
        SpecularTerm = pow(clamp(dot(N, H), 0, 1), SpecularMapValue);
    }
    
    return (float3)LightSpecular * SpecularTerm;
}

float3 CalculateDirectional(DirectionalLight Light, float3 Normal, float3 ViewDir, float2 TexCoord, float SpecularMapValue, float3 PixelColor, float Shadow)
{
    float3 LightDir = normalize(-Light.Dir);
    
    float3 Ambient = AmbientLighting(Light.Ambient, PixelColor);
    float3 Diffuse = DiffuseLighting(Normal, LightDir, Light.Diffuse, TexCoord, PixelColor);
    float3 Specular = SpecularLighting(Normal, LightDir, ViewDir, Light.Specular, SpecularMapValue, PixelColor);
    
    return (Ambient + Diffuse + Specular) * (1.0 - Shadow);
}

float3 CalculatePointLight(PointLight Light, float3 WorldPosition, float3 Normal, float3 ViewDir, float2 TexCoord, float SpecularMapValue, float3 PixelColor, float Shadow)
{
    float3 LightDir = normalize(Light.Position - WorldPosition);
    float Distance = length(Light.Position - WorldPosition);
    float Attenuation = 1.0 / (Light.Attenuation.x + Light.Attenuation.y * Distance + Light.Attenuation.z * (Distance * Distance));

    
    float3 Ambient = AmbientLighting(Light.Ambient, PixelColor) * Attenuation;
    float3 Diffuse = DiffuseLighting(Normal, LightDir, Light.Diffuse, TexCoord, PixelColor) * Attenuation;
    float3 Specular = SpecularLighting(Normal, LightDir, ViewDir, Light.Specular, SpecularMapValue, PixelColor) * Attenuation;
    
    return Ambient + Diffuse + Specular;
}

float4 main(PS_INPUT input) : SV_TARGET
{
    float3 V = normalize(CamPosition - input.WorldPos.xyz);

    // Albedo mapping
    float4 TextureColor;
    if (CurrentMaterial.bUseAlbedoTexture == 0)
    {
        TextureColor = CurrentMaterial.DiffuseColor;
    }
    else
    {
        TextureColor = Texture.Sample(ObjectSamplerState, input.TexCoord);

    }
    
    // Specular mapping
    float SpecularMapValue;
    if (CurrentMaterial.bUseSpecularMap == 0)
    {
        SpecularMapValue = CurrentMaterial.SpecExponent;
    }
    else
    {
        SpecularMapValue = SpecularMap.Sample(ObjectSamplerState, input.TexCoord).x;
    }
    
    // Normal mapping
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
    
    // Shadow maping
    float2 ProjectedTexCoord;
    float DepthValue = 0.0f;
    float LightDepthValue;
    float Shadow = 0.0f;
    
    ProjectedTexCoord.x = input.LightViewPos.x / input.LightViewPos.w / 2.0f + 0.5f;
    ProjectedTexCoord.y = -input.LightViewPos.y / input.LightViewPos.w / 2.0f + 0.5f;
    
    // Find if we are in the projected viewport (0 to 1 range)
    if ((saturate(ProjectedTexCoord.x) == ProjectedTexCoord.x) && (saturate(ProjectedTexCoord.y) == ProjectedTexCoord.y))
    {
        DepthValue = ShadowDepth.Sample(SampleTypeClamp, ProjectedTexCoord).r;
        LightDepthValue = input.LightViewPos.z / input.LightViewPos.w;
        
        float Bias = max(0.02 * (1.0 - dot(BumpNormal, normalize(Sun.Position - input.WorldPos.xyz))), 0.005);
    
        // PCF
        float2 TexelSize;
        ShadowDepth.GetDimensions(TexelSize.x, TexelSize.y);
        TexelSize = 1.0 / TexelSize;
        
        for (int x = -1; x <= 1; x++)
        {
            for (int y = -1; y <= 1; y++)
            {
                float PCFDepth = ShadowDepth.Sample(SampleTypeClamp, ProjectedTexCoord.xy + float2(x, y) * TexelSize).r;
                Shadow += LightDepthValue - Bias > PCFDepth ? 1.0f : 0.0f;
            }
        }
        Shadow /= 9.0f; // we sampled 9 samples
        
        // compare the depth of the shadow map and the light.
        if (LightDepthValue - Bias >= DepthValue)
        {
            return float4(AmbientLighting(Sun.Ambient, (float3) TextureColor), 1.0f);
        }
    } 
    else
    {
        Shadow = 0.0f;
    }
    
    // early abort if our alpha is too smal
    if (TextureColor.a < 0.01)
        discard;
    
    float3 FinalColor = (float3) TextureColor * CalculateDirectional(Sun, BumpNormal, V, input.TexCoord, SpecularMapValue, (float3) TextureColor, Shadow);
    
    for (int i = 0; i < LightsCount; i++)
    {
        FinalColor += (float3) TextureColor * CalculatePointLight(Lights[i], input.WorldPos.xyz, BumpNormal, V, input.TexCoord, SpecularMapValue, (float3) TextureColor, Shadow);
    }
    
    return float4(saturate(FinalColor), 1.0f);
}