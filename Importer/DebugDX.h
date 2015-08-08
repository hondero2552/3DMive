#pragma once
#include "Debug_ALL.h"
#include <cassert>
#include <wrl\client.h>
#include <d3d11_1.h>
//#include <dxgi1_2.h>
using Microsoft::WRL::ComPtr;

// Direct2D
#include <d2d1_1.h>
#include <d2d1.h>
#include <d2d1_1helper.h>

#pragma comment(lib, "D2d1.lib")

#if defined(DEBUG) || defined(_DEBUG)
#include <Initguid.h>
#include <debugapi.h>
#include <dxgidebug.h>
typedef HRESULT (WINAPI *GetDebugInterface)(REFIID riid, void **ppDebug);
#endif


#include <vector>
using std::wstring;
using std::for_each;
using std::vector;
namespace omi
{
    struct TexDimension
    {
        wstring mName;
        size_t  mDimension;
        bool    mbfound;
    };
}
template <typename T>
void SAFE_RELEASE_COM(T& x)
{
    if (x)
    {
        x->Release();
        x = nullptr;
    }
}

static wchar_t* GetTextureName(const wstring& fullpath)
{
    // find the "." character
    uint dotAt      = 0;
    uint startsAt   = 0;

    // find the "\" and the "."
    for(size_t i = fullpath.size(); i > 0 ; --i)
    {
        wchar_t ch = fullpath[i];

        if(ch == L'.')
            dotAt = i-1;
        else if(ch == L'\\')
        {
            startsAt = i;
            break; // end the loop
        }
    }
    uint size = dotAt - startsAt;
    wchar_t * name = new wchar_t[size+1];

    // copy the characters
    for(uint i = 0; i < size; ++i)
        name[i] = fullpath[++startsAt];

    // null terminate the string
    name[size] = L'\0';

    // return the name, this code is not responsible for releasing the memory
    return name;
}

static wstring GetTextureFileFormat(const wstring& fullpath)
{
    auto found = std::find(fullpath.begin(), fullpath.end(), L'.');
    ++found;
    wstring format = wstring(found, fullpath.end());
    return format;
}

static bool AllTheSame(const vector<omi::TexDimension>& textures)
{
    bool allthesame = true;
    size_t size = textures.size();
    for(size_t i = 0; i < size; ++i)
    {
        const omi::TexDimension& ltexture = textures[i];
        for(size_t j = 0; j < size; ++j)
        {
            if(ltexture.mbfound && textures[j].mbfound && ltexture.mDimension != textures[j].mDimension)
            {
                allthesame = false;
            }
        }
    }

    return allthesame;
}

// Helper structure
class CompiledShader
{
private:
    char* m_compiledShader;
    uint m_length;
public:
    CompiledShader(void) : m_compiledShader(nullptr) { }
    ~CompiledShader(void) 
    { 
        SAFE_DELETE_ARRAY(m_compiledShader); 
    }

    void SetByteCode(char * ptr, uint length) { m_compiledShader = ptr; m_length = length; }

    const char* GetByteCode(void) const { return m_compiledShader; }
    uint length(void) const { return m_length; }
};