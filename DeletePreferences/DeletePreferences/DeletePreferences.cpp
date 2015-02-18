// DeletePreferences.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <string>
using std::wstring;
const wchar_t* wsSETTINGS_FILENAME = L"\\3DMiveSettings.config";

int _tmain(int argc, _TCHAR* argv[])
{
    PWSTR szUserAppLocal_path;
    HRESULT hr = SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_CREATE, nullptr, &szUserAppLocal_path);
    if(SUCCEEDED(hr))
    {
        _wprintf_p(L"%s", L"Deleting preference files...");
        wstring wzFile = szUserAppLocal_path;        
        wzFile += wsSETTINGS_FILENAME;
        BOOL bdeleted = DeleteFile(wzFile.c_str());
        if(bdeleted == ERROR_FILE_NOT_FOUND)
        {
            _wprintf_p(L"%s", L"The file was not found.");
        }
        else
            _wprintf_p(L"%s", L"The preference file was successfully deleted.");
        CoTaskMemFree(szUserAppLocal_path);
    }
	return 0;
}

