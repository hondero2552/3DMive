#include "IUIElement.h"

ID2D1DeviceContext* IUIElement::m_pContext      = nullptr;
IDWriteFactory1* IUIElement::m_pWriteFactory    = nullptr;

void UIButtonBase::SetContext(ID2D1DeviceContext* Context)
{
    m_pContext = Context;
}

void UIButtonBase::SetWriteFactory(IDWriteFactory1* pFactory)
{
    m_pWriteFactory = pFactory;
}

void UIButtonBase::SetPermanentlyActiveEffect(ACTIVE_EFFECT effect, const D2D1_COLOR_F& _optional_Brush)
{
    assert(effect != ACTIVE_EFFECT::NOPERMANENTSTATUS);

    m_effectActive = effect;
    if(effect == DRAWMASK_A)    
        m_BrushColor_active = _optional_Brush;
    
}

void UIButtonBase::SetHoveringEffect(HOVERING_EFFECT effect, const D2D1_COLOR_F& _optional_Brush)
{
    assert(effect != HOVERING_EFFECT::NOHOVERINGEFFECT);

    m_effectHovering = effect;
    if(effect == HOVERING_EFFECT::DRAWMASK_H || effect == HOVERING_EFFECT::DRAWOUTLINE_H)    
        m_BrushColor_hovering = _optional_Brush;
    
    else if( effect == HOVERING_EFFECT::CHANGEBITMAP_H)// This doesn't look rigth    
        assert(m_pBitmapBrush_MouseHoveringStatus);    
}

void UIButtonBase::SetClickedEffect(CLICKED_EFFECT effect, const D2D1_COLOR_F& _optional_Brush)
{
    assert(effect != CLICKED_EFFECT::NOCLICKEDEFFECT);

    m_effectClicked = effect;
    if(effect == CLICKED_EFFECT::DRAWMASK_C)    
        m_BrushColor_clicked = _optional_Brush;    
}

void UIButtonBase::SetDefaultEffect(DEFAULT_EFFECT effect, const D2D1_COLOR_F& _optional_Brush)
{
    assert(effect != DEFAULT_EFFECT::NODEFAULTEFFECT);

    m_effectDefault = effect;
    if(effect == DEFAULT_EFFECT::DRAWMASK_D || effect == DEFAULT_EFFECT::DRAWOUTLINE_D)    
        m_BrushColor_default = _optional_Brush;    
}

void UIButtonBase::MouseClickedMe(void)
{
    m_bImClicked = true;

    if(m_effectClicked == CLICKED_EFFECT::CHANGEBITMAP_C)
    {
        assert(m_pBitmapBrush_ClickedStatus != nullptr);
        // save the current status of the button i.e. active or default
        SaveCurrentStatus();
        // 
        m_pBitmapBrush_Current = m_pBitmapBrush_ClickedStatus;
    }
}

// This will only be called on a button after a user has pressed AND released the mouse button ON-TOP of it
void UIButtonBase::MouseButtonUp(void)
{    
    // If the button supports permanent activation
    if(m_effectActive != ACTIVE_EFFECT::NOPERMANENTSTATUS)
    {
        // if the button is not currently active then flag it as active
        // else flag it as NOT active
        if(m_bImPermanentlyActive == false)
        {
            m_bImPermanentlyActive = true;
            // if the permanently active effect is to change the bitmap then set it as appropriately
            if(m_effectActive == ACTIVE_EFFECT::CHANGEBITMAP_A)            
                m_pBitmapBrush_Current = m_pBitmapBrush_PermanentlyActiveStatus;            
        }
        else
        {
            m_bImPermanentlyActive = false;
            // if the permanently active effect is to change the bitmap then set the default bitmap status again
            if(m_effectActive == ACTIVE_EFFECT::CHANGEBITMAP_A)            
                m_pBitmapBrush_Current = m_pBitmapBrush_DefaultStatus;            
        }
    }
    // If it doesn't then just set the default-status bitmap as the current bitmap
    else
        m_pBitmapBrush_Current = m_pBitmapBrush_DefaultStatus;
}

