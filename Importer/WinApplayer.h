#pragma once

#include <string>
#include <forward_list>
#include "Debug_ALL.h"
#include "MathTypes.h"
#include "common_data_attributes.h"
#include "IView.h"
#include "version_control.h"

using namespace omi;
using std::wstring;
using std::forward_list;
// STL
#include <list>
#include <memory>
#include <vector>
using std::vector;
using std::list;
using std::wstring;

#define IMPORT_FROM_BUTTON  1000
#define EXPORT_TO_BUTTON    1001
#define EXECUTE_BUTTON      1002

#define IMPORT_FROM_EDITBOX 1003
#define EXPORT_TO_EDITBOX   1004
#define EXECUTE_STATUS      1005

// global enums
enum HWNDS { HWND_PARENT, HWND_IMPORT_FROM, HWND_EXPORT_TO, HWND_MESH_DIPLAY, HWND_EDIT_BOX_IMPORT_EXPORT};

class WinAppLayer
{
private:
    //Variables
    HWND			  m_AppHwnd;
    HINSTANCE		  m_HInstance;
    wstring		  m_AppName;
    IView* m_pIView;
    int m_iWidth;
    int m_iHeight;
    bool m_bOnPause;

public:
    //Functions
    WinAppLayer(wstring& AppName, HINSTANCE& hinstance);
    ~WinAppLayer(void);

    //Getters
    const HWND& GethWnd(void) const { return m_AppHwnd; }
    const HINSTANCE& GetHInstance(void) const { return m_HInstance; } 

    //Windows Message loop
    int MessageLoop(void);

    //Window Procedure to Handle messages from Windows®
    LRESULT CALLBACK WinProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    void mTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

    bool Init(void);
    const wstring& GetAppName(void) const { return m_AppName; }

    void CreateDialogBoxes(void);    

private:
    inline void GameLoop(void);
    bool InitializeRenderer(void);

    // Private functions
    void GetInputFileName(void);
    void GetOutputFileName(void);
    void ExecuteImportExport(void);
};