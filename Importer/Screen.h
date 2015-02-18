#pragma once
#include "Debug_ALL.h"
#include "Quadrant.h"
#include <forward_list>
#include <algorithm>

using std::for_each;
using std::forward_list;

enum SCREEN { MAIN_SCREEN, CAMERA_SETTINGS, MESH_SETTINGS, IMPORTEXPORTMESH, ENVIRONMENTOPTIONS, MATERIALEDITING_SCREEN, MATERIALS_TERM_SCREEN, MATERIAL_CHANNEL_SCREEN };

typedef forward_list<IUIElement*> IUIElements;

class Screen
{
    float m_transparency;
    bool m_bScreenHidden;
    bool m_bMouseFocus;
    
    Screen* m_pParentScreen;    
    D2D1_ROUNDED_RECT m_roundedRect;
    SCREEN m_type;
    IUIElements m_UIElementsList;
    Quadrant* m_pQuadrants;

    IUIElement* m_pCurrentUIElement;
    
    D2D1_RECT_F m_screenAreaRect;
    ID2D1SolidColorBrush* m_blackTransparentBrush;
    
    omi::Rectangle m_Rectangle;
    
    // Adds a UIElemnt to the appropriate quadrant by calculating its relative position in the UI window
    void AddToQuadrant(IUIElement* UIElement);

public:

    Screen(void) : m_pQuadrants(nullptr), m_pCurrentUIElement(nullptr), m_bScreenHidden(false), m_bMouseFocus(false){ }
    ~Screen(void);
    Screen* GetScreenPtr(void) { return this; }    
    SCREEN GetScreenType(void) { return m_type; }
    
    float GetHeight(void) const { return (m_Rectangle.mbottom - m_Rectangle.mtop); }
    float GetWidth(void) const  { return (m_Rectangle.mright - m_Rectangle.mleft); }
    
    omi::Rectangle GetScreenRect(void) const { return m_Rectangle; }
    
    bool InitializeScreen(float left, float right, float top, float bottom, float transparency, float radius, SCREEN screenType);
    IUIElement* GetCurrentUIElement(void)       { return m_pCurrentUIElement; }
    void SetParent(const Screen* pParentScreen) { m_pParentScreen = const_cast<Screen*>(pParentScreen); }
    
    const Screen* GetParentScreen(void) const   { return m_pParentScreen; }
    Screen* GetParentScreen(void)               { return m_pParentScreen; }
    //    
    void AddUIElement(IUIElement* ptrUIEle);

    // User Interaction
    UIBUTTON MouseClicked(int x, int y);
    UIBUTTON MouseButtonUp(int x, int y);
    UIBUTTON MouseHoveringAt(int x, int y);

    void DumpUIElements(void);
    
    inline void ShowScreen(void)    { m_bScreenHidden = false; }
    inline void HideScreen(void)    { m_bScreenHidden = true;  }
    inline void CaptureMouse(void)  { m_bMouseFocus = true; }
    inline void ReleaseMouse(void)  { m_bMouseFocus = false; }

    inline bool IsHidden(void)      { return m_bScreenHidden; }
    inline bool HasMouseFocus(void) { return m_bMouseFocus; }

    void DrawElements(void);
};