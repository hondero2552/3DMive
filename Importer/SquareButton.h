#if defined(WINDOWS) || defined(_WINDOWS)
#pragma once
#endif
#ifndef SQUARE_BUTTON_H
#define SQUARE_BUTTON_H
#include "IUIElement.h"
#include "MathTypes.h"
#include "UIElement Types.h"
using namespace omi;

class SquareButton : public UIButtonBase
{
private:
    float m_radius, m_width, m_height;
    // 
    D2D1_RECT_F m_SquareRect;    
    D2D1_ROUNDED_RECT m_RoundedRect;
public:
    SquareButton(void) { m_type = BUTTON_TYPE::SQUARE_BUTTON; }
    ~SquareButton(void) { }
    
    float2 GetCenterPoint(void) const { return float2(m_SquareRect.left + m_radius, m_SquareRect.top + m_radius); }
    float GetRadius(void) const { return m_radius; }    

    bool IsInsideElement(int xcoord, int ycoord);
    void Initialize(const float left, float top, float width, float height, UIBUTTON button, const wstring& text, const D2D1_COLOR_F& textColor);

    void DrawMe();
    
    void RePositionElement(D2D1_POINT_2F& newTopLeft);

private:
    SquareButton(const SquareButton& UIE) { }
};
#endif