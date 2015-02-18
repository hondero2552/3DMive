
cbuffer Lights : register(b3)
{
    float4 gColor; // 12
}
float4 main(float4 PosH : SV_POSITION) : SV_TARGET
{
    return gColor;
}