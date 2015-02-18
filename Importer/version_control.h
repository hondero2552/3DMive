#if defined(WINDOWS) || defined(_WINDOWS)
#pragma once
#endif

#ifndef VERSION_CONTROL_H
#define VERSION_CONTROL_H

#include "Debug_ALL.h"

const static wchar_t * APPNAME = {L"3D-MIVE"};
const bool I_AM_BETA = true;

#define WM_UPDATE_STABLE    0x0400
#define WM_UPDATE_BETA      0x0401

#define M_MAJOR_VERSION      0
#define M_MINOR_VERSION      0
#define M_BUGS_FIXED_VERSION 0

#define M_MAJOR_VERSION_BETA      1
#define M_MINOR_VERSION_BETA      1
#define M_BUGS_FIXED_VERSION_BETA 0

const wchar_t MAJOR_WCHART      = L'1';
const wchar_t MINOR_WCHART      = L'1';
const wchar_t BUGS_FIXED_WCHART = L'0';

enum VERSION_TYPE {MAJOR_VERSION = 0x4111111, MINOR_VERSION, BUG_FIXED, MAJOR_VERSION_BETA, MINOR_VERSION_BETA, BUG_FIXED_BETA, VERSION_UNDEFINED };
struct VERSION_TOKEN
{
    VERSION_TYPE mVersion;
    int mNumber;
    VERSION_TOKEN(void) : mVersion(VERSION_TYPE::VERSION_UNDEFINED), mNumber(0) {  }
};


static bool UpdateAvailableBeta(vector<VERSION_TOKEN>& version_token)
{
    int major, minor, bug_fixed;
    for_each(version_token.begin(), version_token.end(), [&] (VERSION_TOKEN& _versionToken)
    {
        if(_versionToken.mVersion == VERSION_TYPE::MAJOR_VERSION_BETA)
            major = _versionToken.mNumber;
        else if(_versionToken.mVersion == VERSION_TYPE::MINOR_VERSION_BETA)
            minor = _versionToken.mNumber;
        else if(_versionToken.mVersion == VERSION_TYPE::BUG_FIXED_BETA)
            bug_fixed = _versionToken.mNumber;
    });
    
    return ( (major > M_MAJOR_VERSION_BETA) || (minor > M_MINOR_VERSION_BETA) || (bug_fixed > M_BUGS_FIXED_VERSION_BETA) );
}

static bool UpdateAvailableStable(vector<VERSION_TOKEN>& version_token)
{
    int major, minor, bug_fixed;
    for_each(version_token.begin(), version_token.end(), [&] (VERSION_TOKEN& _versionToken)
    {
        if(_versionToken.mVersion == VERSION_TYPE::MAJOR_VERSION)
            major = _versionToken.mNumber;
        else if(_versionToken.mVersion == VERSION_TYPE::MINOR_VERSION)
            minor = _versionToken.mNumber;
        else if(_versionToken.mVersion == VERSION_TYPE::BUG_FIXED)
            bug_fixed = _versionToken.mNumber;
    });
    
    return ( (major > M_MAJOR_VERSION) || (minor > M_MINOR_VERSION) || (bug_fixed > M_BUGS_FIXED_VERSION) );
}

static VERSION_TYPE GetVersionEnum(const string& _szVersion)
{
    // Stable versions
    if(_szVersion ==        "MAJOR_VERSION")
    {
        return VERSION_TYPE::MAJOR_VERSION;
    }
    else if(_szVersion ==   "MINOR_VERSION")
    {
        return VERSION_TYPE::MINOR_VERSION;
    }
    else if(_szVersion ==   "BUG_FIXED_VERSION")
    {
        return VERSION_TYPE::BUG_FIXED;
    }

    // BETA version
    else if(_szVersion == "MAJOR_VERSION_BETA")
    {
        return VERSION_TYPE::MAJOR_VERSION_BETA;
    }
    else if(_szVersion == "MINOR_VERSION_BETA")
    {
        return VERSION_TYPE::MINOR_VERSION_BETA;
    }
    else if(_szVersion ==   "BUG_FIXED_VERSION_BETA")
    {
        return VERSION_TYPE::BUG_FIXED_BETA;
    }

    return VERSION_TYPE::VERSION_UNDEFINED;
}

static vector<VERSION_TOKEN> GetVersionTokens(const string& file)
{
    vector<VERSION_TOKEN> version_token;
    version_token.reserve(10);
    for(size_t index = 0; index < file.size(); ++index)
    {
        if(file[index] == '[')
        {
            VERSION_TOKEN version_enum;
            string szversion;
            ++index;
            while(file[index] != ']')
            {
                szversion+= file[index];
                ++index;
            }
            version_enum.mVersion = GetVersionEnum(szversion);
            szversion.clear();
            while(file[index] != '[')
                ++index;

            ++index;
            while(file[index] != ']')
            {
                szversion+= file[index];
                ++index;
            }
            version_enum.mNumber = atoi(szversion.c_str());
            version_token.push_back(version_enum);
        }
    }
    return version_token;
}

#endif