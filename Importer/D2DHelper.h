#pragma once
// Direct2D
#include <d2d1_1.h>
#include <d2d1.h>
#include <Wincodec.h>
#include <wincodecsdk.h>

#include "DebugDX.h"


static ID2D1BitmapBrush* CreateBitmapBrush(wchar_t* filename, IWICImagingFactory * pFactory, ID2D1DeviceContext* pDeviceContext)
{
    //
    ID2D1BitmapBrush* pBitmapBrush      = nullptr;
    IWICBitmapDecoder* pDecoder         = nullptr;
    IWICBitmapFrameDecode* pSource      = nullptr;
    IWICStream* pStream                 = nullptr;
    IWICFormatConverter* pConverter     = nullptr;

    // Create decoder
    HRESULT hr = pFactory->CreateDecoderFromFilename(filename, nullptr, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &pDecoder);

    // Get The image
    if(SUCCEEDED(hr))
    {
        hr = pDecoder->GetFrame(0, &pSource);
    }

    // Create the converter so we can convert ANY image into our specified D2D Format (BGRA)
    if(SUCCEEDED(hr))
    {
        hr = pFactory->CreateFormatConverter(&pConverter);
    }

    // Convert the pixel format
    if(SUCCEEDED(hr))
    {
        hr = pConverter->Initialize(pSource, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, nullptr, 0.f, WICBitmapPaletteTypeMedianCut);
    }

    // Create the bitmap
    ID2D1Bitmap* bitmap = nullptr;// change this to use MS comptr class
    if(SUCCEEDED(hr))
    {
        hr = pDeviceContext->CreateBitmapFromWicBitmap(pConverter, &bitmap);
    }

    // Create the Bitmap-Brush
    if(SUCCEEDED(hr))
    {
        hr = pDeviceContext->CreateBitmapBrush(bitmap, &pBitmapBrush);
        pBitmapBrush->SetExtendModeX(D2D1_EXTEND_MODE_CLAMP);
        pBitmapBrush->SetExtendModeY(D2D1_EXTEND_MODE_CLAMP);
        pBitmapBrush->SetInterpolationMode(D2D1_BITMAP_INTERPOLATION_MODE_LINEAR);
    }

    SAFE_RELEASE_COM( bitmap );
    SAFE_RELEASE_COM( pConverter );
    SAFE_RELEASE_COM( pSource );
    SAFE_RELEASE_COM( pDecoder );

    return pBitmapBrush;
}

static IDWriteTextLayout* CreateTextLayout(IDWriteFactory1* pWriteFactory, const wstring& text, const float font_size = FONT_SIZE)
{
    IDWriteTextLayout* pTextLayout = nullptr;
    // Create the text formatting
    const float box_witdh = text.length() * font_size;
    HRESULT hr = S_OK;
    ComPtr<IDWriteTextFormat> pTextFormat;

    hr = pWriteFactory->CreateTextFormat(
        wzFONT_TYPE, 
        nullptr, 
        DWRITE_FONT_WEIGHT_NORMAL, 
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL, 
        FONT_SIZE, 
        L"", 
        &pTextFormat);
    if(SUCCEEDED(hr))
        hr = pWriteFactory->CreateTextLayout(text.c_str(), text.length(), pTextFormat.Get(), box_witdh, FONT_SIZE, &pTextLayout);

    return pTextLayout;
}