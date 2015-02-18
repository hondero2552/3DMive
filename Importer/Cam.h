#pragma once
#include "math_funcs.h"
using namespace omi;
const float NEAR_PLANE = 0.01f;
const float FAR_PLANE = 10000.0f;
const float CLOSE_TO_90 = MLConvertToRadians(89.0);

class Camera_FP
{
private:
  float m_FieldOfView;      // Field of view angle in Degrees
  float m_ratio;            // Height/Width
  
  float m_minimum_radius;

  int m_iMovementSpeed;
  int m_iZoomSpeed;
  
  float4 m_Position;
  float4 m_Target;

  Matrix m_translation_WorldToLocalSpace;

  // Camera movement stuff
  uint m_width;
  uint m_height;
  float m_pitch;
  float m_head;
  float m_radius;

  Matrix m_view;
  Matrix m_projection;
  Matrix m_viewProjection;
  // Hidden copy constructor
  Camera_FP(Camera_FP& cam) { }
public:
  Camera_FP(uint width, uint height);
  ~Camera_FP(void) { }

  inline const Matrix& VGetViewProjectionMatrix(void) { return m_viewProjection; }
  inline const Matrix& VGetProjectionMatrix(void) const { return m_projection; }  
  inline const Matrix& VGetViewMatrix(void) const { return m_view; };
  inline const float& VGetFoV(void) const { return m_FieldOfView; }
  inline const float4& VGetEyePosition(void) const { return m_Position; }
  
  void VInitCamera(const float3& _pitch_head_radius, const float4& _target, const float& _FoVDegrees, const float& _ratio);  

  void VOnResize(uint height, uint width);

  void SetMovementSpeed(int value) { m_iMovementSpeed = value; }
  void SetZoomSpeed(int value) { m_iZoomSpeed = value; }
  int GetZoomSpeed(void) { return m_iZoomSpeed; }
  
  void SetRadius(float _radius) { m_radius = _radius; m_radius = clamp(m_radius, m_minimum_radius, FLT_MAX); }

  void SetMinimumRadius(float _minRadius);
  void ZoomIn(void);
  void ZoomOut(void);
  void MoveCamera(int vertically, int horizontally);
  float2 GetHeadandPitch(void) const { return float2(m_head, m_pitch); }

  void SetLookAtPoint(const float4& _target);
};