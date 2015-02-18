#include "ScrollBar.h"

ID2D1BitmapBrush* ScrollBar::m_pBitmapBrush_leftbutton  = nullptr;
ID2D1BitmapBrush* ScrollBar::m_pBitmapBrush_rightbutton = nullptr;

ScrollBar::ScrollBar(void) : m_bRightButtonClicked(false), m_bLeftButtonClicked(false), m_bEmpyRectangleClick(false),
    m_bhoveringEmptyRect(false), m_bhoveringLeftButton(false), m_bhoveringRigthButton(false),
    m_currentValue(0.0f), m_minvalue(0), m_movementinterval(0.0), m_iframe(0), m_pID2DTextLayoutTitle(nullptr), m_pSolidColorBrush(nullptr),
    m_pID2DTextLayoutValue(nullptr)
{

}

ScrollBar::~ScrollBar(void)
{
    SAFE_RELEASE_COM( m_pSolidColorBrush );
    SAFE_RELEASE_COM( m_pID2DTextLayoutTitle );
    SAFE_RELEASE_COM( m_pID2DTextLayoutValue );
}

void ScrollBar::DrawMe(void)
{
    // Set the transformed for the static bitmaps    
    m_pBitmapBrush_rightbutton->SetTransform( m_transformRightButton );
    m_pBitmapBrush_leftbutton->SetTransform( m_transformLeftButton );

    const int MIN_FRAME = 20;
    const int SPEED     = 1;

    ++m_iframe;

    m_filledRectangle.right = m_middleButton.left;

    // Draw filled rectangular area
    if( (m_filledRectangle.right - m_filledRectangle.left) > 0.0f)
    {
        m_pSolidColorBrush->SetColor(D2D1::ColorF(D2D1::ColorF::DodgerBlue));
        m_pContext->FillRectangle(m_filledRectangle, m_pSolidColorBrush);
    }

    // Draw left button
    {
        if(m_bLeftButtonClicked)
        {
            // Set the color to use when filling the rectangle
            m_pSolidColorBrush->SetColor(D2D1::ColorF(D2D1::ColorF::Red));
            m_pContext->FillRectangle(m_leftButton, m_pSolidColorBrush);

            // if the current frame is greater then frame 20, and the speed interval is completed then move the middle button
            if(m_iframe > MIN_FRAME && m_iframe % SPEED == 0)
                MoveMiddleButton();
        }
        m_pContext->FillRectangle(m_leftButton, m_pBitmapBrush_leftbutton);
    }
    
    // Draw right button
    {
        if(m_bRightButtonClicked)
        {
            // Set the color to use when filling the rectangle
            m_pSolidColorBrush->SetColor(D2D1::ColorF(D2D1::ColorF::Red));
            m_pContext->FillRectangle(m_rightButton, m_pSolidColorBrush);

            // if the current frame is greater the frame 20, and the speed interval is completed then move the middle button
            if(m_iframe > MIN_FRAME && m_iframe % SPEED == 0)
                MoveMiddleButton();
        }
        m_pContext->FillRectangle(m_rightButton, m_pBitmapBrush_rightbutton);
    }
    
    // Draw middle button
    {
        m_pSolidColorBrush->SetColor(D2D1::ColorF(D2D1::ColorF::White));
        m_pContext->FillRectangle(m_middleButton, m_pSolidColorBrush);
    }
    
    // Draw Guide rectangle
    {
        m_pSolidColorBrush->SetColor(D2D1::ColorF(D2D1::ColorF::LightGray));
        m_pContext->DrawRectangle(m_emptyRectangle, m_pSolidColorBrush, 0.2f);
    }

    // Draw the title of the scroll bar
    {
        DWRITE_TEXT_METRICS metrics;
        m_pID2DTextLayoutTitle->GetMetrics(&metrics);
        const float RECT_WIDTH = m_emptyRectangle.right - m_emptyRectangle.left;
        D2D1_POINT_2F point;
        point.x = m_emptyRectangle.left + (RECT_WIDTH - metrics.width)/2.0f;
        point.y = m_emptyRectangle.bottom + 2.0f;    
        m_pSolidColorBrush->SetColor(D2D1::ColorF(D2D1::ColorF::White));
        UIButtonBase::m_pContext->DrawTextLayout(point, m_pID2DTextLayoutTitle, m_pSolidColorBrush);
    }

    // Draw the value of the scroll bar
    {
        DWRITE_TEXT_METRICS metrics;
        m_pID2DTextLayoutValue->GetMetrics(&metrics);
        const float RECT_WIDTH = m_emptyRectangle.right - m_emptyRectangle.left;
        D2D1_POINT_2F point;
        point.x = m_emptyRectangle.left + (RECT_WIDTH - metrics.width)/2.0f;
        point.y = m_emptyRectangle.top - (FONT_SIZE + 2.0f);
        UIButtonBase::m_pContext->DrawTextLayout(point, m_pID2DTextLayoutValue, m_pSolidColorBrush);
    }

    // reset frame count if necessary
    if(m_iframe >= 60)
    {
        if(m_bLeftButtonClicked || m_bRightButtonClicked) // continue setting the frame to 20 if either button is clicked, this is so the middle button continue to move while the mouse is pressed
            m_iframe = MIN_FRAME;
        else
            m_iframe = 0;
    }
}

