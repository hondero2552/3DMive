#include "ConstBuffer.hlsl"

float4 main(float3 PosL : POSITION) : SV_POSITION
{
    return mul(float4(PosL, 1.0), gViewProjTransf);
}