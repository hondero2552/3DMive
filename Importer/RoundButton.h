#if defined(WINDOWS) || defined(_WINDOWS)
#pragma once
#endif
#ifndef ROUNDBUTTON_H
#define ROUNDBUTTON_H
#include "IUIElement.h"
#include "MathTypes.h"

using namespace omi;

class RoundButton : public UIButtonBase
{
private:
    float m_xcoord, m_ycoord, m_radius;
    // 
    D2D1_ELLIPSE m_ellipse;// the CIRCLE structure found in UIHelper.h will replace this. 
    Circle m_circle;
public:    
    RoundButton(void);
    RoundButton(const RoundButton& UIE) = delete;
    ~RoundButton(void);
    
    float2 GetCenterPoint(void) const { return float2(m_xcoord, m_ycoord); }
    float GetRadius(void) const { return m_radius; }    

    bool IsInsideElement(int xcoord, int ycoord);

    void Initialize(const float2& center, float radius, UIBUTTON button, const wstring& text, const D2D1_COLOR_F&);
    void Initialize(const float2& center, float radius, uint _outlineWidth, UIBUTTON button, const wstring& text, const Color& _outlineColor);
    void DrawMe();

    void RePositionElement(D2D1_POINT_2F& _newTopLeft);
    
};
#endif