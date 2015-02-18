#include "ConstBuffer.hlsl"
#include "Samplers.hlsl"
#include "Textures.hlsl"
struct VS_OUT
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION;
    float3 Norm : NORMAL;
    float2 Tex  : TEXCOORD;
};

float4 main(VS_OUT _in) : SV_TARGET
{
    const float3 surface_normal = normalize(_in.Norm);
    const float3 light_vec      = -gDirLight.Direction.xyz;
    const float3 eyePos         = normalize(gEyePos.xyz - _in.PosW);
    const float3 hvector        = normalize(light_vec + eyePos);
    
    float4 ambient  = gDirLight.Ambient * gMaterial.Ambient;
    float4 diffuse  = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 texColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
    
    [branch]
    if(gUseTextures)
        texColor = gTextures512.Sample(gLinearSam, float3(_in.Tex, gDiffuseMapIndex));
    
    // diffuse factor
    float diffuseFactor = dot(surface_normal, light_vec);
    [branch]
    if(diffuseFactor > 0.0f)
    {
        diffuse = diffuseFactor * gDirLight.Diffuse * gMaterial.Diffuse;
        // Specular facttor & color
        float HdotN = saturate(dot(hvector, surface_normal));
        specular = pow(HdotN, gMaterial.Specular.w) * gDirLight.Specular;
    }
    
    // Modulate with late add
    return (texColor * (ambient + diffuse)) + specular;
}