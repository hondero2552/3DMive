#include "ConstBuffer.hlsl"
#include "Samplers.hlsl"
#include "Textures.hlsl"

struct VS_OUT
{
    float4 PosH     : SV_POSITION;
    float3 PosW     : POSITION;
    float3 Norm     : NORMAL;
    float2 Tex      : TEXCOORD;
    float4 Tangent  : TANGENT;
};

float4 main(VS_OUT _in) : SV_TARGET
{
    // Normal Map    
    const float3 normalmap = 2.0f * (gTextures512.Sample(gLinearSam, float3(_in.Tex, gNormalMapIndex)).rgb) - 1.0f;
    const float3 N = normalize(_in.Norm);
    const float3 T = normalize(_in.Tangent.xyz - dot(_in.Tangent.xyz, N)* N);
    const float3 B = cross(N, T) * _in.Tangent.w;

    const float3x3 TBN = float3x3(T, B, N);
    
    // Light vector, eye-position, and surface normal, all in tangent space
    const float3 lightv = mul((-gDirLight.Direction).xyz, TBN);
    const float3 normal = mul(normalmap, TBN);
    const float3 eyePos = mul(normalize((gEyePos.xyz - _in.PosW)).xyz, TBN);
    
    // Lighting Calculations    
    float3 hvector = normalize(lightv + eyePos);
    
    float4 ambient  = gDirLight.Ambient * gMaterial.Ambient;
    float4 diffuse  = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 texColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
    
    [branch]
    if(gUseTextures)
        texColor = gTextures512.Sample(gLinearSam, float3(_in.Tex, gDiffuseMapIndex));
    
    // diffuse factor
    float diffuseFactor = dot(normal, lightv);
    [branch]
    if(diffuseFactor > 0.0f)
    {
        diffuse = diffuseFactor * gDirLight.Diffuse * gMaterial.Diffuse;
        
        // Specular facttor & color
        float HdotN = saturate(dot(hvector, normal));
        specular = gDirLight.Specular * pow(HdotN, gMaterial.Specular.w);
    }
    
    // Modulate with late add
    float4 litcolor = float4( ((texColor * (ambient + diffuse)) + specular) );
    if(gUseTextures)
        litcolor.a = texColor.a * gMaterial.Diffuse.a;
    else
        litcolor.a = gMaterial.Diffuse.a;

    return litcolor;
    //return texColor;
}