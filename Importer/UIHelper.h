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
        const uint VERTS_NUMBER     = static_cast<uint>(CIRCUMFERENCE * 0.90);

    }
    // calculate inside triangles(for the fillable area)

    // calculate UV indices for inner fillable area

}

#endif