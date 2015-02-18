#if defined(WINDOWS) || defined(_WINDOWS)
#pragma once
#endif

#ifndef MESH_HELPER_H
#define MESH_HELPER_H
#include "Texture.h"
#include "math_funcs.h"

using std::vector;

using namespace omi;

static void ReleaseIndexPtrMemory(INDEX_FORMAT format, void*& pIndices)
{
    if(format == INDEX_FORMAT::SHORT_TYPE)
    {
        unsigned short* _ptr = reinterpret_cast<unsigned short*>(pIndices);
        SAFE_DELETE_ARRAY( _ptr );
    }
    else
    {
        uint* _ptr = reinterpret_cast<uint*>(pIndices);
        SAFE_DELETE_ARRAY( _ptr );
    }
}

template<typename T>
static void EmptyPointersList(std::forward_list<T*>& list)
{
    for_each(list.begin(), list.end(), [&] (T*& _ptr)
    {
        SAFE_DELETE( _ptr );
    });
}

// Creates new objects by copying the value of the objects in the first list.
template<typename T>
static void CopyPointersList(const std::forward_list<T*>& _source, std::forward_list<T*>& _destination)
{
    for_each(_source.begin(), _source.end(), [&] (const T* _ptr)
    {
        T* lptr     = new T();
        (*lptr)     = (*_ptr);
        _destination.push_front( lptr );
    });
}

struct PERGROUPDATA
{
    wstring                     mMaterialName;
    Material mMaterial;    
    std::forward_list<Texture*> mTexturesList;
    INDEX_FORMAT                mIndicesFormat;
    uint                        mIndicesCount;
    void*                       mpIndices;   
    
    PERGROUPDATA(void) : mpIndices(nullptr), mIndicesFormat(UNKNOWN), mIndicesCount(0) { }

    ~PERGROUPDATA(void)
    {
        EmptyPointersList( mTexturesList );
        ReleaseIndexPtrMemory(mIndicesFormat, mpIndices);
    }

private:
    PERGROUPDATA(const PERGROUPDATA&) { }
};

#endif