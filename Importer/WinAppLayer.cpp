#include "WinApplayer.h"
#include <Windowsx.h>
#include <iostream>
#include <sstream>
#include <cassert>
#include "View.h"
//--------------------------------|
// WinAppLayer Class Definition	  |
//--------------------------------|

namespace { WinAppLayer* temp = nullptr; }

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return temp->WinProc(hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK ChildProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return temp->WinProc(hwnd, uMsg, wParam, lParam);
}

WinAppLayer::WinAppLayer(wstring& AppName, HINSTANCE& hinstance) :
m_AppName(AppName), m_HInstance(hinstance), m_bOnPause(false), m_pIView(nullptr)
{
    temp = this;
}

bool WinAppLayer::Init(void)
{
    SetProcessDPIAware();// Why are we calling this?
    View* pview = new View();

    // start out with a square Window
    m_iWidth = static_cast<int>(static_cast<double>(GetSystemMetrics(SM_CYSCREEN))*.85);
    m_iHeight = m_iWidth;

    RECT rect = { 0, 0, m_iWidth, m_iHeight };
    DWORD dwstyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    AdjustWindowRectEx(&rect, dwstyle, false, WS_EX_WINDOWEDGE);

    // register the Window class
    WNDCLASSEX wc;
    wc.cbClsExtra = 0;
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.cbWndExtra = 0;
    wc.hbrBackground = HBRUSH(COLOR_APPWORKSPACE + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hIconSm = NULL;
    wc.hInstance = m_HInstance;
    wc.lpfnWndProc = WindowProc;
    wc.lpszClassName = m_AppName.c_str();
    wc.lpszMenuName = NULL;
    wc.style = NULL;

    if (!RegisterClassEx(&wc))
    {
        MessageBox(NULL, L"Class couldn't be registered.", NULL, MB_OK);
        return false;
    }

    // Create the Window class
    int Width = (rect.right - rect.left);
    int Height = (rect.bottom - rect.top);

    m_AppHwnd = CreateWindowEx(WS_EX_WINDOWEDGE, m_AppName.c_str(), nullptr,
        dwstyle, 0, 0, Width, Height, NULL, NULL, m_HInstance, NULL);

    // The window wasn't created
    if (!m_AppHwnd)
    {
        MessageBox(NULL, L"The window could not be created.", NULL, MB_OK);
        return false;
    }

    if (!pview->InitDevices())
        return false;
    if (!pview->CreateRenderTargets(m_AppHwnd, m_iWidth, m_iHeight))
        return false;

    // show the windo
    ShowWindow(m_AppHwnd, SW_SHOWNORMAL);
    UpdateWindow(m_AppHwnd);
    SetWindowText(m_AppHwnd, m_AppName.c_str());

    // 
    m_pIView = pview;

    return true;
}

int WinAppLayer::MessageLoop(void)
{
    MSG msg;
    SecureZeroMemory(&msg, sizeof(MSG));

    // Message loop
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, NULL, 0, NULL, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else if (!m_bOnPause)
        {
            GameLoop();
        }
        else
            Sleep(100);
    }
    return static_cast<int>(msg.wParam);
}

