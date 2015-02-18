#if defined(_WINDOWS) || defined(WINDOWS)
#pragma once
#else
#ifndef SCROLLBAR_H
#define SCROLLBAR_H
#endif
#endif

#include "IUIElement.h"
#include <sstream>

class ScrollBar : public IUIElement
{
    static ID2D1BitmapBrush*    m_pBitmapBrush_leftbutton;
    static ID2D1BitmapBrush*    m_pBitmapBrush_rightbutton;
    ID2D1SolidColorBrush*       m_pSolidColorBrush;    

    IDWriteTextLayout* m_pID2DTextLayoutTitle;
    IDWriteTextLayout* m_pID2DTextLayoutValue;

    D2D1_MATRIX_3X2_F m_transformLeftButton;
    D2D1_MATRIX_3X2_F m_transformRightButton;

    D2D1_RECT_F m_middleButton;
    D2D1_RECT_F m_leftButton;
    D2D1_RECT_F m_rightButton;
    D2D1_RECT_F m_filledRectangle;
    D2D1_RECT_F m_emptyRectangle;

    D2D1_COLOR_F m_buttonColor;
    D2D1_COLOR_F m_FilledRectColor;

    UIBUTTON m_type;

    bool m_bRightButtonClicked;
    bool m_bLeftButtonClicked;
    bool m_bEmpyRectangleClick;
    bool m_bhoveringLeftButton, m_bhoveringRigthButton, m_bhoveringEmptyRect;

    float m_currentValue;
    int m_minvalue;
    float m_movementinterval;
    int m_iframe;
    wstring m_text;
    wstring m_token;

    void MoveMiddleButton(void);

public:
    ScrollBar(void);
    ~ScrollBar(void);
    bool Initialize(float left, float top, UIBUTTON type, const wstring& title, int currentvalue = 1, int minvalue = 1, wchar_t* token = L"%");
    
    // Inherited methods
    inline bool IsInsideElement(int x, int y);
    inline UIBUTTON GetButton(void) const { return m_type; }
    float2 GetCenterPoint(void) const;
    float GetRadius(void) const;

    inline void UnclickedMe(void) { MouseHoveringOnMe(true); }
    inline bool isClicked(void) { return (m_bLeftButtonClicked || m_bRightButtonClicked); }

    void MouseHoveringOnMe(bool yesORno);
    void MouseButtonUp(void);
    void MouseClickedMe(void);
    void DrawMe(void);
    void RestoreToPreviousStatus(void) {  }

    Screen* GetChildScreen(void) { return nullptr; }
    void SetChildScreen(Screen* pScreen) { }

    D2D1_MATRIX_3X2_F& GetTransform(void) { return D2D1::Matrix3x2F::Translation(D2D1::SizeF(0.0f, 0.0f));}
    void RePositionElement(D2D1_POINT_2F& newTopLeft) {  }
    int GetCurrentValue(void) const { return static_cast<int>(m_middleButton.left - m_leftButton.right); }
    const wstring& VGetElementText(void) const { return m_text; }

    void static SetLeftButtonBrush(ID2D1BitmapBrush* brush)
    {
        m_pBitmapBrush_leftbutton = brush;
    }
    void static SetRightButtonBrush(ID2D1BitmapBrush* brush)
    {
        m_pBitmapBrush_rightbutton = brush;
    }
    void static ReleaseStaticMemory(void)
    {
        SAFE_RELEASE_COM( m_pBitmapBrush_leftbutton );
        SAFE_RELEASE_COM( m_pBitmapBrush_rightbutton );
    }    
};