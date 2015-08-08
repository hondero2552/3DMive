#include "Screen.h"
#include "User Interface.h"

Screen::~Screen(void)
{
    DumpUIElements();

    // Delete the array of quadrants
    SAFE_DELETE_ARRAY( m_pQuadrants );
    SAFE_RELEASE_COM( m_blackTransparentBrush );    
}

void Screen::DrawElements(void)
{
    // Draw this screen and then draw the screen buttons
    UIButtonBase::GetContext()->FillRoundedRectangle(m_roundedRect, m_blackTransparentBrush);
    for_each(m_UIElementsList.begin(), m_UIElementsList.end(), [&] (IUIElement* _ptr)
    {
        _ptr->DrawMe();
    });
}

void Screen::AddToQuadrant(IUIElement* pUIElement)
{
    // Get the center of the circle button of the UI-Element and the radius
    const float2 center = pUIElement->GetCenterPoint();
    const float radius  = pUIElement->GetRadius();

    // Treat the UI Element as if it was a square...calculate each corner of the square
    const float2 corners [] = 
    {
        float2( (center.u - radius), center.v - radius),    // Top Left
        float2( (center.u + radius), center.v - radius),    // Top Right
        float2( (center.u - radius), center.v + radius),    // Bottom Left
        float2( (center.u + radius), center.v + radius)     // Bottom Right
    };

    // For each corner of the square button see if any touches any quadrant, and then added the UIElement to that quadrant
    // Keep in mind that a button can be in multiple quadrants at the same time, but the same UIElement won't be added twice
    for(uint i = 0; i < 4; ++i)
    {
        const float width = GetWidth();
        const float height= GetHeight();

        // Left half
        if(corners[i].u < (m_Rectangle.mleft + (width/2.0f)))
        {
            // First quadrant
            if(corners[i].v < (m_Rectangle.mtop + (height/2.0f)))
                m_pQuadrants[QUADRANT::ONE].AddUIElement(pUIElement);
            
            // Third Quadrant
            else
                m_pQuadrants[QUADRANT::THREE].AddUIElement(pUIElement);            
        }        
        // Right half
        else
        {
            // Second Quadrant
            if(corners[i].v < (m_Rectangle.mtop + (height/2.0f)))
                m_pQuadrants[QUADRANT::TWO].AddUIElement(pUIElement);
            
            // Fourth Quadrant
            else
                m_pQuadrants[QUADRANT::FOUR].AddUIElement(pUIElement);
        }
    }
}

void Screen::AddUIElement(IUIElement* ptrUIElement)
{
    // Here is where we will be adding the code to add the 2d buttons/geometry to the renderer so it can track vertices, textures, and indices information
    m_UIElementsList.push_front(ptrUIElement);    
    AddToQuadrant(ptrUIElement);
}

bool Screen::InitializeScreen(float left, float right, float top, float bottom, float transparency, float radius, SCREEN screentType)
{
    m_blackTransparentBrush = nullptr;
    m_pQuadrants = new Quadrant[4];                                         // Add Try/Catch exception block of code
    if(!m_pQuadrants)
        return false;

    m_Rectangle.mleft       = m_screenAreaRect.left     = left;
    m_Rectangle.mright      = m_screenAreaRect.right    = right;
    m_Rectangle.mtop        = m_screenAreaRect.top      = top;
    m_Rectangle.mbottom     = m_screenAreaRect.bottom   = bottom;
    m_roundedRect.rect      = m_screenAreaRect;
    m_roundedRect.radiusX   = m_roundedRect.radiusY     = radius;
    
    m_transparency  = transparency;
    m_type          = screentType;
    
    D2D_COLOR_F color;
    //{0.4f, 0.6f, 0.75f, 1.0f};
    
    color.r = 0.2f; color.g = 0.2f; color.b = 0.2f;
    color.a = transparency;
    
    UIButtonBase::GetContext()->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black, m_transparency) , &m_blackTransparentBrush);
    
    return true;
}

