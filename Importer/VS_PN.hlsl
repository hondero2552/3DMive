#include "ConstBuffer.hlsl"
struct VS_IN
{
    float3 PosL : POSITION;
    float2 Tex  : TEXCOORD;
    float3 Norm : NORMAL;    
};

struct VS_OUT
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION;
    float3 Norm : NORMAL;
};

VS_OUT main(VS_IN _in)
{
    VS_OUT _out;
    _out.PosH = mul(float4(_in.PosL, 1.0), gViewProjTransf);
    _out.PosW = _in.PosL;
    _out.Norm = _in.Norm;
    _out.Tex  = _in.Tex;
    return _out;
}