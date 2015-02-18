#if defined(_WINDOWS) || defined (WINDOWS)
#pragma once
#endif

#ifndef DEBUG_ALL_H
#define DEBUG_ALL_H

#include <mutex>
#include <thread>
#include <new>
#include <memory>
#include <assert.h>
#include <algorithm>
#include <cstdio>
#include <vector>
#include <string>
#include <forward_list>

using std::unique_ptr;
using std::vector;
using std::forward_list;
using std::string;
using std::wstring;
using std::thread;
using std::for_each;


enum TEXTURE_RESOLUTION {RES_256, RES_512, RES_1024, RES_2048, UNSUPPORTED};

typedef unsigned int uint;
typedef unsigned long ulong;
#define OMI_NEW new

#if defined(_WINDOWS) || defined (WINDOWS)
#if defined(WINDOWS_7)
#define _WIN32_WINNT _WIN32_WINNT_WIN7
#define NTDDI_VERSION NTDDI_WIN7
#elif defined(WINDOWS_8)
#define _WIN32_WINNT _WIN32_WINNT_WIN8
#define NTDDI_VERSION NTDDI_WIN8
#endif
#define omi_snprintf _snprintf_s
#include <Windows.h>
#include <Shlobj.h>
#include <ShObjIdl.h>
#else
#endif

// Template Functions

template <typename T, typename Ty> // This function needs to be moved out of here. Maybe create some kind of helper header or something like that.
inline T find_last(T& first, T& last, const Ty& value)
{
    T found = last;
    while(first != last)
    {
        if(*first == value)
            found = first;
        ++first;
    }
    return found;
}
// Deletes what the pointer points to ONLY if the pointer != nullptr;
template <typename T>
void SAFE_DELETE(T& x)
{
    if(x)
    {
        delete x;
        x = nullptr;
    }
}

// Deletes an array only if the pointer != nullptr
template <typename T>
void SAFE_DELETE_ARRAY(T& x)
{
    if(x)
    {
        delete [] x;
        x = nullptr;
    }
}

// ---------|
// MEMORY   |
// ---------|
template <typename T>
bool NEW(T*& t)
{
    try
    {
        t = new T();
    }
    catch (std::bad_alloc)
    {
        t = nullptr;
        return false;
    }
    return true;
}

template <typename T>
bool NEW_ARRAY(T*& t, int size)
{
    try
    {
        t = new T[size];
    }
    catch (std::bad_alloc)
    {
        t = nullptr;
        return false;
    }
    return true;
}

static char* OpenFile(const wchar_t* name, size_t& length, const wchar_t* flags)// This function needs to be moved out of here. Maybe create some kind of helper header or something like that.
{
    length = 0;
    FILE* pfile = nullptr;
    char* buffer = nullptr;

#if defined(_WINDOWS) || defined(WINDOWS)
    _wfopen_s(&pfile, name, flags);
#else
    FILE* file = fopen(fname.c_str(), "r");
#endif
    // If the file could not be found/open
    if(pfile)
    {
        //Load the file into the Heap
        fseek(pfile, NULL, SEEK_END);

        length  = static_cast<int>( ftell(pfile) );
        buffer  = new char [length]; 

        rewind(pfile);
        fread(buffer, 1, length, pfile);
        fclose(pfile);
    }

    return buffer;
}

static size_t FindCharacter(const char* buffer, char character)// This function needs to be moved out of here. Maybe create some kind of helper header or something like that.
{
    size_t index = 1;
    while(buffer[index] != EOF)
    {
        if(buffer[index] == character)
            return index;

        ++index;
    }
    return 0;// NOT FOUND
}

static TEXTURE_RESOLUTION GetTextureIndexFromResolution(uint resolution)// This function needs to be moved out of here. It needs to be where the textures are dealt with, such as the renderer-helper
{
    TEXTURE_RESOLUTION SRV_INDEX = UNSUPPORTED;
    switch (resolution)
    {
    case 256:
        SRV_INDEX = RES_256;
        break;
    case 512:
        SRV_INDEX = RES_512;
        break;
    case 1024:
        SRV_INDEX = RES_1024;
        break;
    case 2048:
        SRV_INDEX = RES_2048;
        break;
    }
    return SRV_INDEX;
}

static wstring GetFileFormat(const wstring& filename)// This function needs to be moved out of here. Maybe create some kind of helper header or something like that.
{
    wstring name;
    name.reserve(5);
    auto dot = find_last(filename.begin(), filename.end(), L'.');
    if(dot != filename.end())
    {
        ++dot;  // take it pass the '.'
        for_each(dot, filename.end(), [&] (const wchar_t& ch)
        {
            name += ch;
        });
    }
    return name;
}

static wstring GetFileName(const wstring& fullfilename)// This function needs to be moved out of here. Maybe create some kind of helper header or something like that.
{
    wstring name;
    name.reserve(20);

    auto dot        = find_last(fullfilename.begin(), fullfilename.end(), L'.');
    auto backslash  = find_last(fullfilename.begin(), fullfilename.end(), L'\\');

    if(backslash == fullfilename.end())
        backslash = fullfilename.begin();
    else
        ++backslash;    // take it pass the \\ character
    for_each(backslash, dot, [&] (const wchar_t& ch)
    {
        name += ch;
    });
    return name;
}

static wstring GetFileDirectory(const wstring& fullfilename)// This function needs to be moved out of here. Maybe create some kind of helper header or something like that.
{
    wstring name;
    name.reserve(20);    
    auto backslash = find_last(fullfilename.begin(), fullfilename.end(), L'\\');
    if(backslash != fullfilename.end())
    {
        ++backslash;
        for_each(fullfilename.begin(), backslash, [&] (const wchar_t& ch)
        {
            name += ch;
        });
    }
    return name;
}

static void Wait(long long mlSeconds)
{
    long long current_time  = time(0);
    long long previous_time = current_time;
    while( (current_time - previous_time) < mlSeconds)
        current_time = time(0);
}
#endif