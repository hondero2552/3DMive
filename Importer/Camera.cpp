#include "Cam.h"

Camera_FP::Camera_FP(uint width, uint height) :
    m_width(width), m_height(height),
    m_FieldOfView(0.0f),
    m_ratio(0.0f),
    m_iMovementSpeed(1),
    m_iZoomSpeed(1),
    m_pitch(MLConvertToRadians(-45.0f)),
    m_head(MLConvertToRadians(45.0f)),
    m_radius(150.0f),
    m_minimum_radius(1.0f)
{

}

void Camera_FP::VInitCamera(const float3& _pitch_head_radius, const float4& _target, const float& _FoVDegrees, const float& _ratio)
{
    // Save the state internally
    m_pitch = _pitch_head_radius.x;
    m_head  = _pitch_head_radius.y;
    m_radius= _pitch_head_radius.z;
    SetLookAtPoint(_target);
    m_FieldOfView     = _FoVDegrees;
    m_ratio           = _ratio;

    MLMatrixPerspectiveFoVLH(
        MLConvertToRadians(_FoVDegrees),
        _ratio, 
        NEAR_PLANE, 
        FAR_PLANE, 
        m_projection
        );

    // Code re-use
    MoveCamera(0, 0);
}


void Camera_FP::VOnResize(uint height, uint width)
{
    m_ratio = static_cast<float>(width)/height;

    MLMatrixPerspectiveFoVLH(MLConvertToRadians(m_FieldOfView), m_ratio, NEAR_PLANE, FAR_PLANE, m_projection);
    m_viewProjection = m_view * m_projection;
}

void Camera_FP::ZoomIn(void)
{
    m_radius    -= static_cast<float>(m_iZoomSpeed);
    m_radius    = clamp(m_radius, m_minimum_radius, FLT_MAX);
}

void Camera_FP::ZoomOut(void)
{
    m_radius += static_cast<float>(m_iZoomSpeed);
}

void Camera_FP::MoveCamera(int Vertically, int Horizontally)
{
    const float RADIANS_PER_PIXEL = MLConvertToRadians( m_iMovementSpeed*10.0f / (m_height) );
    m_pitch += RADIANS_PER_PIXEL*(-Vertically);
    m_head  += RADIANS_PER_PIXEL*Horizontally;

    if(m_pitch > CLOSE_TO_90)
        m_pitch = CLOSE_TO_90;
    else if(m_pitch < -CLOSE_TO_90)
        m_pitch = -CLOSE_TO_90;

    if(m_head > TWO_PI)
        m_head -= TWO_PI;
    else if(m_head < -TWO_PI)
        m_head -= TWO_PI;

    // Get the position from polar coordinates to cartesian coordinates
    m_Position.x = m_radius     * cos(m_pitch) * sin(m_head);
    m_Position.y = (-m_radius)  * sin(m_pitch);
    m_Position.z = m_radius     * cos(m_pitch) * cos(m_head);
    m_Position.w = 1.0f;

    // Translate the position to the local coordinate system of the look-at vector
    m_Position = m_Position * m_translation_WorldToLocalSpace;

    // Rebuild the world to camera matrix
    MLMatrixLookAtLH(
        m_Position, 
        m_Target, 
        float4(0.0f, 1.0f, 0.0f, 0.0f), 
        m_view
        );

    m_viewProjection = m_view * m_projection;
}

void Camera_FP::SetLookAtPoint(const float4& _target)
{
    // Save the current target position
    m_Target = _target;

    // Rebuild the translation matrix
    MLMatrixCreateTranslation(m_Target.x, m_Target.y, m_Target.z, m_translation_WorldToLocalSpace);
}

void Camera_FP::SetMinimumRadius(float _minRadius)
{
    m_minimum_radius = _minRadius;
}