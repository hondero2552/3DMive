// Updater.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include <string>
#include <iostream>
#include <time.h>
#include <WinInet.h>
#include <Shlobj.h>

using std::string;
using std::wstring;

/* Standard error macro for reporting API errors */ 
void Wait(long long mlSeconds);
void ClearScreen( HANDLE hConsole );

int _tmain(int argc, _TCHAR* argv[])
{
    wstring BETA(L"BETA");
    wstring STABLE(L"STABLE");

    if( (argc < 2))
    {
        wprintf_s(L"%s", L"Invalid command line arguments. Too few.");
        Wait(2);
        return 0;
    }
    else if(BETA != argv[1] && STABLE != argv[1])
    {
        wprintf_s(L"%s", L"Invalid command line arguments. Wrong value.");
        Wait(2);
        return 0;
    }


    const wstring FileVersionURL = BETA == argv[1] ? L"http://www.3dmive.com/updates/betaversion.txt" : L"http://www.3dmive.com/updates/releaseversion.txt";

    HANDLE  hconsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if(hconsole == INVALID_HANDLE_VALUE)
        hconsole = nullptr;

    // Before anything ensure an internet connection exists....
    HRESULT hr = E_FAIL;
    wprintf_s(L"%s", L"Detecting internet connection...");
    if(InternetAttemptConnect(NULL) == ERROR_SUCCESS)
    {
        if(InternetCheckConnection(L"http://www.google.com", FLAG_ICC_FORCE_CONNECTION, NULL))
        {
            hr = S_OK;
            wprintf_s(L"%s\n", L" Internet conection detected.");
        }
        else
        {
            DWORD error = GetLastError();// Good place to implement some error reporting to file
            wprintf_s(L"%s\n",  L"There was a problem detecting your internet connection. Please ensure you are connected to the internet.");
            Wait(2);
        }
    }

    // 
    if(SUCCEEDED(hr))
    {
        wprintf_s(L"%s", L"Requesting interet session from Windows...");
        HINTERNET hOpenConnection = InternetOpen(L"3DMive", INTERNET_OPEN_TYPE_DIRECT, nullptr, nullptr, NULL);
        if(hOpenConnection)
        {
            wprintf_s(L"%s\n", L" internet session granted.");

            wprintf_s(L"%s", L"Connecting to the update server and requesting version information. Please wait...");
            HINTERNET hOpenURL = InternetOpenUrl(hOpenConnection, FileVersionURL.c_str(), nullptr, 0, INTERNET_FLAG_NO_CACHE_WRITE, NULL);
            if(hOpenURL)
            {
                wprintf_s(L"%s\n", L" Connection stablised.");

                string file;
                wstring wzConsoleOutput;
                char szBuffer[1025];
                wchar_t doubleToWChar_t[256];
                szBuffer[1024]      = '\0';
                DWORD buffsize      = sizeof(size_t);
                size_t file_size    = 0;

                const wchar_t* consoleoutput = L"Requesting update file information... ";

                wzConsoleOutput += consoleoutput;                
                wzConsoleOutput += L"0% completed";
                wprintf_s(L"%s", wzConsoleOutput.c_str());

                HttpQueryInfo(hOpenURL, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, &file_size, &buffsize, 0);
                file.reserve(file_size);                

                DWORD bytesRead = 0;
                DWORD totalbytesread = 0;
                do
                {
                    if(InternetReadFile(hOpenURL, szBuffer, 1024, &bytesRead) && bytesRead)
                    {
                        totalbytesread += bytesRead;
                        szBuffer[bytesRead] = '\0';      //prevent the string class from reading passed the end of the data
                        file += szBuffer;

                        // Update the console output
                        {
                            wzConsoleOutput.clear();
                            wzConsoleOutput += consoleoutput;

                            // Get the percentage
                            const double percentage_downloaded = static_cast<double>((totalbytesread*100.0)/file_size);
                            _snwprintf_s(doubleToWChar_t, 256, 5, L"%f", percentage_downloaded);

                            wzConsoleOutput += doubleToWChar_t;
                            wzConsoleOutput += L"% ";
                            wzConsoleOutput += L"completed";
                            ClearScreen(hconsole);
                            wprintf_s(L"%s", wzConsoleOutput.c_str());
                        }
                    }
                }
                while(bytesRead);

                wprintf_s(L"%s\n", L"\nDetails acquired successfully.");
                Wait(2);
                ClearScreen(hconsole);

                // Download the file with the update file info
                wstring updateFile(file.begin(), file.end());
                wstring filetodownload = L"http://www.3dmive.com/downloads/";
                filetodownload += updateFile;

                HINTERNET hOpenURLFileToDownload = InternetOpenUrl(hOpenConnection, filetodownload.c_str(), nullptr, 0, INTERNET_FLAG_NO_CACHE_WRITE, NULL);
                if(hOpenURLFileToDownload)
                {
                    file.clear();
                    HttpQueryInfo(hOpenURLFileToDownload, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, &file_size, &buffsize, 0);
                    wzConsoleOutput = L"Downloading update file. ";
                    wprintf_s(L"%s\n", L"Downlading update file. 0% completed.");
                    file.reserve(file_size);
                    bytesRead         = 0;
                    totalbytesread    = 0;
                    
                    // Create file and save it to the user's Downloads folder
                    PWSTR szUserAppLocal_path;
                    HRESULT hr = SHGetKnownFolderPath(FOLDERID_Downloads, KF_FLAG_CREATE, nullptr, &szUserAppLocal_path);

                    wstring file_name = szUserAppLocal_path;
                    file_name += L"\\";
                    file_name += updateFile;

                    FILE* cfile = nullptr;
                    _wfopen_s(&cfile, file_name.c_str(), L"wb");
                    
                    CoTaskMemFree(szUserAppLocal_path);
                    do
                    {
                        if(InternetReadFile(hOpenURLFileToDownload, szBuffer, 1024, &bytesRead) && bytesRead)
                        {
                            totalbytesread += bytesRead;
                            fwrite(szBuffer, bytesRead, 1, cfile);

                            // Update the console output
                            {
                                wzConsoleOutput.clear();
                                wzConsoleOutput += consoleoutput;

                                // Get the percentage
                                const double percentage_downloaded = static_cast<double>((totalbytesread*100.0)/file_size);
                                _snwprintf_s(doubleToWChar_t, 256, 5, L"%f", percentage_downloaded);

                                wzConsoleOutput += doubleToWChar_t;
                                wzConsoleOutput += L"% ";
                                wzConsoleOutput += L"completed";
                                ClearScreen(hconsole);
                                wprintf_s(L"%s", wzConsoleOutput.c_str());
                            }
                        }
                    }
                    while(bytesRead);                    
                    fclose(cfile);

                    InternetCloseHandle(hOpenURLFileToDownload);
                    //
                    if(totalbytesread == file_size)
                    {

                        PROCESS_INFORMATION processInfo;
                        STARTUPINFO startinfo;
                        GetStartupInfo(&startinfo);
                        if(CreateProcess(file_name.c_str(), nullptr, nullptr, nullptr, false, NORMAL_PRIORITY_CLASS, nullptr, nullptr, &startinfo, &processInfo))
                        {
                            CloseHandle(processInfo.hProcess);
                            CloseHandle(processInfo.hThread);
                        }
                    }
                }
                else
                {
                    wprintf_s(L"%\n", L"The update file could not be downloaded.");
                    Wait(2);
                }
                wprintf_s(L"%s", L"Closing connection to the server...");
                InternetCloseHandle(hOpenURL);
                wprintf_s(L"%s\n", L" conection closed successfully.");
            }
            else
            {
                wprintf_s(L"%s\n", L"Request failed. The information requested from the server wasn't found.");
                Wait(2);
            }

            wprintf_s(L"%s", L"Closing internet session... ");
            InternetCloseHandle(hOpenConnection);
            wprintf_s(L"%s\n", L"Session closed.");
        }
        else
        {
            wprintf_s(L"%s\n", "Request failed. Internet session was not allowed by Windows.");
            Wait(2);
        }
    }
    CloseHandle(hconsole);

    return 0;
}

