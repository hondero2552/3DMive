#ifndef CONSTANT_BUFFER
#define CONSTANT_BUFFER
#include "LightSources.hlsl"

cbuffer PerFrame : register(b0)
{
    float4x4 gViewProjTransf;   // 64
    float4x4 gViewTransf;       // 64
    float4   gEyePos;           // 16
}

cbuffer Lights : register(b1)
{
    DirectionalLight gDirLight; // 64
}

cbuffer PerObject : register(b2)
{
    Material gMaterial;     // 64
    uint gDiffuseMapIndex;  // 4
    uint gNormalMapIndex;   // 4
    uint gSpecMapIndex;     // 4
    uint gUseTextures;      // 4
}
#endif