bool ScrollBar::IsInsideElement(int x, int y)
{
    bool isInside = false;
    if( (y < m_leftButton.top) || ( y > m_leftButton.bottom) || (x < m_leftButton.left) || (x > m_rightButton.right) )
        return isInside;
    else
    {
        // Left button 
        if( (x > m_leftButton.left-1) && (x < m_leftButton.right+1))
        {
            m_bhoveringLeftButton = true;
        }
        else
            m_bhoveringLeftButton = false;

        // empty rectangle 
        if( (x > m_leftButton.right) && (x < m_rightButton.left) )
        {
            m_bhoveringEmptyRect = true;
        }
        else
            m_bhoveringEmptyRect = false;
        // right button
        if(x > m_rightButton.left-1 && x < m_rightButton.right+1)
        {
            m_bhoveringRigthButton = true;
        }
        else
            m_bhoveringRigthButton = false;
    }
    return true;
}

float2 ScrollBar::GetCenterPoint(void) const
{
    const auto x_middle = m_emptyRectangle.left + (m_emptyRectangle.right - m_emptyRectangle.left );
    const auto y_middle = m_emptyRectangle.top  + (m_emptyRectangle.bottom - m_emptyRectangle.top );

    return float2(x_middle, y_middle);
}

float ScrollBar::GetRadius(void) const
{
    // radius = Diameter divided by 2
    return (m_rightButton.right -  m_leftButton.left)/2.0f;
}

void ScrollBar::MouseButtonUp(void)
{
    MoveMiddleButton();
    MouseHoveringOnMe(true);
    MouseHoveringOnMe(false);
}

bool ScrollBar::Initialize(float left, float top, UIBUTTON type, const wstring& title, int currentvalue, int minValue, wchar_t * token)
{ 
    const float BUTTON_WIDTH     = 20.0f;
    const float BUTTON_HEIGHT    = 16.0f;

    minValue        = clamp(minValue, 0, 99);
    currentvalue    = clamp(currentvalue, minValue, 100);
    m_token         = token;
    m_minvalue          = minValue;
    m_movementinterval  = (100 - m_minvalue) / 100.0f;
    m_type              = type;

    m_pContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DodgerBlue), &m_pSolidColorBrush);

    // Left Button
    m_leftButton.left       = left;
    m_leftButton.top        = top;
    m_leftButton.right      = left + BUTTON_WIDTH;  // 20 pixels width
    m_leftButton.bottom     = top + BUTTON_HEIGHT;  // 16 pixels height
    m_transformLeftButton   = D2D1::Matrix3x2F::Translation(D2D1::SizeF(m_leftButton.left, m_leftButton.top));

    // Filled rectangle
    m_filledRectangle.left      = m_leftButton.right;
    m_filledRectangle.right     = m_filledRectangle.left;
    m_filledRectangle.top       = top;
    m_filledRectangle.bottom    = m_leftButton.bottom;

    // Middle button/marker
    m_middleButton.left     = m_filledRectangle.left + currentvalue;
    m_middleButton.top      = m_filledRectangle.top;
    m_middleButton.right    = m_middleButton.left + 10.0f;          // Middle button width 
    m_middleButton.bottom   = m_middleButton.top  + BUTTON_HEIGHT;  // Make the middle button square

    // Right button
    m_rightButton.left      = m_leftButton.right + ( m_middleButton.right - m_middleButton.left) + 100.0f; // width of the middle button - 100 is the maximum positions
    m_rightButton.right     = m_rightButton.left + BUTTON_WIDTH;
    m_rightButton.top       = m_leftButton.top;
    m_rightButton.bottom    = m_rightButton.top + BUTTON_HEIGHT;
    m_transformRightButton  = D2D1::Matrix3x2F::Translation( D2D1::SizeF(m_rightButton.left, m_rightButton.top) );

    // Empty rectangle 
    m_emptyRectangle.left   = m_leftButton.right; // Starts at the end of the left scroll bar button
    m_emptyRectangle.right  = m_rightButton.left; // Ends at the start of the right scroll bar button
    m_emptyRectangle.top    = top - 1.0f;
    m_emptyRectangle.bottom = m_emptyRectangle.top + BUTTON_HEIGHT + 1.0f;

    // Initilaize the Scroll bar title
    {
        // Create the text formatting
        const float BOX_WIDTH = 140.f;
        HRESULT hr = S_OK;
        ComPtr<IDWriteTextFormat> pTextFormat;
        hr = UIButtonBase::m_pWriteFactory->CreateTextFormat(wzFONT_TYPE, 
            nullptr, DWRITE_FONT_WEIGHT_NORMAL, 
            DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 
            FONT_SIZE, L"", &pTextFormat);

        hr = UIButtonBase::m_pWriteFactory->CreateTextLayout(title.c_str(), title.length(), pTextFormat.Get(), BOX_WIDTH, FONT_SIZE, &m_pID2DTextLayoutTitle);
    }

    MoveMiddleButton(); // calling this at initialization helps the scroll bar title and value to be properly drawn
    return true;
}

