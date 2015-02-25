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
    /*All this mesaurements are in pixel size.*/
    float m_outline_width;
    float m_radius;
    float2 m_center;
public:
    Circle(void) : m_outline_width(1), m_radius(1), m_center() { }
    float GetRadius(void) const { return m_radius; }
    float GetOutlineWidth(void) const { return m_outline_width; }
};

static StrPtrFloat& createcircle(const float2& center, const float& _radius, const float outline_width)
{
    struct lCircleVertex
    {
        float2 mPosW;
        float2 mUV;
    };

    // calculate vertices
    const uint CIRCUMFERENCE = static_cast<uint>(2 * PI_DOUBLE * _radius);
    const uint VERTS_NUMBER = static_cast<uint>((CIRCUMFERENCE % 2 == 0 ? CIRCUMFERENCE : CIRCUMFERENCE + 1));// this ensures the vertices amount is always even
    const double radians_interval = 2.0 / static_cast<double>(VERTS_NUMBER);

    // Allocate the necessary memory to prevent std::vector from calling the expensive allocator function every time the vector increases in size
    std::vector<lCircleVertex> vertices;
    vertices.reserve(VERTS_NUMBER);

    /* Generate the vertices for the circle in World-Space*/
    for (short circle = 1; circle < 3; ++circle)
    {
        double angle = 0.0;
        const double lRadius = circle == 1 ? (_radius - outline_width) : _radius;
        for (size_t vertex_index = 0; vertex_index < VERTS_NUMBER; ++vertex_index)
        {
            // x/y coordinates for innner circle vertices
            const float xPosW = static_cast<float>(cos(angle) * (lRadius));
            const float yPosW = static_cast<float>(sin(angle) * (lRadius));

            // UV coordinates for inner circle
            const float uTexCoord_World = xPosW == 0.0f ? (0.50f) : (xPosW < 0.0f ? 1.0f / (lRadius - abs(xPosW)) : 1.0f / (lRadius + xPosW));
            const float vTexCoord_World = yPosW == 0.0f ? (0.50f) : (yPosW < 0.0f ? 1.0f / (lRadius + abs(xPosW)) : 1.0f / (lRadius - yPosW));

            // save the vertex
            lCircleVertex lvertex;
            lvertex.mPosW.u = xPosW;
            lvertex.mPosW.v = yPosW;
            lvertex.mUV.u   = uTexCoord_World;
            lvertex.mUV.v   = vTexCoord_World;
            vertices.push_back(lvertex);

            // next vertex
            angle += radians_interval;
        }
    }
   
    /* Generate Indices */
    vector<unsigned short> indices;
    indices.reserve(VERTS_NUMBER * 3);
    const unsigned short HALF_VERTICES = VERTS_NUMBER / 2;
    for (unsigned short i = 0; i < HALF_VERTICES; ++i)
    {
        const unsigned short BOTTOM_RIGHT   = i;
        const unsigned short TOP_RIGHT      = i + HALF_VERTICES;
        const unsigned short TOP_LEFT       = (TOP_RIGHT + 1 < (VERTS_NUMBER + 1)) ? (TOP_RIGHT + 1) : (HALF_VERTICES + 1);
        const unsigned short BOTTOM_LEFT    = (BOTTOM_LEFT + 1 < (HALF_VERTICES + 1)) ? BOTTOM_LEFT + 1 : 0;

        // clockwise
        indices.push_back(BOTTOM_RIGHT);
        indices.push_back(BOTTOM_LEFT);
        indices.push_back(TOP_LEFT);

        indices.push_back(BOTTOM_RIGHT);
        indices.push_back(TOP_LEFT);
        indices.push_back(TOP_RIGHT);
    }
}

static float4 getColor(const COLOR_NAME& color, const float& alpha = 1.0f)
{
    const uint redShift = 16;
    const uint greenShift = 8;
    const uint blueShift = 0;

    const uint redMask = (0xff << redShift);
    const uint greenMask = (0xff << greenShift);
    const uint blueMask = (0xff << blueShift);

    const float R = static_cast<float>((color & redMask) >> redShift) / 255.0f;
    const float G = static_cast<float>((color & greenMask) >> greenShift) / 255.0f;
    const float B = static_cast<float>((color & blueMask) >> blueShift) / 255.0f;
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

enum BRUSH_TYPE { UNDEFINED_BRUSH = 0xDEADDDD, SOLID_COLOR = 0X5011D, BITMAP_BRUSH = 0xB118AB };
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
    const Color& GetColor(void) const { return m_color; }
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