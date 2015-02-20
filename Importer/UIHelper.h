#if defined(WINDOWS) || defined(_WINDOWS)
#pragma once
#endif
#include "math_funcs.h"
#ifndef UIHELPER_H
#define UIHELPER_H
using namespace omi;

static void createcircle(const float2& center, const float& radius, const float outline_width)
{
    // calculate vertices
    {
        // y = sin(angle in radians) * radius
        // x = cos(angle in radians) * radius
        const double CIRCUMFERENCE  = 2*PI_DOUBLE*radius;
        const uint VERTS_NUMBER     = static_cast<uint>(CIRCUMFERENCE * 0.90)*2;// I need to explain this code better
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

#endif