void ScrollBar::MouseHoveringOnMe(bool yesORno)
{
    if(yesORno)
    {
        m_bEmpyRectangleClick = m_bLeftButtonClicked = m_bRightButtonClicked = false;
    }
    else
    {
        m_bhoveringEmptyRect = m_bhoveringLeftButton = m_bhoveringRigthButton = false;
    }
}

void ScrollBar::MouseClickedMe(void) 
{
    m_iframe = 0;
    m_bEmpyRectangleClick   = m_bhoveringEmptyRect      ? true : false;
    m_bLeftButtonClicked    = m_bhoveringLeftButton     ? true : false;
    m_bRightButtonClicked   = m_bhoveringRigthButton    ? true : false;
    MoveMiddleButton();
}

void ScrollBar::MoveMiddleButton(void)
{
    bool value_changed = false;

    if(m_bRightButtonClicked)       // right-arrow being clicked
    {
        if(m_middleButton.right < m_rightButton.left)
        {
            m_middleButton.left     += 1.0f;
            m_middleButton.right    += 1.0f;
            value_changed = true;
        }
    }
    else if(m_bLeftButtonClicked)   // left-arrow being clicked
    {
        if(m_middleButton.left > (m_leftButton.right + m_minvalue))
        {
            m_middleButton.left     -= 1.0f;
            m_middleButton.right    -= 1.0f;
            value_changed = true;
        }
    }

    if(value_changed || m_pID2DTextLayoutValue == nullptr) // m_pID2DTextLayoutValue is tested here because at first it is NULL and we need to initialize it
    {
        // Reset the textLayout COM object
        SAFE_RELEASE_COM( m_pID2DTextLayoutValue );

        // get the new value that the user will see
        const int CURRENT_VALUE =  GetCurrentValue();

        // Use c++ streams to parse the integer value to text characters
        std::wostringstream ws;
        ws << CURRENT_VALUE;
        wstring& str = ws.str();
        str += m_token;

        // Create the text formatting
        const float BOX_WIDTH = 32.0f;
        HRESULT hr = S_OK;
        ComPtr<IDWriteTextFormat> pTextFormat;
        hr = UIButtonBase::m_pWriteFactory->CreateTextFormat(L"Consolas", nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, 
            DWRITE_FONT_STRETCH_NORMAL, FONT_SIZE, L"", &pTextFormat);

        // Create the TextLayout
        hr = UIButtonBase::m_pWriteFactory->CreateTextLayout(str.c_str(), str.length(), pTextFormat.Get(), BOX_WIDTH, FONT_SIZE, &m_pID2DTextLayoutValue);
    }
}
