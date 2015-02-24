#include "SquareButton.h"

void SquareButton::Initialize(const float left, float top, float width, float height, UIBUTTON button, const wstring& text, const D2D1_COLOR_F& textColor)
{
    assert(UIButtonBase::m_pContext != nullptr);
    assert(UIButtonBase::m_pWriteFactory != nullptr);

    m_id      = button;
    m_width     = width;
    m_height    = height;
    m_radius    = m_width/2.0f;

    m_SquareRect.left   = left;
    m_SquareRect.top    = top;
    m_SquareRect.right  = left + m_width;
    m_SquareRect.bottom = top + m_height;

    m_BrushColor_buttontext = textColor;
    m_RoundedRect.rect = m_SquareRect;
    m_RoundedRect.radiusX = m_RoundedRect.radiusY = 5.0f;

    if(text.size() > 0)
    {
        // Create the text formatting
        const float box_witdh = 64.0f;
        ComPtr<IDWriteTextFormat> pTextFormat;
        HRESULT hr = UIButtonBase::m_pWriteFactory->CreateTextFormat(wzFONT_TYPE, nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, FONT_SIZE, L"", &pTextFormat);

        pTextFormat->SetLineSpacing(DWRITE_LINE_SPACING_METHOD_UNIFORM, FONT_SIZE, 10.0f);

        hr = UIButtonBase::m_pWriteFactory->CreateTextLayout(text.c_str(), text.length(), pTextFormat.Get(), box_witdh, FONT_SIZE, &m_pID2DTextLayout);
    }
    else
    {
        m_bDrawText = false;
    }

    // Create Transformation matrix that will be applied to every brush so the bitmap is centered
    m_Trasnform = D2D1::Matrix3x2F::Translation(D2D1::SizeF(m_SquareRect.left, m_SquareRect.top));

    UIButtonBase::m_pContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black, 1.0f), &m_SolidColorBrush_UseAll);

    m_SolidColorBrush_UseAll->SetTransform( m_Trasnform );
}

bool SquareButton::IsInsideElement(int xcoord, int ycoord) // NEED TO COMMENT
{
    bool lbIsInsideButton   = false;

    if((xcoord < m_SquareRect.right + 1.0f) && (xcoord > m_SquareRect.left - 1.0f) &&
        (ycoord < m_SquareRect.bottom+ 1.0f) && (ycoord > m_SquareRect.top - 1.0f))
    {
        lbIsInsideButton = true;
    }

    return lbIsInsideButton;
}

void SquareButton::DrawMe()
{
    // Check if the mouse is hovering on the button and the button is not pressed
    if(m_bHoveringMe && m_effectHovering != HOVERING_EFFECT::NOHOVERINGEFFECT && !m_bImClicked && !m_bImPermanentlyActive)
    {
        m_SolidColorBrush_UseAll->SetColor( m_BrushColor_hovering );
        if(m_effectHovering == HOVERING_EFFECT::DRAWOUTLINE_H)
        {
            UIButtonBase::m_pContext->DrawRoundedRectangle(m_RoundedRect, m_SolidColorBrush_UseAll, m_OutlineBrushSize);
        }
        else if(m_effectHovering == HOVERING_EFFECT::DRAWMASK_H)
        {
            UIButtonBase::m_pContext->FillRoundedRectangle(m_RoundedRect, m_SolidColorBrush_UseAll);
        }
    }
    // if the button is pressed
    else if(m_bImClicked && m_effectClicked == CLICKED_EFFECT::DRAWMASK_C)
    {
        m_SolidColorBrush_UseAll->SetColor( m_BrushColor_clicked );
        UIButtonBase::m_pContext->FillRoundedRectangle(m_RoundedRect, m_SolidColorBrush_UseAll);
    }
    // do this if the button was completely clicked (mouse clicked AND release on the button) and it supports Permanently-Active status
    else if(m_bImPermanentlyActive && m_effectActive == ACTIVE_EFFECT::DRAWMASK_A)
    {
        m_SolidColorBrush_UseAll->SetColor( m_BrushColor_active );
        UIButtonBase::m_pContext->FillRoundedRectangle(m_RoundedRect, m_SolidColorBrush_UseAll);
    }
    // Do this for normal drawing, i.e. default-status
    else if(m_bDefaultStatus)
    {
        m_SolidColorBrush_UseAll->SetColor( m_BrushColor_default );
        if(m_effectDefault == DRAWOUTLINE_D)
        {
            UIButtonBase::m_pContext->DrawRoundedRectangle(m_RoundedRect, m_SolidColorBrush_UseAll, m_OutlineBrushSize);
        }
        else if (m_effectDefault == DRAWMASK_D)
        {
            UIButtonBase::m_pContext->FillRoundedRectangle(m_RoundedRect, m_SolidColorBrush_UseAll);
        }
    }

    UIButtonBase::m_pContext->FillRoundedRectangle(m_RoundedRect, m_pBitmapBrush_Current);

    // Draw the buttons text
    if(m_bDrawText)
    {
        DWRITE_TEXT_METRICS metrics;
        m_pID2DTextLayout->GetMetrics(&metrics);

        D2D1_POINT_2F point;
        point.x = m_SquareRect.right + FONT_SIZE;

        if(metrics.lineCount == 3)
        {
            point.y = m_SquareRect.top +  (m_height - (metrics.layoutHeight * metrics.lineCount * 1.2f));    
        }
        else
        {
            point.y = m_SquareRect.top + (metrics.height / metrics.lineCount /2.0f);
        }
        m_SolidColorBrush_UseAll->SetColor(m_BrushColor_buttontext);
        UIButtonBase::m_pContext->DrawTextLayout(point, m_pID2DTextLayout, m_SolidColorBrush_UseAll);
    }
}

void SquareButton::RePositionElement(D2D1_POINT_2F& newTopLeft)
{
    m_SquareRect.left   = newTopLeft.x;
    m_SquareRect.top    = newTopLeft.y;
    m_SquareRect.right  = newTopLeft.x + m_width;
    m_SquareRect.bottom = newTopLeft.y + m_height;

    m_RoundedRect.rect = m_SquareRect;
    
    m_Trasnform = D2D1::Matrix3x2F::Translation(D2D1::SizeF(newTopLeft.x, newTopLeft.y));

    ResetTrasform();
}