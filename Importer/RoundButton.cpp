#include "RoundButton.h"

RoundButton::RoundButton(void) :
    m_xcoord(0.0f), m_ycoord(0.0f), m_radius(0.0f)
{

}

RoundButton::~RoundButton(void)
{

}

bool RoundButton::IsInsideElement(int xcoord, int ycoord) // NEED TO COMMENT
{
    bool lbIsInsideButton   = false;
    const float xdistance   = fabs(xcoord - m_xcoord);
    const float ydistance   = fabs(ycoord - m_ycoord);

    if( ( xdistance < m_radius) &&
        ( ydistance < m_radius) )
    {
        const float distance = sqrtf( (xdistance*xdistance) + (ydistance * ydistance) );

        if( distance < (m_radius + 1.0f) )
        {
            lbIsInsideButton = true;
        }
    }
    return lbIsInsideButton;
}

void RoundButton::Initialize(const float2& center, float radius, UIBUTTON button, const wstring& text, const D2D1_COLOR_F& textcolor)
{    
    assert(UIButtonBase::m_pContext != nullptr);
    assert(UIButtonBase::m_pWriteFactory != nullptr);

    m_type = button;
    m_xcoord = center.u;
    m_ycoord = center.v;
    m_radius = radius;
    m_ellipse.point.x = m_xcoord;
    m_ellipse.point.y = m_ycoord;
    m_ellipse.radiusX = m_ellipse.radiusY = m_radius;
    m_BrushColor_buttontext = textcolor;

    if(text.size() > 0)
    {
        m_wzText = text;
        // Create the text formatting
        const float box_witdh = text.length() * FONT_SIZE;
        ComPtr<IDWriteTextFormat> pTextFormat;
        HRESULT hr = S_OK;
        hr = UIButtonBase::m_pWriteFactory->CreateTextFormat(wzFONT_TYPE, 
            nullptr, DWRITE_FONT_WEIGHT_NORMAL, 
            DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 
            FONT_SIZE, L"", &pTextFormat);

        hr = UIButtonBase::m_pWriteFactory->CreateTextLayout(text.c_str(), text.length(), pTextFormat.Get(), box_witdh, FONT_SIZE, &m_pID2DTextLayout);
    }
    else
    {
        m_bDrawText = false;
    }

    // Create Transformation matrix that will be applied to every brush so the bitmap is centered
    const float2 newOrigin( (center.u - m_radius), (center.v - m_radius));
    m_Trasnform = D2D1::Matrix3x2F::Translation(D2D1::SizeF(newOrigin.u, newOrigin.v));
    UIButtonBase::m_pContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black, 1.0f), &m_SolidColorBrush_UseAll);
    m_SolidColorBrush_UseAll->SetTransform( m_Trasnform );
}

void RoundButton::DrawMe(void)
{
    // Check if the mouse is hovering on the button and the button is not pressed
    if(m_bHoveringMe && m_effectHovering != HOVERING_EFFECT::NOHOVERINGEFFECT && !m_bImClicked && !m_bImPermanentlyActive)
    {
        m_SolidColorBrush_UseAll->SetColor( m_BrushColor_hovering );

        if(m_effectHovering == HOVERING_EFFECT::DRAWOUTLINE_H)        
            UIButtonBase::m_pContext->DrawEllipse(m_ellipse, m_SolidColorBrush_UseAll, m_OutlineBrushSize);
        
        else if(m_effectHovering == HOVERING_EFFECT::DRAWMASK_H)        
            UIButtonBase::m_pContext->FillEllipse(m_ellipse, m_SolidColorBrush_UseAll);        
    }
    // if the button is pressed
    else if(m_bImClicked && m_effectClicked == CLICKED_EFFECT::DRAWMASK_C)
    {
        m_SolidColorBrush_UseAll->SetColor( m_BrushColor_clicked );
        UIButtonBase::m_pContext->FillEllipse(m_ellipse, m_SolidColorBrush_UseAll);
    }
    // do this if the button was completely clicked (mouse clicked AND release on the button) and it supports Permanently-Active status
    else if(m_bImPermanentlyActive && m_effectActive == ACTIVE_EFFECT::DRAWMASK_A)
    {
        m_SolidColorBrush_UseAll->SetColor( m_BrushColor_active );
        UIButtonBase::m_pContext->FillEllipse(m_ellipse, m_SolidColorBrush_UseAll);
    }
    // Do this for normal drawing, i.e. default-status
    else if(m_bDefaultStatus)
    {
        m_SolidColorBrush_UseAll->SetColor( m_BrushColor_default );
        
        if(m_effectDefault == DRAWOUTLINE_D)
            UIButtonBase::m_pContext->DrawEllipse(m_ellipse, m_SolidColorBrush_UseAll, m_OutlineBrushSize);
        
        else if (m_effectDefault == DRAWMASK_D)
            UIButtonBase::m_pContext->FillEllipse(m_ellipse, m_SolidColorBrush_UseAll);        
    }

    UIButtonBase::m_pContext->FillEllipse(m_ellipse, m_pBitmapBrush_Current);

    // Draw the buttons text
    if(m_bDrawText)
    {
        DWRITE_TEXT_METRICS metrics;
        m_pID2DTextLayout->GetMetrics(&metrics);

        D2D1_POINT_2F point;
        point.x = m_xcoord - (metrics.width/2.0f);
        point.y = m_ycoord + 34.0f;    
        m_SolidColorBrush_UseAll->SetColor(m_BrushColor_buttontext);
        UIButtonBase::m_pContext->DrawTextLayout(point, m_pID2DTextLayout, m_SolidColorBrush_UseAll);
    }
}

void RoundButton::RePositionElement(D2D1_POINT_2F& newTopLeft)
{
    m_xcoord = m_radius + newTopLeft.x;
    m_ycoord = m_radius + newTopLeft.y;

    m_ellipse.point.x   = m_xcoord;
    m_ellipse.point.y   = m_ycoord;
    m_Trasnform         = D2D1::Matrix3x2F::Translation(D2D1::SizeF(newTopLeft.x, newTopLeft.y));

    ResetTrasform();
}