#pragma once
#include "MathTypes.h"
namespace omi
{
  struct DirectionalLight
  {
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;
	float4 Direction;
  };
  
  struct PointLight
  {
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;
	float3 Position;
	float  Range;
	float3 Attenuation;
	float padding;
  };
}