#if defined(WINDOWS) || defined(_WINDOWS)
#pragma once
#endif
#include "math_funcs.h"
#include "color codes.h"
#ifndef UIHELPER_H
#define UIHELPER_H
using namespace omi;

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
            float x = cos(angle) * (radius - outline_width);
            // y coordinate for inner vertices
            float y = cos(angle) * (radius - outline_width);
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

    const float R = static_cast<float>((color & redMask)   >> redShift)     / 255.0f;
    const float G = static_cast<float>((color & greenMask) >> greenShift)   / 255.0f;
    const float B = static_cast<float>((color & blueMask)  >> blueShift)    / 255.0f;
    const float A = alpha;
    return float4(R, G, B, A);
}
class COLOR
{
public:
    COLOR() : r(0.0f), g(0.0f), b(0.0f), a(1.0f){}
    COLOR(const COLOR_NAME& _color, const float& _alpha = 1.0f)
    {
        SetColor(_color, _alpha);
    }
    void SetColor(const COLOR_NAME& _color, const float& _alpha = 1.0f)
    {
        float4 lColor = getColor(_color, _alpha);
        r = lColor.x;
        g = lColor.y;
        b = lColor.z;
        a = lColor.w;
    }
private:
    float r;
    float g;
    float b;
    float a;
};

struct CIRCLE
{
    short outline_width;
    short radius;
    float2 center;
};
#endif