void Wait(long long mlSeconds)
{
    long long current_time  = time(0);
    long long previous_time = current_time;
    while( (current_time - previous_time) < mlSeconds)
        current_time = time(0);
}

void ClearScreen( HANDLE hConsole )
{
    if(!hConsole)
        return;
    COORD coordScreen = { 0, 0 };    /* here's where we'll home the cursor */ 
    BOOL bSuccess;
    DWORD cCharsWritten;
    CONSOLE_SCREEN_BUFFER_INFO consoleBuffer; // to get console buffer info, i.e. number of character cells in the current buffer, and attributes 
    DWORD consolesize;                 


    // get the number of character cells in the current buffer

    bSuccess = GetConsoleScreenBufferInfo( hConsole, &consoleBuffer );
    consolesize = consoleBuffer.dwSize.X * consoleBuffer.dwSize.Y; // width * height

    // fill the entire screen with blanks
    bSuccess = FillConsoleOutputCharacter( hConsole, (TCHAR) ' ', consolesize, coordScreen, &cCharsWritten );

    // get the current text attribute
    bSuccess = GetConsoleScreenBufferInfo( hConsole, &consoleBuffer );

    // now set the buffer's attributes accordingly
    bSuccess = FillConsoleOutputAttribute( hConsole, consoleBuffer.wAttributes,
        consolesize, coordScreen, &cCharsWritten );

    // put the cursor at (0, 0)
    bSuccess = SetConsoleCursorPosition( hConsole, coordScreen );
}
