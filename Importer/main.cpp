#include "WinApplayer.h"

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    wstring w(APPNAME);
    w += L' ';
    w += MAJOR_WCHART;
    w += L'.';
    w += MINOR_WCHART;
    w += L'.';
    w += BUGS_FIXED_WCHART;

    if(I_AM_BETA)
        w += L" [BETA] ";

#if defined(_DEBUG) || defined(_DEBUG)
    w += L" [DEBUG MODE]";
#endif
    HWND hwnd = FindWindow(w.c_str(), nullptr);
    if(hwnd == nullptr)
    {

        WinAppLayer app(w, hInstance);
        if(!app.Init())
            return 0;
        else
            return app.MessageLoop();
    }
    else
    {
        ShowWindow(hwnd, SW_SHOW);
        SetForegroundWindow(hwnd);
        return 0;
    }
}