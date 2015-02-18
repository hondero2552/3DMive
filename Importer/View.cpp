#include "View.h"
#include "version_control.h"
#include <Wininet.h>

View::View(void): m_UIVisible(true), m_fps(0), m_timerFrequency(0.0),
    m_pRenderer(nullptr), 
    m_pUserInterface(nullptr)
#if defined(DEBUG) || defined(_DEBUG)
    ,m_DebugDevice(nullptr)  
#endif
{

}

View::~View(void)
{ 
    SAFE_DELETE(m_pUserInterface);
    SAFE_DELETE(m_pRenderer);    

#if defined(_DEBUG) || defined(DEBUG)
    HRESULT hr = m_DebugDevice->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);  
    SAFE_RELEASE_COM(m_DebugDevice);
#endif  

}

bool View::AddMesh(const Mesh* pMesh)
{
    return m_pRenderer->AddMesh(pMesh);
}

void View::PrintMessage(UI_MSG_TYPE type, const wchar_t* message)
{
    wstring Title;
    DWORD flags = MB_OK;
    switch (type)
    {
    case CRITICAL_ERROR:
        Title = L"Critical Error";
        flags |= MB_ICONERROR;
        break;
    case WARNING:
        Title = L"WARNING";
        flags |= MB_ICONWARNING;
        break;
    case INFORMATION:
        break;
    case STATS:
        break;
    default:
        break;
    }

    MessageBox(m_Hwnd, message, Title.c_str(), flags);
}

bool View::InitDevices(void)
{
    m_pUserInterface    = new UserInterface();              // TRY\CATCH BLOCK HERE
    m_pRenderer         = new RendererDx();                 // TRY\CATCH BLOCK HERE

    if(!m_pRenderer->InitializeDeviceAndContext())
        return false;

    IDXGIDevice2* tempDevice = m_pRenderer->GetDXGIDevice();
    m_pUserInterface->InitD2DDevices(this, tempDevice );    

#if defined(DEBUG) || defined(_DEBUG)
    HMODULE module                          = GetModuleHandle(L"Dxgidebug.dll");
    GetDebugInterface dxgiGetDebugInterface = (GetDebugInterface) GetProcAddress(module, "DXGIGetDebugInterface");
    HRESULT hr                              = dxgiGetDebugInterface(IID_PPV_ARGS(&m_DebugDevice));
#endif

    // Timer
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    m_timerFrequency = static_cast<double>(frequency.QuadPart)/1000.0;

    SAFE_RELEASE_COM(tempDevice);

    return true;
}

bool View::CreateRenderTargets(const HWND& hwnd, int width, int height)
{
    m_Hwnd = hwnd;
    // Direct3D
    m_pRenderer->InitializeSwapChain(hwnd, height, width);
    m_pRenderer->InitializeResources();

    IDXGISwapChain1* pswapchain = m_pRenderer->GetSwapChain();

    // Direct2D
    m_pUserInterface->InitRenderTarget(hwnd, pswapchain);
    m_pUserInterface->SetRenderTargetSize(width, height);
    m_pUserInterface->InitDeviceDepentResources();

    // Release temp Object
    SAFE_RELEASE_COM(pswapchain);

    // check for a software update asyncronously
    std::thread(std::bind(&View::CheckForUpdates, std::ref(*this))).detach();

    return true;
}

void View::RenderScene(void)
{
    static Timer timer1, timer2;

    timer1.tick();
    timer2.tick();
    static double potentialFPS = 0;
    if( timer1.TimeElapsed() > 15)
    {
        LARGE_INTEGER previous_tick;
        LARGE_INTEGER current_tick;
        QueryPerformanceCounter(&previous_tick);
        ++m_fps;
        // Draw the meshes inside the screen i.e. imported meshes, grid, axis' arrows, etc
        m_pRenderer->VDrawScene();
        if(m_UIVisible)
        {
            // Draw UI
            m_pUserInterface->DrawScene();
        }
        m_pRenderer->PresentScene();
        QueryPerformanceCounter(&current_tick);

        // Update the PotentialFPS variable
        potentialFPS += 1000/((current_tick.QuadPart - previous_tick.QuadPart)/m_timerFrequency);        
        timer1.RestartCount();
    }

    if(timer2.TimeElapsed() > 999)
    {   
        m_pUserInterface->SetCurrentFPS(m_fps);
        m_pUserInterface->SetPotentialFPS(static_cast<int>(potentialFPS/m_fps));

        potentialFPS    = 0.0;
        m_fps           = 0;

        timer2.RestartCount();
    }    
}

const Mesh* View::FlipFaces(void)
{
    return m_pUserInterface->FlipFaces();
}

bool View::IsMeshFlipable(void) const
{
    return m_pUserInterface->IsFlipable();
}

void View::VExportTextures(const wstring& _in_PathNameFormat, const wstring& _out_Fullpath, const wstring& _out_Format)
{
    const wstring newFile = _out_Fullpath + GetFileName(_in_PathNameFormat) + L'.' + _out_Format;
    // Windows 
#if defined(_WINDOWS) || defined(WINDOWS)
    // For Windows 8
    CopyFile(_in_PathNameFormat.c_str(), newFile.c_str(), false);
#else

#endif
}

void View::SetAABB(void)
{
    m_pUserInterface->SetMeshAABB( m_pRenderer->GetAABB() );
}

void View::CheckForUpdates(void)
{
    // Before anything ensure an internet connection exists....
    HRESULT hr = E_FAIL;    
    if(InternetAttemptConnect(NULL) == ERROR_SUCCESS)
    {
        if(InternetCheckConnection(L"http://www.google.com", FLAG_ICC_FORCE_CONNECTION, NULL))
            hr = S_OK;
        else
        {
            DWORD error = GetLastError();// Good place to implement some error reporting to file
        }
    }

    // 
    if(SUCCEEDED(hr))
    {
        HINTERNET hOpenConnection = InternetOpen(L"3DMive", INTERNET_OPEN_TYPE_DIRECT, nullptr, nullptr, NULL);
        if(hOpenConnection)
        {
            HINTERNET hOpenURL = InternetOpenUrl(hOpenConnection, L"http://www.3dmive.com/updates/3dmiveversion.txt", nullptr, 0, INTERNET_FLAG_NO_CACHE_WRITE, NULL);
            if(hOpenURL)
            {
                string file;
                char szBuffer[1025];
                szBuffer[1024] = '\0';
                DWORD buffsize      = sizeof(size_t);
                size_t file_size    = 0;
                HttpQueryInfo(hOpenURL, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, &file_size, &buffsize, 0);
                file.reserve(file_size);

                // Download the file the file
                DWORD bytesRead = 0;
                do
                {
                    if(InternetReadFile(hOpenURL, szBuffer, 1024, &bytesRead) && bytesRead)
                    {
                        szBuffer[bytesRead] = '\0';      // prevent the string class from reading passed the end of the data
                        file += szBuffer;
                    }
                }
                while(bytesRead);

                auto tokens = GetVersionTokens(file);
                const bool updatestoBeta    = UpdateAvailableBeta(tokens);
                const bool updatesToStable  = UpdateAvailableStable(tokens);

                if(updatestoBeta || updatesToStable)
                {
                    const UINT message = updatestoBeta ? WM_UPDATE_BETA : WM_UPDATE_STABLE;
                    if(!PostMessage(m_Hwnd, message, NULL, NULL))
                    {
                        DWORD lasterror = GetLastError();
                        uint debug = 0;
                    }
                }

                InternetCloseHandle(hOpenURL);
            }
            InternetCloseHandle(hOpenConnection);
        }
    }
}