// -----------------------------------|
// Windows' Message Procedure WinProc |
// -----------------------------------|
LRESULT WinAppLayer::WinProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static bool bProcessing = false;
    static bool mouseIsCaptured = false;
    static bool mouseClickedUIButton = false;
    static bool blightfollowCamera = false;

    static int mouse_x = 0;
    static int mouse_y = 0;

    switch (message)
    {
    case WM_UPDATE_STABLE:
    case WM_UPDATE_BETA:
    {
        wstring lMessageToUser;
        wstring cmdArguments(L"Updater.exe ");

        if ((I_AM_BETA && message == WM_UPDATE_BETA) || (!I_AM_BETA && message == WM_UPDATE_STABLE) || (I_AM_BETA && WM_UPDATE_STABLE))
            lMessageToUser = TEXT("There is an newer version of 3DMive, would you like to download it now?");

        else if (!I_AM_BETA && message == WM_UPDATE_BETA)
            lMessageToUser = TEXT("There is a newer BETA version of 3DMive, would you like to download it now?");

        cmdArguments += message == WM_UPDATE_BETA ? TEXT("BETA") : TEXT("STABLE");

        if (MessageBox(m_AppHwnd, lMessageToUser.c_str(), L"Quick Question!", MB_YESNO | MB_ICONQUESTION) == IDYES)
        {
            PROCESS_INFORMATION processInfo;
            STARTUPINFO startinfo;
            GetStartupInfo(&startinfo);
            if (CreateProcess(L"Updater.exe", const_cast<wchar_t*>(cmdArguments.c_str()), nullptr, nullptr, false, CREATE_NEW_CONSOLE, nullptr, nullptr, &startinfo, &processInfo))
            {
                PostQuitMessage(WM_QUIT);
                CloseHandle(processInfo.hProcess);
                CloseHandle(processInfo.hThread);
            }
            else
            {
                DWORD error = GetLastError();
                char buffer[256];
                omi_snprintf(buffer, 256, "%i", error);

                string msg;
                msg += "There was an error calling the Updater module. Error code is ";
                msg += buffer;
                msg += '.';
                MessageBox(m_AppHwnd, wstring(msg.begin(), msg.end()).c_str(), nullptr, MB_OK | MB_ICONERROR);
            }
        }
    }
    break;
    case WM_DESTROY:
        PostQuitMessage(WM_QUIT);
        return 0;
        // For when the windows is being minimized or Maximized
    case WM_ACTIVATE:
    {
        if (LOWORD(wParam) == WA_ACTIVE || LOWORD(wParam) == WA_CLICKACTIVE)
            m_bOnPause = false;
        else if (LOWORD(wParam) == WA_INACTIVE)// What is this used for        
            m_bOnPause = true;
    }
    return 0;

    case WM_MOUSEWHEEL:
    {
        if (!bProcessing)
        {
            bProcessing = true; // Do not accept more scrolls until this one is preocessed
            int iDelta = GET_WHEEL_DELTA_WPARAM(wParam);
            if (iDelta > 0)
                m_pIView->ZoomIn();
            else
                m_pIView->ZoomOut();

            bProcessing = false;
        }
    }
    return 0;

    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    {
        mouse_x = GET_X_LPARAM(lParam);
        mouse_y = GET_Y_LPARAM(lParam);
        if (message == WM_LBUTTONDOWN)
        {
            View* pView = reinterpret_cast<View*>(m_pIView);

            // If a UIButton was pressed set the appropriate flag
            if (pView->MouseClickAt(mouse_x, mouse_y) != UIBUTTON::NO_BUTTON)
                mouseClickedUIButton = true;
        }
    }
    return 0;

    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
        if (message == WM_LBUTTONUP && mouseClickedUIButton)// Only process a mouse-button-up if it was on top of a button both at the click event and up event 
        {
            mouse_x = GET_X_LPARAM(lParam);
            mouse_y = GET_Y_LPARAM(lParam);

            View* pView = reinterpret_cast<View*>(m_pIView);

            const UIBUTTON button = pView->MouseButtonUp(mouse_x, mouse_y);
            switch (button)
            {
            case UIBUTTON::FLIPFACES:
            {
                // Only flip faces if there is a mesh loaded.
                if (pView->GetTriangleCount() > 0)
                {
                    if (pView->IsMeshFlipable())
                    {
                        pView->DeleteMesh();
                        m_pIView->AddMesh(pView->FlipFaces());
                    }
                    else                    
                        m_pIView->PrintMessage(UI_MSG_TYPE::CRITICAL_ERROR,L"Mesh is not flippable because it does not have the same amount of normals and vertices.");                    
                }
                else
                    m_pIView->PrintMessage(UI_MSG_TYPE::WARNING, L"There is no 3D-Mesh loaded.");
            }
            break;
            case UIBUTTON::NORMALSONOFF:
                pView->ShowNormals();
                break;

            case UIBUTTON::AXESONOFF:
                pView->ShowAxes();
                break;

            case UIBUTTON::GRIDONOFF:
                pView->ShowGrid();
                break;

            case UIBUTTON::WIREFRAMEMODE:
                pView->WireFrameMode();
                break;

            case UIBUTTON::BACKFACECULLING:
                pView->BackFaceCulling();
                break;

            case UIBUTTON::TEXTURESONOFF:
                pView->TexturesOnOff();
                break;

            case UIBUTTON::FOCUS_ON_MESH:
                pView->FocusOnMesh();
                break;

            case UIBUTTON::DELETEMESH:
            {
                pView->DeleteMesh();
                uint lTriangCount = pView->GetTriangleCount();
                pView->SetTriangleCount(lTriangCount);
            }
            break;

            case UIBUTTON::CAMERAMOVEMENT:
            {
                int value = pView->GetCameraMovementSpeedValue();
                pView->SetCameraMovementSpeedValue(value);
            }
            break;

            case UIBUTTON::CAMERAZOOMSPEED:
            {
                int value = pView->GetCameraZoomSpeed();
                pView->SetCameraZoomSpeed(value);
            }
            break;

            case UIBUTTON::IMPORTMESH:
            {
                uint lTriangCount = pView->GetTriangleCount();
                pView->SetTriangleCount(lTriangCount);
                pView->SetAABB();
                SetWindowText(m_AppHwnd, m_AppName.c_str()); // to prevent the Window text from changing to the file being imported
            }
            break;
            case UIBUTTON::LIGHT_FOLLOW_CAMERA:
                blightfollowCamera = blightfollowCamera ? false : true; // flip the flag
                pView->StopFollowingCamera();
                break;
            case UIBUTTON::LIGHT_TO_EYE:
                if (blightfollowCamera == false)
                {
                    pView->LightFollowCamera();
                    pView->StopFollowingCamera();
                }
                break;
            }
        }

        // reset mouse coords and mouse-capture static variable
        mouseClickedUIButton = false;
        mouse_x = mouse_y = 0;

        if (mouseIsCaptured)// If the mouse was captured with the BUTTONDOWN event we need to release it
        {
            mouseIsCaptured = false;
            ReleaseCapture();
        }
        return 0;

    case WM_MOUSEMOVE:
    {
        const int last_x = GET_X_LPARAM(lParam);
        const int last_y = GET_Y_LPARAM(lParam);
        View* pView = reinterpret_cast<View*>(m_pIView);
        const float2 dpi = pView->GetDPI();
        // Moving the Camera
        if ((wParam & MK_LBUTTON) && mouseClickedUIButton == false)
        {
            // Capture the mouse while dragging it
            if (!mouseIsCaptured)
            {
                mouseIsCaptured = true;
                SetCapture(hwnd);
            }
            int moved_y = last_y - mouse_y;
            int moved_x = last_x - mouse_x;

            pView->MoveCamera(moved_y, moved_x);

            if (blightfollowCamera)
                pView->LightFollowCamera();

            mouse_y = last_y;
            mouse_x = last_x;
        }

        // Moving the Light Source
        else if ((wParam & MK_RBUTTON) && !mouseClickedUIButton && !blightfollowCamera)
        {
            // Capture the mouse while dragging it in case the user goes outside the windows we don't loose focus of mouse drag events
            if (!mouseIsCaptured)
            {
                mouseIsCaptured = true;
                SetCapture(hwnd);
            }
            int moved_y = last_y - mouse_y;
            int moved_x = last_x - mouse_x;

            pView->MoveLight(moved_y, moved_x);

            mouse_y = last_y;
            mouse_x = last_x;
        }

        // Mouse Hovering
        else if (mouseClickedUIButton == false)// Process it because the user is not hovering over a button
            pView->MouseHoveringAt(last_x, last_y);
    }
    break;
    default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }
    return 0;
}

//---------------------------------|
//---------- Game loop ------------|
// --------------------------------|
inline void WinAppLayer::GameLoop(void)
{
    m_pIView->RenderScene();
}

// -----------|
// Destructor |
// -----------|
WinAppLayer::~WinAppLayer(void)
{
    SAFE_DELETE(m_pIView);
}