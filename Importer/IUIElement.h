#pragma once
#include "DebugDX.h"
#include <dwrite_1.h>
#include "UIHelper.h"
#include "UIElement Types.h"
#include "math_funcs.h"

using namespace omi;

const float FONT_SIZE               = 12.0f;
static const wchar_t* wzFONT_TYPE   = L"Consolas";
class Screen;
enum class BUTTON_TYPE {UNDEFINED, ROUND_BUTTON, SQUARE_BUTTON, SCROLL_BAR};
class IUIElement
{
    IUIElement(const IUIElement& UIE) { };
public:
    IUIElement(void) { }
    virtual ~IUIElement(void) { }

    virtual inline bool IsInsideElement(int x_coordinate, int y_coordinate) = 0;
    virtual inline UIBUTTON GetButton(void) const = 0;
    virtual float2 GetCenterPoint(void) const = 0;
    virtual float GetRadius(void) const = 0;
    
    virtual inline void UnclickedMe(void) = 0;
    virtual inline bool isClicked(void) = 0;

    virtual void MouseHoveringOnMe(bool yesORno) = 0;
    virtual void MouseButtonUp(void) = 0;
    virtual void MouseClickedMe(void) = 0;
    virtual void DrawMe(void) = 0;
    virtual void RestoreToPreviousStatus(void) = 0;
    virtual const wstring& VGetElementText(void) const = 0;

    virtual BUTTON_TYPE GetButtonType(void) const = 0;

    virtual Screen* GetChildScreen(void) = 0;
    virtual void SetChildScreen(Screen* pScreen) = 0;

    virtual D2D1_MATRIX_3X2_F& GetTransform(void) = 0;
    virtual void RePositionElement(D2D1_POINT_2F& newTopLeft) = 0;
protected:    
    static ID2D1DeviceContext* m_pContext;
    static IDWriteFactory1* m_pWriteFactory;
};

class UIButtonBase : public IUIElement
{    
    UIButtonBase(const UIButtonBase& B) { } // Innaccessible copy constructor
protected:
    bool m_bHoveringMe;
    bool m_bImClicked;
    bool m_bImPermanentlyActive;
    bool m_bDefaultStatus;
    float m_OutlineBrushSize;
    bool m_bDrawText;
    Screen* m_pChildScreen;
    wstring m_wzText;
    UIBUTTON m_id;
    BUTTON_TYPE m_type;
    BUTTON_STATUS m_previousStatus;    
    D2D1_MATRIX_3X2_F m_Trasnform;
    
    // Effects
    HOVERING_EFFECT     m_effectHovering;
    CLICKED_EFFECT      m_effectClicked;
    ACTIVE_EFFECT       m_effectActive;
    DEFAULT_EFFECT      m_effectDefault;

    Color new_m_BrushColor_active;
    Color new_m_BrushColor_buttontext;
    Color new_m_BrushColor_clicked;
    Color new_m_BrushColor_default;
    Color new_m_BrushColor_hovering;

    D2D1_COLOR_F m_BrushColor_active;
    D2D1_COLOR_F m_BrushColor_buttontext;
    D2D1_COLOR_F m_BrushColor_clicked;
    D2D1_COLOR_F m_BrushColor_default;
    D2D1_COLOR_F m_BrushColor_hovering;

    ID2D1BitmapBrush* m_pBitmapBrush_ClickedStatus;
    ID2D1BitmapBrush* m_pBitmapBrush_Current;
    ID2D1BitmapBrush* m_pBitmapBrush_DefaultStatus;
    ID2D1BitmapBrush* m_pBitmapBrush_MouseHoveringStatus;
    ID2D1BitmapBrush* m_pBitmapBrush_PermanentlyActiveStatus;
    
    ID2D1SolidColorBrush* m_SolidColorBrush_UseAll;

    IDWriteTextLayout* m_pID2DTextLayout;

    void ResetTrasform(void);

public:
    
    // Default Constructor
    UIButtonBase(void) : m_bHoveringMe(false), m_bImClicked(false), m_bImPermanentlyActive(false), m_bDefaultStatus(true), m_bDrawText(true),
        m_OutlineBrushSize(0.0f),
        m_pBitmapBrush_Current(nullptr),
        m_effectActive(ACTIVE_EFFECT::NOPERMANENTSTATUS), 
        m_effectClicked(NOCLICKEDEFFECT), 
        m_effectHovering(NOHOVERINGEFFECT),
        m_effectDefault(NODEFAULTEFFECT),

        m_SolidColorBrush_UseAll(nullptr),
        m_pBitmapBrush_DefaultStatus(nullptr),
        m_pBitmapBrush_MouseHoveringStatus(nullptr),
        m_pBitmapBrush_ClickedStatus(nullptr),
        m_pBitmapBrush_PermanentlyActiveStatus(nullptr),
        m_pID2DTextLayout(nullptr),
        m_pChildScreen(nullptr),
        m_type(BUTTON_TYPE::UNDEFINED)
    {  }

    // Destructor
    ~UIButtonBase(void) 
    {
        SAFE_RELEASE_COM( m_pID2DTextLayout );

        SAFE_RELEASE_COM( m_SolidColorBrush_UseAll );

        SAFE_RELEASE_COM( m_pBitmapBrush_DefaultStatus );
        SAFE_RELEASE_COM( m_pBitmapBrush_MouseHoveringStatus );
        SAFE_RELEASE_COM( m_pBitmapBrush_ClickedStatus );
        SAFE_RELEASE_COM( m_pBitmapBrush_PermanentlyActiveStatus );
    }
    
    void UnclickedMe(void) { m_bImClicked = false; }
    inline bool isClicked(void) { return m_bImClicked; }

    static void SetContext(ID2D1DeviceContext* context);
    static void SetWriteFactory(IDWriteFactory1* pFactoy);

    static ID2D1DeviceContext* GetContext(void) { return m_pContext; }    
    void SetOutlineBrushSize(const float& size) { m_OutlineBrushSize = size; }
    float GetOutlineBrushSize(void) const { return m_OutlineBrushSize; }
    
    void RestoreToPreviousStatus(void);
    void MouseHoveringOnMe(bool yesORno);
    void MouseClickedMe(void);
    void MouseButtonUp(void);
    
    void SetPermanentlyActiveEffect(ACTIVE_EFFECT effect, const D2D1_COLOR_F& _optional_Brush);
    void SetHoveringEffect(HOVERING_EFFECT effect, const D2D1_COLOR_F& _optional_Brush);
    void SetClickedEffect(CLICKED_EFFECT effect, const D2D1_COLOR_F& _optional_Brush);
    void SetDefaultEffect(DEFAULT_EFFECT effect, const D2D1_COLOR_F& _optional_Brush);

    void SetDefaultStatusBitmap(ID2D1BitmapBrush* Bitmap);
    void SetMosueHoveringStatusBitmap(ID2D1BitmapBrush* Bitmap);
    void SetClickedStatusBitmap(ID2D1BitmapBrush* Bitmap);
    void SetPermanentActiveStatusBitmap(ID2D1BitmapBrush* Bitmap);

    BUTTON_TYPE GetButtonType(void) const { return m_type; }
    UIBUTTON GetButton(void) const { return m_id; }
    
    D2D1_MATRIX_3X2_F& GetTransform(void) { return m_Trasnform; }

    Screen* GetChildScreen(void) { return m_pChildScreen; }
    void SetChildScreen(Screen* pScreen) { m_pChildScreen = pScreen; }

    const wstring& VGetElementText(void) const { return m_wzText; }

private:
    void SaveCurrentStatus(void);
};