UIBUTTON Screen::MouseClickedAt(int x, int y)
{
    UIBUTTON button = UIBUTTON::NO_BUTTON;

    if(m_pCurrentUIElement != nullptr)
    {
        if(m_pCurrentUIElement->IsInsideElement(x, y))
        {
            m_pCurrentUIElement->MouseClickedMe();
            button = m_pCurrentUIElement->GetButton();
        }
    }
    return button;
}

UIBUTTON Screen::MouseButtonUpAt(int x, int y)
{
    assert(m_pCurrentUIElement != nullptr);
    UIBUTTON button = UIBUTTON::NO_BUTTON;
    m_pCurrentUIElement->UnclickedMe();// Call this in case it was clicked but not all the way through
    
    // First check if a button in the current screen was clicked

    if(m_pCurrentUIElement->IsInsideElement(x, y))
    {
        // IF a button belonging to this screen was clicked then check if this button has a child screen
        // and set it as the current screen of the user interface
        m_pCurrentUIElement->MouseButtonUp();   // Every UIElement has implements this function, this is where they do their specific thing.
        button = m_pCurrentUIElement->GetButton();
    }
    else
    {
        m_pCurrentUIElement->MouseHoveringOnMe(false);
        m_pCurrentUIElement->RestoreToPreviousStatus();
        m_pCurrentUIElement = nullptr;
        MouseHoveringAt(x, y);
    }
    return button;
}

UIBUTTON Screen::MouseHoveringAt(int x, int y)
{
    UIBUTTON button = UIBUTTON::NO_BUTTON;
    
    // see if it hovering on the current UI Element. THIS IS USED TO RESET ANY BUTTONS AFTER A DIALOG MENU
    if(m_pCurrentUIElement != nullptr)
    {
        if(m_pCurrentUIElement->IsInsideElement(x, y))
        {
            button = m_pCurrentUIElement->GetButton();
            return button;
        }
        else
        {
            m_pCurrentUIElement->MouseHoveringOnMe(false);
            m_pCurrentUIElement = nullptr;
        }    
    }

    // Test if it is hovering inside this  screen (which is the current UI screen), and if it's not then return
    if( x < m_Rectangle.mleft || x > m_Rectangle.mright || y < m_Rectangle.mtop || y > m_Rectangle.mbottom)
    {
        button = UIBUTTON::OUTSIDE_SCREEN;
        return button;
    }

    // Calculate the Quadrant
    const float half_width  = m_Rectangle.mleft + ( (m_Rectangle.mright - m_Rectangle.mleft) / 2.0f );
    const float half_height = m_Rectangle.mtop  + ( (m_Rectangle.mbottom - m_Rectangle.mtop) / 2.0f );
    
    QUADRANT Q;
    if(x < half_width)
    {
        if(y < half_height)
            Q = QUADRANT::ONE;
        else
            Q = QUADRANT::THREE;
    }
    else
    {
        if(y < half_height)
            Q = QUADRANT::TWO;
        else
            Q = QUADRANT::FOUR;
    }

    // Check every UIElement in the quadrant and send notifications to them so they can perform their unique task when the mouse is hovering over them
    for(auto iter = m_pQuadrants[Q].GetList().begin(); iter != m_pQuadrants[Q].GetList().end(); ++iter)
    {
        IUIElement* _ptr = *iter;
        if(_ptr->IsInsideElement(x, y))
        {
            m_pCurrentUIElement = _ptr;
            m_pCurrentUIElement->MouseHoveringOnMe(true);
            button = m_pCurrentUIElement->GetButton();
            break;
        }
    }
    return button;
}

void Screen::DumpUIElements(void)
{
    // Delete all the UIElements of this screen
    for_each(m_UIElementsList.begin(), m_UIElementsList.end(), [&] (IUIElement* _ptr)
    {
        SAFE_DELETE(_ptr);
    });
    m_UIElementsList.clear();
    
    //
    for(size_t i = 0; i < 4; ++i)
    {
        m_pQuadrants[i].GetList().clear();
    }
    
    m_pCurrentUIElement = nullptr;
}