void UIButtonBase::MouseHoveringOnMe(bool yesORno)
{
    // Set hovering status of the button
    m_bHoveringMe = yesORno;

    // If the mouse is hovering on me
    if(m_bHoveringMe)
    {
        // and the hovering effect if to change the bitmap, then set hovering
        // status bitmap as the current bitmap brush
        if(m_effectHovering == CHANGEBITMAP_H)
        {
            SaveCurrentStatus(); // this must be called in to save the current status of the button, i.e. active/default
            m_pBitmapBrush_Current = m_pBitmapBrush_MouseHoveringStatus;
        }
    }
    else
    {
        // if the mouse is not hovering on me anymore then restore the
        // current bitmap to the status before the mouse was hovering over it
        if(m_effectHovering == CHANGEBITMAP_H)        
            RestoreToPreviousStatus();
        
    }
}

void UIButtonBase::SaveCurrentStatus(void)
{
    // Check if the button is in its default status or is permanently active
    // and save the status as appropriate
    // NOTE: ONLY defautl and Permanently-Active statuses are supported, mouse-hovering is not a status.
    if(m_pBitmapBrush_Current == m_pBitmapBrush_DefaultStatus)    
        m_previousStatus = BUTTON_STATUS::DEFAULT;    
    else    
        m_previousStatus = BUTTON_STATUS::PERMANENTLYACTIVE;
}

void UIButtonBase::RestoreToPreviousStatus(void)
{
    switch (m_previousStatus)
    {
    case DEFAULT:
        m_pBitmapBrush_Current = m_pBitmapBrush_DefaultStatus;
        break;
    case PERMANENTLYACTIVE:
        m_pBitmapBrush_Current = m_pBitmapBrush_PermanentlyActiveStatus;
        break;
    }
}

void UIButtonBase::SetDefaultStatusBitmap(ID2D1BitmapBrush* Bitmap) 
{ 
    m_pBitmapBrush_DefaultStatus = Bitmap;
    m_pBitmapBrush_Current = Bitmap;
    m_pBitmapBrush_DefaultStatus->SetTransform(m_Trasnform);
}

void UIButtonBase::SetPermanentActiveStatusBitmap(ID2D1BitmapBrush* Bitmap)
{ 
    m_pBitmapBrush_PermanentlyActiveStatus = Bitmap;
    m_pBitmapBrush_PermanentlyActiveStatus->SetTransform(m_Trasnform);
}

void UIButtonBase::SetMosueHoveringStatusBitmap(ID2D1BitmapBrush* Bitmap) 
{
    m_pBitmapBrush_MouseHoveringStatus = Bitmap;
    m_pBitmapBrush_MouseHoveringStatus->SetTransform(m_Trasnform);
}

void UIButtonBase::SetClickedStatusBitmap(ID2D1BitmapBrush* Bitmap)
{
    m_pBitmapBrush_ClickedStatus = Bitmap; 
    m_pBitmapBrush_ClickedStatus->SetTransform(m_Trasnform);
}

void UIButtonBase::ResetTrasform(void)// Do I really need to transform every brush?
{
    m_SolidColorBrush_UseAll->SetTransform(m_Trasnform);
    m_pBitmapBrush_DefaultStatus->SetTransform(m_Trasnform);

    if( m_pBitmapBrush_MouseHoveringStatus )
        m_pBitmapBrush_MouseHoveringStatus->SetTransform(m_Trasnform);

    if( m_pBitmapBrush_ClickedStatus )
        m_pBitmapBrush_ClickedStatus->SetTransform(m_Trasnform);

    if( m_pBitmapBrush_PermanentlyActiveStatus )
        m_pBitmapBrush_PermanentlyActiveStatus->SetTransform( m_Trasnform );
}