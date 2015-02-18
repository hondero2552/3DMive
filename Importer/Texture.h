#if defined(_WINDOWS) || defined (WINDOWS)
#pragma once
#endif

#ifndef TEXTURE_H
#define TEXTURE_H

#include "common_data_attributes.h"
#include "Debug_ALL.h"
#include <string>
using std::wstring;
using std::string;

class Texture
{
private:
    TEXTURE_TYPE m_type;
    wstring m_path;

public:
    Texture(void);
    ~Texture(void);
    void SetFilePath(const wstring& path) { m_path = path; }
    void SetType(const TEXTURE_TYPE& type) { m_type = type; }

    const TEXTURE_TYPE& GetTextureType(void) const { return m_type; }
    const wstring& GetTextureFullPath(void) const { return m_path; }
};

static string GetTextureTypeForOBJ(TEXTURE_TYPE type)
{
    string wtype;
    switch (type)
    {
    case UNDEFINED:
        break;
    case AMBIENT_MAP:
        wtype = "map_Ka";
        break;
    case DIFFUSE_MAP:
        wtype = "map_Kd";
        break;
    case NORMAL_MAP_TANGENT:
        wtype = "map_bump";
        break;
    case HEIGHT_MAP:
        break;
    case LIGHT_MAP:
        break;
    case SHADOW_MAP:
        break;
    case SPECULAR_MAP:
        wtype = "map_Ks";
        break;
    default:
        break;
    }
    return wtype;
}

#endif