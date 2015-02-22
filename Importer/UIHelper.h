#if defined(WINDOWS) || defined(_WINDOWS)
#pragma once
#endif
#ifndef UIHELPER_H
#define UIHELPER_H
#include "math_funcs.h"
#include "color codes.h"
#include <string>
using std::wstring;
using namespace omi;
class Circle
{
    short m_outline_width;
    short m_radius;
    float2 m_center;
public:
    Circle(void) : m_outline_width(1), m_radius(1), m_center() { }
};

static void createcircle(const float2& center, const float& radius, const float outline_width)
{
    // calculate vertices
    {
        // y = sin(angle in radians) * radius
        // x = cos(angle in radians) * radius
        const double CIRCUMFERENCE = 2 * PI_DOUBLE*radius;
        const uint VERTS_NUMBER = static_cast<uint>(CIRCUMFERENCE * 0.90) * 2;// I need to explain this code better
        std::vector<float2> vertices;
        vertices.reserve(VERTS_NUMBER);

        const double radians_interval = 2.0 / (VERTS_NUMBER / 2);
        double angle = 0.0;
        while (angle < 2.0)
        {
            // inner circle coordinates
            float2 vertex;
            // x coordinate for innner
            const float x = static_cast<float>(cos(angle) * (radius - outline_width));
            // y coordinate for inner vertices
            const float y = static_cast<float>(cos(angle) * (radius - outline_width));

            vertex.u = x;
            vertex.v = y;
            // next vertex
            angle += radians_interval;
        }
    }
    // calculate inside triangles(for the fillable area)

    // calculate UV indices for inner fillable area
}
static float4 getColor(const COLOR_NAME& color, const float& alpha = 1.0f)
{
    const uint redShift     = 16;
    const uint greenShift   = 8;
    const uint blueShift    = 0;

    const uint redMask      = (0xff << redShift);
    const uint greenMask    = (0xff << greenShift);
    const uint blueMask     = (0xff << blueShift);

    const float R = static_cast<float>((color & redMask)   >> redShift) / 255.0f;
    const float G = static_cast<float>((color & greenMask) >> greenShift) / 255.0f;
    const float B = static_cast<float>((color & blueMask)  >> blueShift) / 255.0f;
    const float A = alpha;
    return float4(R, G, B, A);
}
class Color
{
public:
    Color() : r(0.0f), g(0.0f), b(0.0f), a(1.0f){}
    explicit Color(const COLOR_NAME& _color, const float& _alpha = 1.0f)
    {
        SetColor(_color, _alpha);
    }
    explicit Color(const Color& _color) {}
    float4 getColor(void) { return float4(r, g, b, a); }
    void SetColor(const COLOR_NAME& _color, const float& _alpha = 1.0f)
    {
        float4 lColor = ::getColor(_color, _alpha);
        r = lColor.x;
        g = lColor.y;
        b = lColor.z;
        a = lColor.w;
    }
    void SetColor(const float4& _color) { r = _color.x; g = _color.y; b = _color.z; a = _color.w; }
private:
    float r;
    float g;
    float b;
    float a;
};

enum BRUSH_TYPE { UNDEFINED_BRUSH= 0xDEADDDD, SOLID_COLOR = 0X5011D, BITMAP_BRUSH = 0xB118AB };
class BrushBase
{
public:
    BrushBase() : m_type(BRUSH_TYPE::UNDEFINED_BRUSH){}
    BRUSH_TYPE GetType() const { return m_type; }
    void SetBrushType(const BRUSH_TYPE& _type) { m_type = _type; }
private:
    BRUSH_TYPE m_type;
};

class SolidColorBrush : public BrushBase
{
public:
    SolidColorBrush(void){ SetBrushType(BRUSH_TYPE::SOLID_COLOR); }
    BRUSH_TYPE GetType(void) const { return BrushBase::GetType(); }
    void SetColor(const Color& _color){ m_color = _color; }
    const Color& GetColor (void) const { return m_color; }
private:
    Color m_color;
};

class BitmapBrush : public BrushBase
{
public:
    BitmapBrush(void){ SetBrushType(BRUSH_TYPE::BITMAP_BRUSH); }
    BRUSH_TYPE GetType(void) const { return BrushBase::GetType(); }
    const wstring& GetImageFullPath(void) { }
private:
    wstring m_wzImageFullPath;
};

#endif