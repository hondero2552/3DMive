#include "User Interface.h"
#include "D2DHelper.h"

#include "RoundButton.h"
#include "SquareButton.h"
#include "ScrollBar.h"
#include "View.h"

const FILE_FORMAT FILE_FORMATS_ARRAY [4] = {FILE_FORMAT::OBJ, FILE_FORMAT::FBX, FILE_FORMAT::HND, FILE_FORMAT::OBJE };

UserInterface::UserInterface(void) : m_iWidth(0), m_iHeight(0), m_iPotentialFPS(0), m_frameCount(0), m_hwnd(0), m_bMouseIsCaptured(false),
    m_pD2DDevice(nullptr),
    m_pD2DContext(nullptr),
    m_pD2DFactory(nullptr),
    m_pWicImagingFactory(nullptr),
    m_pD2DTargetBitmap(nullptr),
    m_pTextFormat(nullptr),
    m_pDWriteFactory(nullptr),
    m_pTextBrush(nullptr),
    m_pBackArrow(nullptr),
    m_dlg_OpenFile(nullptr), m_dlg_SaveFile(nullptr), m_pIView(nullptr), m_pCurrentScreen(nullptr), m_pMaterialsScreen(nullptr),
    m_MaterialBeingEdited(nullptr),
    m_iTriangleCount(0),
    m_iCameraMovementSpeed(1), m_iCameraZoomSpeed(1),
    m_dpiX(96.0f), m_dpiY(96.0f)
{

}

UserInterface::~UserInterface(void)
{
    SaveUserSettingsToFile();

    // Release the file dialogs
    SAFE_RELEASE_COM( m_dlg_OpenFile);
    SAFE_RELEASE_COM( m_dlg_SaveFile);

    // Delete all the Meshes
    EmptyMeshList();
    SAFE_DELETE( m_pBackArrow );

    // Release the static memory from the scroll bar
    ScrollBar* ptr = reinterpret_cast<ScrollBar*>( m_pCameraMovementSpeedSB );
    ptr->ReleaseStaticMemory();

    // Delete all the Screens
    for_each(m_ScreensList.begin(), m_ScreensList.end(), [&] (Screen* _ptr)
    {
        SAFE_DELETE( _ptr );
    });

    // Release DirectX2D Variables
    SAFE_RELEASE_COM( m_pTextBrush );
    SAFE_RELEASE_COM( m_pDWriteFactory );
    SAFE_RELEASE_COM( m_pTextFormat );
    SAFE_RELEASE_COM( m_pD2DTargetBitmap );
    SAFE_RELEASE_COM( m_pD2DContext );
    SAFE_RELEASE_COM( m_pD2DDevice );
    SAFE_RELEASE_COM( m_pD2DFactory );

    SAFE_RELEASE_COM( m_pWicImagingFactory );

    // Release the COM Library
    CoUninitialize();
}

bool UserInterface::InitD2DDevices(IView* pview, IDXGIDevice2* d3dDevice)
{
    m_pIView = pview;

    // Direct 2D Stuff
    D2D1_FACTORY_OPTIONS debugLevel;
#if defined(DEBUG) || defined(_DEBUG)
    debugLevel.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#else
    debugLevel.debugLevel = D2D1_DEBUG_LEVEL_NONE;
#endif

    HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory1), &debugLevel, reinterpret_cast<void**>(&m_pD2DFactory));

    hr = m_pD2DFactory->CreateDevice(d3dDevice, &m_pD2DDevice);

    hr = m_pD2DDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &m_pD2DContext);

    // Create text fonts factory
    hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(m_pDWriteFactory), reinterpret_cast<IUnknown **>(&m_pDWriteFactory));

    if(SUCCEEDED(hr))
    {
        hr = m_pDWriteFactory->CreateTextFormat(L"Consolas", nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 14.0f, L"", &m_pTextFormat);
    }

    // This will always need to be done before anything else
    UIButtonBase::SetContext(m_pD2DContext);
    UIButtonBase::SetWriteFactory(m_pDWriteFactory);

    // Load the default values in case they were not saved to the file 
    LoadDefaultValues();

    // We need to check if the user has used the program before and saved the settings, if so then load them approprately
    {
        PWSTR szUserAppLocal_path;
        HRESULT hr = SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_CREATE, nullptr, &szUserAppLocal_path);

        wstring wzFile = szUserAppLocal_path;
        wzFile += wsSETTINGS_FILENAME;

        size_t fileLength = 0;
        const char* FILEBUFFER = OpenFile(wzFile.c_str(), fileLength, L"rb");

        // Only change the default values if the user has used the program before
        if(FILEBUFFER)
        {
            LoadSettingsFromFile(FILEBUFFER, fileLength);
        }

        // Free used memory
        CoTaskMemFree(szUserAppLocal_path);
        SAFE_DELETE_ARRAY( FILEBUFFER );
    }

    return true;
}

bool UserInterface::InitRenderTarget(const HWND& hwnd, IDXGISwapChain1* pSwapChain)
{
    m_hwnd = hwnd;
    // Initialize the COM Library
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED  | COINIT_SPEED_OVER_MEMORY | COINIT_DISABLE_OLE1DDE);
    if( !SUCCEEDED(hr) )
    {
        return false;                                                   // ADD DEBUG MESSAGE
    }
    float2 DPI = GetDPI();

    IDXGISurface1* dxgibackBuffer = nullptr;

    hr = pSwapChain->GetBuffer(0, IID_PPV_ARGS(&dxgibackBuffer));

    D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(
        D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
        D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE),
        DPI.u,
        DPI.v);

    hr = m_pD2DContext->CreateBitmapFromDxgiSurface(dxgibackBuffer, &bitmapProperties, &m_pD2DTargetBitmap);

    // last step
    m_pD2DContext->SetTarget(m_pD2DTargetBitmap);
    m_RenderTargetSize = m_pD2DContext->GetSize();

    CreateDialogBoxes();
    return true;
}

float2 UserInterface::GetDPI(void)
{
    m_pD2DFactory->GetDesktopDpi(&m_dpiX, &m_dpiY);    
    return float2(m_dpiX, m_dpiY);
}

bool UserInterface::InitDeviceDepentResources(void)
{
    // Initialize the WIC factory so we can create D2DBitmaps from image files
    HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, (LPVOID*)&m_pWicImagingFactory);
    if(FAILED(hr))
    {
        m_pIView->PrintMessage(UI_MSG_TYPE::CRITICAL_ERROR, L"There was a problem initializing the WIC Imaging factory. Program excution cannot continue.");
        return false;
    }

    m_pD2DContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White, 1.0f), &m_pTextBrush);
    m_pD2DContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Maroon, 1.0f), &m_pTextBrushRed);

    CreateContexMenu();

    CreateMainScreen();

    // Initialize statistics
    SetCurrentFPS(0);
    SetPotentialFPS(0);
    SetTraingleCount(0);
    m_pTextLayoutCorrdinateSystem = CreateTextLayout(m_pDWriteFactory, wstring(L"Coord System: Left-Handed"), UI_STATS_FONT_SIZE);

    return true;
}

void UserInterface::SetCurrentFPS(int fps) 
{ 
    m_iCurrentFPS = fps;
    std::wostringstream ws; 
    ws << L"FPS: " << m_iCurrentFPS;
    m_pTextLayoutFPS = CreateTextLayout(m_pDWriteFactory, ws.str(), UI_STATS_FONT_SIZE);
}

void UserInterface::SetPotentialFPS(int potentialFPS)
{
    m_iPotentialFPS = potentialFPS;

    std::wostringstream ws; ws << L"Potential fps: " << m_iPotentialFPS;    
    m_pTextLayoutPotentialFPS = CreateTextLayout(m_pDWriteFactory, ws.str(), UI_STATS_FONT_SIZE);
}

void UserInterface::SetTraingleCount(uint count) 
{
    m_iTriangleCount = count;

    std::wostringstream ws; ws << L"Triangle count: " << m_iTriangleCount;
    m_pTextLayoutTriangleCount = CreateTextLayout(m_pDWriteFactory, ws.str(), UI_STATS_FONT_SIZE);
}

void UserInterface::SetMeshAABB(const AABB& _aabb)
{    
    const float length  = _aabb.m_highest.z - _aabb.m_lowest.z;
    const float width   = _aabb.m_highest.x - _aabb.m_lowest.x;
    const float height  = _aabb.m_highest.y - _aabb.m_lowest.y;

    std::wostringstream ws; ws 
        << L"Mesh length(z-axis): " << length;
    m_pTextLayoutMeshLength = CreateTextLayout(m_pDWriteFactory, ws.str(), UI_STATS_FONT_SIZE);

    std::wostringstream ws1;ws1 
        << L"Mesh width (x-axis): " << width;
    m_pTextLayoutMeshWidth = CreateTextLayout(m_pDWriteFactory, ws1.str(), UI_STATS_FONT_SIZE);

    std::wostringstream ws2; ws2 
        << L"Mesh height(y-axis): " << height;
    m_pTextLayoutMeshHeight = CreateTextLayout(m_pDWriteFactory, ws2.str(), UI_STATS_FONT_SIZE);
}

void UserInterface::DrawScene(void)
{
    m_pD2DContext->BeginDraw();

    m_pCurrentScreen->DrawElements();
    
    if(!m_pContextMenuScreen->IsHidden())
        m_pContextMenuScreen->DrawElements();

    // Draw frame statistics, i.e. FPS, Potential-FPS, Triangle Count, handedness
    {
        const float VERTICAL_SEPARATION_FACTOR = 14.0f;// This should be implemented to reflect the size of the fonts being used plus 2 pixels vertically.
        D2D1_POINT_2F point;
        point.x = 1.0f;// This is to align the text from the left as well as separate it from the window border

        // Draw FPS
        if(m_pTextLayoutFPS) // When will this NOT be true?
        {
            point.y = 1.0f;
            m_pD2DContext->DrawTextLayout(point, m_pTextLayoutFPS.Get(), m_iCurrentFPS >= 60 ? m_pTextBrush : m_pTextBrushRed.Get());
        }

        // Draw Potential-FPS
        if(m_pTextLayoutPotentialFPS)
        {
            point.y += VERTICAL_SEPARATION_FACTOR;
            m_pD2DContext->DrawTextLayout(point, m_pTextLayoutPotentialFPS.Get(), m_pTextBrush);
        }

        // Draw the coordinate system info
        if(m_pTextLayoutCorrdinateSystem)
        {
            point.y += VERTICAL_SEPARATION_FACTOR;
            m_pD2DContext->DrawTextLayout(point, m_pTextLayoutCorrdinateSystem.Get(), m_pTextBrush);
        }

        // Draw The Triangle Count
        if(m_pTextLayoutTriangleCount && m_iTriangleCount > 0)
        {
            point.y += VERTICAL_SEPARATION_FACTOR;
            m_pD2DContext->DrawTextLayout(point, m_pTextLayoutTriangleCount.Get(), m_pTextBrush);
            // Draw length
            point.y += VERTICAL_SEPARATION_FACTOR;
            m_pD2DContext->DrawTextLayout(point, m_pTextLayoutMeshLength.Get(), m_pTextBrush);
            // Draw width
            point.y += VERTICAL_SEPARATION_FACTOR;
            m_pD2DContext->DrawTextLayout(point, m_pTextLayoutMeshWidth.Get(), m_pTextBrush);
            // Draw height
            point.y += VERTICAL_SEPARATION_FACTOR;
            m_pD2DContext->DrawTextLayout(point, m_pTextLayoutMeshHeight.Get(), m_pTextBrush);
        }
    }

    // Draw the arrow on the screen    
    m_pBackArrow->DrawMe();

    m_pD2DContext->EndDraw();
}

void UserInterface::SetRenderTargetSize(int width, int height)
{
    m_iWidth    = width;
    m_iHeight   = height;
}

// Process user request or properly forward the message to the view
UIBUTTON UserInterface::MouseButtonUp(int x, int y, uint quadrant)
{
    UIBUTTON clickedButton = UIBUTTON::OUTSIDE_SCREEN;                  ///////////////////////////////////////////////FIX THIS OUTSIDE_SCREEN may NEVER HAPPEN

    // First check which screen has the mouse focus ( see MouseHoveringAt(x, y) ) and then test
    // if any of the screen's buttons was completely pressed. If the current screen has the mouse
    // focus then don't forget to test the BackArrow button which does not belong to any screen
    if(m_pCurrentScreen->HasMouseFocus())
    {
        if(m_pBackArrow->isClicked() == false)
        {
            // check if the mouse was up on top of the button that was initially clicked
            clickedButton = m_pCurrentScreen->MouseButtonUp(x, y);
            IUIElement* pUIElement = m_pCurrentScreen->GetCurrentUIElement();

            // if it was, the current UIElement of the screen will be set to that button
            // else it will return nullptr signaling that the mouse button was not released on the clicked button
            if(pUIElement)
            {
                // Handle the specific case of editing material's property
                if(m_pCurrentScreen->GetScreenType() == SCREEN::MATERIALEDITING_SCREEN)
                {
                    // Set current material being edited
                    SetCurrentMaterialBeingEdited(pUIElement->VGetElementText());
                }

                // Get the child screen of that button and set it if it does have one
                Screen* ptr = pUIElement->GetChildScreen();
                if(ptr)
                {
                    SetCurrentScreen(*ptr);
                    // Handle the specific case of being in the material editing screen
                    // now we have to clear the current material being edited
                }
            }
        }
        else
        {
            m_pBackArrow->UnclickedMe();
            if(m_pBackArrow->IsInsideElement(x, y))
            {
                clickedButton = m_pBackArrow->GetButton();
            }
            else
            {
                m_pBackArrow->RestoreToPreviousStatus();
                m_pBackArrow->MouseHoveringOnMe(false);
            }
        }
    }
    else if(m_pContextMenuScreen->HasMouseFocus())
    {
        clickedButton = m_pContextMenuScreen->MouseButtonUp(x, y);        
    }

    switch (clickedButton)
    {
    case IMPORTMESH:
        GetInputFileName();
        MouseHoveringAt(x, y, quadrant);
        {
            Screen* ptr = m_pCurrentScreen->GetParentScreen();
            if(ptr)
                SetCurrentScreen(*ptr);
        }
        break;
    case BACKARROW: // Back button was clicked
        {
            Screen* ptr = m_pCurrentScreen->GetParentScreen();
            if(ptr)
                SetCurrentScreen(*ptr);
        }
        break;
    case NO_BUTTON:
        MouseHoveringAt(x, y, NULL);
        break;
    case UIBUTTON::SHOWHIDECONTEXTMENU:
        m_pContextMenuScreen->IsHidden() ? m_pContextMenuScreen->ShowScreen() : m_pContextMenuScreen->HideScreen();
        break;
    case DELETEMESH:
        EmptyMeshList();
        break;
    case EXPORTMESH:
        GetOutputFileName();
        break;
        // set the term that is being edited
    case UIBUTTON::AMBIENT_TERM:
    case UIBUTTON::DIFFUSE_TERM:
    case UIBUTTON::SPECULAR_TERM:
        m_MaterialTermBeingEdited = clickedButton;
        break;
    case UIBUTTON::RED_CHANNEL:
    case UIBUTTON::GREEN_CHANNEL:
    case UIBUTTON::BLUE_CHANNEL:
    case UIBUTTON::ALPHA_CHANNEL:
    case UIBUTTON::SPECULAR_FACTOR:
        EditMaterialChannel(m_pCurrentScreen->GetCurrentUIElement(), clickedButton);
        break;
    default:
        break;
    }
    return clickedButton;
}

void UserInterface::CreateDialogBoxes(void)
{
    // OPEN FILE
    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_dlg_OpenFile));  
    if (SUCCEEDED(hr))
    {
        // File formats
        COMDLG_FILTERSPEC filetypes[] = 
        {   
            { L"Wavefront Files (.obj)", L"*.obj"}
        };
        m_dlg_OpenFile->SetFileTypes(ARRAYSIZE(filetypes), filetypes);
    }

    // SAVE FILE
    hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_dlg_SaveFile));
    if (SUCCEEDED(hr))
    {
        // File formats
        COMDLG_FILTERSPEC filetypes[] = 
        { 
            {L"Wavefront Files (.OBJ)", L"*.obj"}
        };
        m_dlg_SaveFile->SetFileTypes(ARRAYSIZE(filetypes), filetypes);

        // File open flags
        DWORD flags;
        m_dlg_SaveFile->GetOptions(&flags);
        m_dlg_SaveFile->SetOptions(flags | FOS_CREATEPROMPT | FOS_NOTESTFILECREATE | FOS_STRICTFILETYPES);

        // Add default extension
    }
}

// after the user has selected a file get its name 
// and save it for future reference
void UserInterface::GetInputFileName(void)
{
    assert(m_dlg_OpenFile);
    if(std::distance(m_Importers.begin(), m_Importers.end()) > 0)
    {
        m_pIView->PrintMessage(UI_MSG_TYPE::CRITICAL_ERROR, L"Only one 3D Mesh at the time.");
        return;
    }
    // First show the Dialog  
    HRESULT hr = m_dlg_OpenFile->Show(m_hwnd);
    if(SUCCEEDED(hr))
    {
        IShellItem* pItem = nullptr;
        m_dlg_OpenFile->GetResult(&pItem);

        if (SUCCEEDED(hr))
        {
            PWSTR szfilepath;
            hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &szfilepath);
            if(SUCCEEDED(hr))
            {
                m_ImportFilename = szfilepath;                          // keep a copy the name and path of the file
                CoTaskMemFree(szfilepath);                              // free the memory of the file path
                SetWindowText(m_hwnd, m_ImportFilename.c_str());        // Set the import-file static edge string
            }

            // 
            IOMesh* pImporter   = new IOMesh(m_ImportFilename, m_pIView);
            const Mesh* pMesh   = pImporter->GetImportedMesh();
            if(pMesh)
            {
                m_Importers.push_front( pImporter );
                if(m_pIView->AddMesh(pMesh))
                {
                    m_materials_name = pImporter->GetMaterialsName();   // Get the names of all the valid materials of the mesh
                    CreateMaterialsEditingScreens();                    // Create the screens so the user can edit the materials
                }
            }
        }
        SAFE_RELEASE_COM(pItem);
    }
    m_frameCount = 0; // Reset the frames count so it won't show some cray ass numbers.
}

void UserInterface::GetOutputFileName(void)
{
    if(std::distance(m_Importers.begin(), m_Importers.end()) == 0)
    {
        m_pIView->PrintMessage(UI_MSG_TYPE::CRITICAL_ERROR, L"Note: There is no 3D Mesh to be exported."); 
        return;
    }
    // First show the Dialog
    HRESULT hr = m_dlg_SaveFile->Show(m_hwnd);
    if(SUCCEEDED(hr))
    {
        IShellItem* pItem = nullptr;
        m_dlg_SaveFile->GetResult(&pItem);

        if (SUCCEEDED(hr))
        {
            PWSTR szfilepath;
            hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &szfilepath);

            if(SUCCEEDED(hr))
            {
                m_ExportFilename = szfilepath;                      // keep a copy the name and path of the file
                CoTaskMemFree(szfilepath);                          // free the memory of the file path
                UINT index = 0;
                hr = m_dlg_SaveFile->GetFileTypeIndex(&index);
                m_Importers.front()->ExportMesh( m_ExportFilename, FILE_FORMATS_ARRAY[index-1]);
            }
        }
        SAFE_RELEASE_COM(pItem);
    }
}

void UserInterface::CreateMainScreen(void)
{
    const float F_TRANSPARENCY_FACTOR   = 0.5f;
    const float F_SCREEN_TRANSPARENCY   = 0.2f;
    const float F_SCREEN_RADIUS         = 15.0f;
    const float F_BUTTON_RADIUS         = 32.0f;
    const float F_BUTTON_DIAMETER       = F_BUTTON_RADIUS * 2;
    const uint  BUTTON_COUNT            = 6;

    D2D1_RECT_F screen;
    screen.left    = 15.0f;
    screen.right   = ceil(m_RenderTargetSize.width  - 15.00f);
    screen.top     = ceil(m_RenderTargetSize.height * 00.89f);
    screen.bottom  = ceil(m_RenderTargetSize.height - 10.00f);

    const D2D1_RECT_F screen_rect = screen;    

    // -----------------|
    // MAIN SCREEN      |
    // -----------------|
    {
        Screen* lpMainScreen                = nullptr;
        ID2D1BitmapBrush* lpBitmapBrush     = nullptr;
        lpMainScreen                        = new Screen();
        lpMainScreen->InitializeScreen(screen_rect.left, screen_rect.right, screen_rect.top,screen_rect.bottom , F_SCREEN_TRANSPARENCY, F_SCREEN_RADIUS, SCREEN::MAIN_SCREEN);

        const float F_WIDTH                 = lpMainScreen->GetWidth();
        const float F_SEPARATION_FACTOR     = (F_WIDTH - (F_BUTTON_DIAMETER * BUTTON_COUNT)) / BUTTON_COUNT;
        const float F_BRUSH_SIZE            = 2.0f;

        const D2D_COLOR_F default_color     = D2D1::ColorF(D2D1::ColorF::LightGray, 1.0f);
        const D2D_COLOR_F hovering_color    = D2D1::ColorF(D2D1::ColorF::White,     1.0f);
        const D2D_COLOR_F clicked_color     = D2D1::ColorF(D2D1::ColorF::LightCyan, 0.6f);
        const D2D_COLOR_F textColor         = D2D1::ColorF(D2D1::ColorF::White,     1.0f);

        float horizontal_separation = 0.0f;
        const float F_VERTICAL_SEPARATION = ceil((lpMainScreen->GetHeight() * .50f) - (FONT_SIZE/2.0f));    // 50% of height (the middle) /////// ADD MORE INFORMATION ON THIS

        // Import/Export Menu
        {
            RoundButton* ImportExportButton = new RoundButton();

            horizontal_separation += F_SEPARATION_FACTOR + F_BUTTON_DIAMETER;

            ImportExportButton->Initialize(float2(screen_rect.left + horizontal_separation, screen_rect.top + F_VERTICAL_SEPARATION), F_BUTTON_RADIUS, UIBUTTON::IMPORTEXPORT, wstring(L"Import/Export"), textColor);

            lpBitmapBrush = CreateBitmapBrush(L"Icons\\ImportExport.png", m_pWicImagingFactory, m_pD2DContext);
            ImportExportButton->SetDefaultStatusBitmap(lpBitmapBrush);
            lpBitmapBrush = nullptr;

            // Effects
            //ImportExportButton->SetOutlineBrushSize(F_BRUSH_SIZE);
            ImportExportButton->SetOutlineBrushSize(4.0f);
            ImportExportButton->SetDefaultEffect(DEFAULT_EFFECT::DRAWOUTLINE_D, default_color);
            ImportExportButton->SetHoveringEffect(HOVERING_EFFECT::DRAWOUTLINE_H, hovering_color );
            ImportExportButton->SetClickedEffect(CLICKED_EFFECT::DRAWMASK_C, clicked_color);

            lpMainScreen->AddUIElement(ImportExportButton);

            // Import/Export screen
            {
                Screen* pImportExportScreen     = nullptr;

                pImportExportScreen = new Screen();
                pImportExportScreen->InitializeScreen(screen_rect.left, screen_rect.right, screen_rect.top, screen_rect.bottom , F_SCREEN_TRANSPARENCY, F_SCREEN_RADIUS, SCREEN::IMPORTEXPORTMESH);

                // Import Mesh Menu (Round-Button)
                {
                    RoundButton* ImportMeshButton = nullptr;
                    ImportMeshButton = new RoundButton();
                    float x = screen_rect.left + (pImportExportScreen->GetWidth()/2.0f/2.0f);
                    float y = screen_rect.top  + (pImportExportScreen->GetHeight()/2.0f) - (96.0f/FONT_SIZE)/2.0f;

                    ImportMeshButton->Initialize(float2(x, y), 32.0f, UIBUTTON::IMPORTMESH, wstring(L"Import Mesh"), textColor);

                    lpBitmapBrush = CreateBitmapBrush(L"Icons\\ImportMesh.png", m_pWicImagingFactory, m_pD2DContext);
                    ImportMeshButton->SetDefaultStatusBitmap(lpBitmapBrush);
                    lpBitmapBrush = nullptr;

                    // Effects
                    ImportMeshButton->SetOutlineBrushSize(F_BRUSH_SIZE);
                    ImportMeshButton->SetDefaultEffect(DEFAULT_EFFECT::DRAWOUTLINE_D, default_color);
                    ImportMeshButton->SetHoveringEffect(HOVERING_EFFECT::DRAWOUTLINE_H, hovering_color );
                    ImportMeshButton->SetClickedEffect(CLICKED_EFFECT::DRAWMASK_C, clicked_color);

                    // Added to ImportExport Screen
                    pImportExportScreen->AddUIElement(ImportMeshButton);
                }

                // Delete Mesh Menu (Round-Button)
                {
                    RoundButton* DelteMesh = nullptr;
                    DelteMesh = new RoundButton();
                    float x = screen_rect.left + (pImportExportScreen->GetWidth()/2.0f);
                    float y = screen_rect.top  + (pImportExportScreen->GetHeight()/2.0f) - (96.0f/FONT_SIZE)/2.0f;

                    DelteMesh->Initialize(float2(x, y), 32.0f, UIBUTTON::DELETEMESH, wstring(L"Delete Mesh"), textColor);

                    lpBitmapBrush = CreateBitmapBrush(L"Icons\\DeleteMesh.png", m_pWicImagingFactory, m_pD2DContext);
                    DelteMesh->SetDefaultStatusBitmap(lpBitmapBrush);
                    lpBitmapBrush = nullptr;

                    // Effects
                    DelteMesh->SetOutlineBrushSize(F_BRUSH_SIZE);
                    DelteMesh->SetDefaultEffect(DEFAULT_EFFECT::DRAWOUTLINE_D, default_color);
                    DelteMesh->SetHoveringEffect(HOVERING_EFFECT::DRAWOUTLINE_H, hovering_color );
                    DelteMesh->SetClickedEffect(CLICKED_EFFECT::DRAWMASK_C, clicked_color);

                    // Added to ImportExport Screen
                    pImportExportScreen->AddUIElement(DelteMesh);
                }

                // Export Mesh Menu (Round-Button)
                {
                    RoundButton* ExportMeshButton = nullptr;
                    ExportMeshButton = new RoundButton();

                    float x = screen_rect.left + ((pImportExportScreen->GetWidth()/2.0f) + (pImportExportScreen->GetWidth()/2.0f/2.0f) );
                    float y = screen_rect.top  + (pImportExportScreen->GetHeight()/2.0f) - (m_dpiY/FONT_SIZE)/2.0f;

                    ExportMeshButton->Initialize(float2(x, y), 32.0f, UIBUTTON::EXPORTMESH, wstring(L"Export Mesh"), textColor);

                    lpBitmapBrush = CreateBitmapBrush(L"Icons\\ExportMesh.png", m_pWicImagingFactory, m_pD2DContext);
                    ExportMeshButton->SetDefaultStatusBitmap(lpBitmapBrush);
                    lpBitmapBrush = nullptr;

                    // Effects
                    ExportMeshButton->SetOutlineBrushSize(F_BRUSH_SIZE);
                    ExportMeshButton->SetDefaultEffect(DEFAULT_EFFECT::DRAWOUTLINE_D, default_color);
                    ExportMeshButton->SetHoveringEffect(HOVERING_EFFECT::DRAWOUTLINE_H, hovering_color );
                    ExportMeshButton->SetClickedEffect(CLICKED_EFFECT::DRAWMASK_C, clicked_color);

                    // Added to ImportExport Screen
                    pImportExportScreen->AddUIElement(ExportMeshButton);
                }

                // Add the screen to the list of screens in the UI
                m_ScreensList.push_front(pImportExportScreen);

                // Set the screen as the button's child screen
                ImportExportButton->SetChildScreen(pImportExportScreen);

                // Set the main screen as the parent to this screen
                pImportExportScreen->SetParent(lpMainScreen);
            }
        }

        // Camera Menu
        {
            RoundButton* CameraMenuButton = new RoundButton();

            horizontal_separation   += F_SEPARATION_FACTOR + F_BUTTON_RADIUS;
            CameraMenuButton->Initialize(float2(screen_rect.left + horizontal_separation, screen_rect.top + F_VERTICAL_SEPARATION), F_BUTTON_RADIUS, UIBUTTON::CAMERAMENU, wstring(L"Camera Menu"), textColor);

            lpBitmapBrush = CreateBitmapBrush(L"Icons\\CameraMenu.png", m_pWicImagingFactory, m_pD2DContext);
            CameraMenuButton->SetDefaultStatusBitmap(lpBitmapBrush);
            lpBitmapBrush = nullptr;

            // Effects
            CameraMenuButton->SetOutlineBrushSize(F_BRUSH_SIZE);
            CameraMenuButton->SetDefaultEffect(DEFAULT_EFFECT::DRAWOUTLINE_D, default_color);
            CameraMenuButton->SetHoveringEffect(HOVERING_EFFECT::DRAWOUTLINE_H, hovering_color );
            CameraMenuButton->SetClickedEffect(CLICKED_EFFECT::DRAWMASK_C, clicked_color);

            lpMainScreen->AddUIElement(CameraMenuButton);

            // Camera menu screen
            {
                Screen* pCameramenuScreen = new Screen();
                pCameramenuScreen->InitializeScreen(screen_rect.left, screen_rect.right, screen_rect.top, screen_rect.bottom , F_SCREEN_TRANSPARENCY, F_SCREEN_RADIUS, SCREEN::CAMERA_SETTINGS);
                const float pos_y = screen_rect.top + pCameramenuScreen->GetHeight() * 1/2 - FONT_SIZE;

                // Setting the static variables for the scroll bars
                lpBitmapBrush = nullptr;
                lpBitmapBrush = CreateBitmapBrush(L"Icons\\LeftScrollBarButton.png", m_pWicImagingFactory, m_pD2DContext);                    
                ScrollBar::SetLeftButtonBrush( lpBitmapBrush );
                lpBitmapBrush = nullptr;
                lpBitmapBrush = CreateBitmapBrush(L"Icons\\RightScrollBarButton.png", m_pWicImagingFactory, m_pD2DContext);                    
                ScrollBar::SetRightButtonBrush( lpBitmapBrush );
                lpBitmapBrush = nullptr;

                // Camera movement speed Scroll-Bar
                {
                    ScrollBar* cameramovementScrollBar = new ScrollBar();
                    const float pos_x = screen_rect.left + pCameramenuScreen->GetWidth() * 1/4 - 85.0f;

                    cameramovementScrollBar->Initialize(pos_x, pos_y, UIBUTTON::CAMERAMOVEMENT, wstring(L"Panning Speed"), m_iCameraMovementSpeed);

                    // Save this in the UserInterface class it will be needed everytime the camera movement speed is changed.
                    m_pCameraMovementSpeedSB = cameramovementScrollBar;

                    // Add it to the Camera settings' screen
                    pCameramenuScreen->AddUIElement( cameramovementScrollBar );

                    // we do this here so the camera speed is updated after the value has been loaded
                    View* pview = reinterpret_cast<View*>(m_pIView);
                    pview->SetCameraMovementSpeedValue(m_iCameraMovementSpeed);
                }

                // Camera zoom speed Scroll-Bar
                {
                    ScrollBar* CameraZoomSpeed = new ScrollBar();
                    const float pos_x = screen_rect.left + pCameramenuScreen->GetWidth()*3/4 - 85.0f;
                    CameraZoomSpeed->Initialize(pos_x, pos_y, UIBUTTON::CAMERAZOOMSPEED, wstring(L"Zoom Speed"), m_iCameraZoomSpeed);

                    // Save this in the UserInterface class it will be needed everytime the camera movement speed is changed.
                    m_pCameraZoomSpeedSB = CameraZoomSpeed;

                    // Add it to the Camera settings' screen
                    pCameramenuScreen->AddUIElement( CameraZoomSpeed );

                    // we do this here so the camera speed is updated after the value has been loaded
                    View* pview = reinterpret_cast<View*>(m_pIView);
                    pview->SetCameraZoomSpeed(m_iCameraZoomSpeed);
                }

                // Set the main screen as the parent of this screen
                pCameramenuScreen->SetParent( lpMainScreen );

                // Set this screen as the child of the Camera-Menu button
                CameraMenuButton->SetChildScreen( pCameramenuScreen );

                // Add this screen to the screens-list variable for proper destruction
                m_ScreensList.push_front( pCameramenuScreen );
            }
        }

        // Environment Menu
        {
            RoundButton* EnvironmentMenuButton  = new RoundButton();

            horizontal_separation += F_SEPARATION_FACTOR + F_BUTTON_RADIUS;
            EnvironmentMenuButton->Initialize(float2(screen_rect.left + horizontal_separation, screen_rect.top + F_VERTICAL_SEPARATION), F_BUTTON_RADIUS, UIBUTTON::ENVIRONMENTMENU, wstring(L"Environment"), textColor);

            lpBitmapBrush = CreateBitmapBrush(L"Icons\\EnvironmentMenu.png", m_pWicImagingFactory, m_pD2DContext);
            EnvironmentMenuButton->SetDefaultStatusBitmap(lpBitmapBrush);
            lpBitmapBrush = nullptr;

            // Effects
            EnvironmentMenuButton->SetOutlineBrushSize(F_BRUSH_SIZE);
            EnvironmentMenuButton->SetDefaultEffect(DEFAULT_EFFECT::DRAWOUTLINE_D, default_color);
            EnvironmentMenuButton->SetHoveringEffect(HOVERING_EFFECT::DRAWOUTLINE_H, hovering_color );
            EnvironmentMenuButton->SetClickedEffect(CLICKED_EFFECT::DRAWMASK_C, clicked_color);

            lpMainScreen->AddUIElement(EnvironmentMenuButton);

            // Environment Menu Child Screen
            {
                Screen* pEnvironmentScreen = nullptr;
                pEnvironmentScreen = new Screen();
                pEnvironmentScreen->InitializeScreen(screen_rect.left, screen_rect.right, screen_rect.top, screen_rect.bottom , F_SCREEN_TRANSPARENCY, F_SCREEN_RADIUS, SCREEN::ENVIRONMENTOPTIONS);

                // Show/Hide World Axes button (Round-Button)
                {
                    RoundButton* ShowHideAxesButton = nullptr;
                    ShowHideAxesButton              = new RoundButton();

                    float x = screen_rect.left + (pEnvironmentScreen->GetWidth()/2.0f - 128.0f);
                    float y = screen_rect.top  + (pEnvironmentScreen->GetHeight()/2.0f) - (96.0f/FONT_SIZE)/2.0f;

                    ShowHideAxesButton->Initialize(float2(x, y), 32.0f, UIBUTTON::AXESONOFF, wstring(L"Axes On/Off"), textColor);

                    lpBitmapBrush = CreateBitmapBrush(L"Icons\\AxesOnOff.png", m_pWicImagingFactory, m_pD2DContext);
                    ShowHideAxesButton->SetDefaultStatusBitmap(lpBitmapBrush);
                    lpBitmapBrush = nullptr;

                    // Effects
                    ShowHideAxesButton->SetOutlineBrushSize(F_BRUSH_SIZE);
                    ShowHideAxesButton->SetDefaultEffect(DEFAULT_EFFECT::DRAWOUTLINE_D, default_color);
                    ShowHideAxesButton->SetHoveringEffect(HOVERING_EFFECT::DRAWOUTLINE_H, hovering_color );
                    ShowHideAxesButton->SetClickedEffect(CLICKED_EFFECT::DRAWMASK_C, clicked_color);
                    ShowHideAxesButton->SetPermanentlyActiveEffect(ACTIVE_EFFECT::DRAWMASK_A, hovering_color);

                    // Set it as active by default
                    ShowHideAxesButton->MouseButtonUp();

                    // Added to ImportExport Screen
                    pEnvironmentScreen->AddUIElement(ShowHideAxesButton);
                }

                // Show/Hide Grid button (Round-Button)
                {
                    RoundButton* ShowHideGridButton = nullptr;
                    ShowHideGridButton              = new RoundButton();

                    float x = screen_rect.left + (pEnvironmentScreen->GetWidth()/2.0f + 128.0f);
                    float y = screen_rect.top  + (pEnvironmentScreen->GetHeight()/2.0f) - (96.0f/FONT_SIZE)/2.0f;

                    ShowHideGridButton->Initialize(float2(x, y), 32.0f, UIBUTTON::GRIDONOFF, wstring(L"Grid On/Off"), textColor);

                    lpBitmapBrush = CreateBitmapBrush(L"Icons\\GridOnOff.png", m_pWicImagingFactory, m_pD2DContext);
                    ShowHideGridButton->SetDefaultStatusBitmap(lpBitmapBrush);
                    lpBitmapBrush = nullptr;

                    // Effects
                    ShowHideGridButton->SetOutlineBrushSize(F_BRUSH_SIZE);
                    ShowHideGridButton->SetDefaultEffect(DEFAULT_EFFECT::DRAWOUTLINE_D, default_color);
                    ShowHideGridButton->SetHoveringEffect(HOVERING_EFFECT::DRAWOUTLINE_H, hovering_color );
                    ShowHideGridButton->SetClickedEffect(CLICKED_EFFECT::DRAWMASK_C, clicked_color);
                    ShowHideGridButton->SetPermanentlyActiveEffect(ACTIVE_EFFECT::DRAWMASK_A, hovering_color);

                    // Set it as active by default
                    ShowHideGridButton->MouseButtonUp();

                    // Added to ImportExport Screen
                    pEnvironmentScreen->AddUIElement(ShowHideGridButton);
                }

                // Add the screen to the list of screens in the UI
                m_ScreensList.push_front(pEnvironmentScreen);

                // Set the main screen as the parent to this screen
                pEnvironmentScreen->SetParent(lpMainScreen);
                // Set the child appropriately
                EnvironmentMenuButton->SetChildScreen(pEnvironmentScreen);
            }
        }

        // Material Menu
        {
            RoundButton* MaterialMenuButton = new RoundButton();

            horizontal_separation += F_SEPARATION_FACTOR + F_BUTTON_RADIUS;
            MaterialMenuButton->Initialize(float2(screen_rect.left + horizontal_separation, screen_rect.top + F_VERTICAL_SEPARATION), F_BUTTON_RADIUS, UIBUTTON::MATERIALMENU, wstring(L"Materials Menu"), textColor);

            lpBitmapBrush = CreateBitmapBrush(L"Icons\\MaterialMenu.png", m_pWicImagingFactory, m_pD2DContext);
            MaterialMenuButton->SetDefaultStatusBitmap(lpBitmapBrush);
            lpBitmapBrush = nullptr;

            // Effects
            MaterialMenuButton->SetOutlineBrushSize(F_BRUSH_SIZE);
            MaterialMenuButton->SetDefaultEffect(DEFAULT_EFFECT::DRAWOUTLINE_D, default_color);
            MaterialMenuButton->SetHoveringEffect(HOVERING_EFFECT::DRAWOUTLINE_H, hovering_color );
            MaterialMenuButton->SetClickedEffect(CLICKED_EFFECT::DRAWMASK_C, clicked_color);

            lpMainScreen->AddUIElement(MaterialMenuButton);

            // Material Editing screen, it will be empty at in the beginning of the program.            
            {
                Screen* lpMaterialScreen = nullptr;
                lpMaterialScreen = new Screen();
                lpMaterialScreen->InitializeScreen(screen_rect.left, screen_rect.right, screen_rect.top, screen_rect.bottom , F_SCREEN_TRANSPARENCY, F_SCREEN_RADIUS, SCREEN::MATERIALEDITING_SCREEN);

                // Add UI Buttons here (NOT NECESSARY FOR THIS SCREEN)

                // Add this screen to the list of screens so the UI is tracking its where-abouts
                m_ScreensList.push_front(lpMaterialScreen);

                // Set the main screen as the parent screen to this screen
                lpMaterialScreen->SetParent(lpMainScreen);

                // save the pointer to this specific screen in case we want to find it very quickly
                m_pMaterialsScreen = lpMaterialScreen;

                // Se the this screen as the child screen when the user clicks the material editing button
                MaterialMenuButton->SetChildScreen(lpMaterialScreen);
            }            
        }

        // Mesh Menu
        {
            RoundButton* MeshMenuButton = new RoundButton();

            horizontal_separation += F_SEPARATION_FACTOR + F_BUTTON_RADIUS;
            MeshMenuButton->Initialize(float2(screen_rect.left + horizontal_separation, screen_rect.top + F_VERTICAL_SEPARATION), F_BUTTON_RADIUS, UIBUTTON::MESHMENU, wstring(L"Mesh Menu"), textColor);

            lpBitmapBrush = CreateBitmapBrush(L"Icons\\MeshMenu.png", m_pWicImagingFactory, m_pD2DContext);
            MeshMenuButton->SetDefaultStatusBitmap(lpBitmapBrush);
            lpBitmapBrush = nullptr;

            // Effects
            MeshMenuButton->SetOutlineBrushSize(F_BRUSH_SIZE);
            MeshMenuButton->SetDefaultEffect(DEFAULT_EFFECT::DRAWOUTLINE_D, default_color);
            MeshMenuButton->SetHoveringEffect(HOVERING_EFFECT::DRAWOUTLINE_H, hovering_color );
            MeshMenuButton->SetClickedEffect(CLICKED_EFFECT::DRAWMASK_C, clicked_color);

            lpMainScreen->AddUIElement(MeshMenuButton);

            // Mesh Menu screen
            {

                Screen* pMeshMenuScreen = new Screen();
                pMeshMenuScreen->InitializeScreen(screen_rect.left, screen_rect.right, screen_rect.top, screen_rect.bottom , F_SCREEN_TRANSPARENCY, F_SCREEN_RADIUS, SCREEN::MESH_SETTINGS);

                const float const_distance = ((pMeshMenuScreen->GetWidth() * .90f ) - (64.0f*4.0f))/4.0f;
                float separation = const_distance;

                // Back Face Culling
                {
                    RoundButton* BackFaceCullingButton = nullptr;
                    BackFaceCullingButton = new RoundButton();

                    separation += 32.0f;
                    float x = screen_rect.left + (pMeshMenuScreen->GetWidth()*.25f);
                    float y = screen_rect.top  + (pMeshMenuScreen->GetHeight()/2.0f - (96.0f/FONT_SIZE)/2.0f);

                    BackFaceCullingButton->Initialize(float2(x, screen_rect.top + F_VERTICAL_SEPARATION), 32.0f, UIBUTTON::BACKFACECULLING, wstring(L"Back-Face Culling"), textColor);

                    lpBitmapBrush = CreateBitmapBrush(L"Icons\\BackFaceCulling.png", m_pWicImagingFactory, m_pD2DContext);
                    BackFaceCullingButton->SetDefaultStatusBitmap(lpBitmapBrush);
                    lpBitmapBrush = nullptr;

                    // Effects
                    BackFaceCullingButton->SetOutlineBrushSize(F_BRUSH_SIZE);
                    BackFaceCullingButton->SetDefaultEffect(DEFAULT_EFFECT::DRAWOUTLINE_D, default_color);
                    BackFaceCullingButton->SetHoveringEffect(HOVERING_EFFECT::DRAWOUTLINE_H, hovering_color );
                    BackFaceCullingButton->SetClickedEffect(CLICKED_EFFECT::DRAWMASK_C, clicked_color);
                    BackFaceCullingButton->SetPermanentlyActiveEffect(ACTIVE_EFFECT::DRAWMASK_A, hovering_color);

                    BackFaceCullingButton->MouseButtonUp(); // It is set by default

                    // Added to ImportExport Screen
                    pMeshMenuScreen->AddUIElement(BackFaceCullingButton);
                }

                // Flip Faces
                {
                    RoundButton* FlipFacesButton = nullptr;
                    FlipFacesButton = new RoundButton();

                    float x = screen_rect.left + (pMeshMenuScreen->GetWidth()/2.0f);
                    float y = screen_rect.top  + (pMeshMenuScreen->GetHeight()/2.0f - (96.0f/FONT_SIZE)/2.0f);

                    FlipFacesButton->Initialize(float2(x, y), 32.0f, UIBUTTON::FLIPFACES, wstring(L"Change Coord-System"), textColor);

                    lpBitmapBrush = CreateBitmapBrush(L"Icons\\FlipFaces.png", m_pWicImagingFactory, m_pD2DContext);
                    FlipFacesButton->SetDefaultStatusBitmap(lpBitmapBrush);
                    lpBitmapBrush = nullptr;

                    // Effects
                    FlipFacesButton->SetOutlineBrushSize(F_BRUSH_SIZE);
                    FlipFacesButton->SetDefaultEffect(DEFAULT_EFFECT::DRAWOUTLINE_D, default_color);
                    FlipFacesButton->SetHoveringEffect(HOVERING_EFFECT::DRAWOUTLINE_H, hovering_color );
                    FlipFacesButton->SetClickedEffect(CLICKED_EFFECT::DRAWMASK_C, clicked_color);

                    // Added to ImportExport Screen
                    pMeshMenuScreen->AddUIElement(FlipFacesButton);
                }

                // Flip UVs only
                {
                    //RoundButton* FlipFacesButton = nullptr;
                    //FlipFacesButton = new RoundButton();

                    //float x = screen_rect.left + (pMeshMenuScreen->GetWidth()/2.0f);
                    //float y = screen_rect.top  + (pMeshMenuScreen->GetHeight()/2.0f - (96.0f/FONT_SIZE)/2.0f);

                    //FlipFacesButton->Initialize(float2(x, y), 32.0f, UIBUTTON::FLIPFACES, wstring(L"Flip Faces"), textColor);

                    //lpBitmapBrush = CreateBitmapBrush(L"Icons\\FlipFaces.png", pFactory, m_pD2DContext);
                    //FlipFacesButton->SetDefaultStatusBitmap(lpBitmapBrush);
                    //lpBitmapBrush = nullptr;

                    //// Effects
                    //FlipFacesButton->SetOutlineBrushSize(F_BRUSH_SIZE);
                    //FlipFacesButton->SetDefaultEffect(DEFAULT_EFFECT::DRAWOUTLINE_D, default_color);
                    //FlipFacesButton->SetHoveringEffect(HOVERING_EFFECT::DRAWOUTLINE_H, hovering_color );
                    //FlipFacesButton->SetClickedEffect(CLICKED_EFFECT::DRAWMASK_C, clicked_color);

                    //// Added to ImportExport Screen
                    //pMeshMenuScreen->AddUIElement(FlipFacesButton);
                }

                // Wireframe Mode
                {
                    RoundButton* WireFrameModeButton = nullptr;
                    WireFrameModeButton = new RoundButton();

                    float x = screen_rect.left + ((pMeshMenuScreen->GetWidth()/2.0f) + (pMeshMenuScreen->GetWidth()*.25f));
                    float y = screen_rect.top  + (pMeshMenuScreen->GetHeight()/2.0f - (96.0f/FONT_SIZE)/2.0f);

                    WireFrameModeButton->Initialize(float2(x, y), 32.0f, UIBUTTON::WIREFRAMEMODE, wstring(L"Wireframe Mode"), textColor);

                    lpBitmapBrush = CreateBitmapBrush(L"Icons\\WireFrameModeOnOff.png", m_pWicImagingFactory, m_pD2DContext);
                    WireFrameModeButton->SetDefaultStatusBitmap(lpBitmapBrush);
                    lpBitmapBrush = nullptr;

                    // Effects
                    WireFrameModeButton->SetOutlineBrushSize(F_BRUSH_SIZE);
                    WireFrameModeButton->SetDefaultEffect(DEFAULT_EFFECT::DRAWOUTLINE_D, default_color);
                    WireFrameModeButton->SetHoveringEffect(HOVERING_EFFECT::DRAWOUTLINE_H, hovering_color );
                    WireFrameModeButton->SetClickedEffect(CLICKED_EFFECT::DRAWMASK_C, clicked_color);
                    WireFrameModeButton->SetPermanentlyActiveEffect(ACTIVE_EFFECT::DRAWMASK_A, hovering_color);

                    // Added to ImportExport Screen
                    pMeshMenuScreen->AddUIElement(WireFrameModeButton);
                }

                // Set this Screen as the child of Mesh Menu button
                MeshMenuButton->SetChildScreen(pMeshMenuScreen);

                // Set the MainScreen as the parent screen
                pMeshMenuScreen->SetParent(lpMainScreen);

                // Add the screen to the Screens' list
                m_ScreensList.push_front(pMeshMenuScreen);
            }

        }

        // ShowHide Context-Menu
        {
            RoundButton* ShoWHideContextButton  = new RoundButton();

            horizontal_separation += F_SEPARATION_FACTOR + F_BUTTON_RADIUS;
            ShoWHideContextButton->Initialize(float2(screen_rect.left + horizontal_separation, screen_rect.top + F_VERTICAL_SEPARATION), F_BUTTON_RADIUS, UIBUTTON::SHOWHIDECONTEXTMENU, wstring(L"Context Menu"), textColor);

            lpBitmapBrush = CreateBitmapBrush(L"Icons\\ShowHideContextMenu.png", m_pWicImagingFactory, m_pD2DContext);
            ShoWHideContextButton->SetDefaultStatusBitmap(lpBitmapBrush);
            lpBitmapBrush = nullptr;

            // Effects
            ShoWHideContextButton->SetOutlineBrushSize(F_BRUSH_SIZE);
            ShoWHideContextButton->SetDefaultEffect(DEFAULT_EFFECT::DRAWOUTLINE_D, default_color);
            ShoWHideContextButton->SetHoveringEffect(HOVERING_EFFECT::DRAWOUTLINE_H, hovering_color );
            ShoWHideContextButton->SetClickedEffect(CLICKED_EFFECT::DRAWMASK_C, clicked_color);
            ShoWHideContextButton->SetPermanentlyActiveEffect(ACTIVE_EFFECT::DRAWMASK_A, hovering_color);

            // Activate the context menu so it is showing from the Application start up
            ShoWHideContextButton->MouseButtonUp();

            lpMainScreen->AddUIElement(ShoWHideContextButton);
        }

        // Final step add the main screen to the screens list and set it as the current screen in the UI
        m_ScreensList.push_front( lpMainScreen );        
        SetCurrentScreen( *lpMainScreen );
    }
}

void UserInterface::CreateContexMenu(void)
{
    const float TRANSPARENCY_FACTOR = 0.5f;

    ID2D1BitmapBrush* lpBitmapBrush = nullptr;
    SquareButton* lpRoundButton      = nullptr;

    uint size = sizeof(RoundButton);

    const D2D_COLOR_F default_color     = D2D1::ColorF(D2D1::ColorF::LightGray, 1.0f);
    const D2D_COLOR_F hovering_color    = D2D1::ColorF(D2D1::ColorF::White, 1.0f);
    const D2D_COLOR_F clicked_color     = D2D1::ColorF(D2D1::ColorF::LightCyan, 0.6f);
    const D2D_COLOR_F textColor         = D2D1::ColorF(D2D1::ColorF::White, 1.0f);
    const D2D_COLOR_F active_color      = D2D1::ColorF(D2D1::ColorF::White, 1.0f);

    // -----------------|
    // Context Menu     |
    // -----------------|
    {
        Screen* lpScreen    = nullptr;
        omi::Rectangle rect;
        rect.mleft      = m_RenderTargetSize.width * .87f;
        rect.mright     = rect.mleft + 50.0f;
        rect.mtop       = m_RenderTargetSize.height * .10f;
        rect.mbottom    = rect.mtop + 250.0f;
        float yoffset   = 250.0f - (250.0f * .968f);

        lpScreen = new Screen();
        lpScreen->InitializeScreen(rect.mleft, rect.mright, rect.mtop, rect.mbottom, 0.2f, 7.0f, SCREEN::CAMERA_SETTINGS);

        const float xoffset     = 5.0f;
        const float yIncrement  = 41 + (((200.0f * .96f) - (40.0f * 4.0f))/4.0f);

        // Light follow the camera
        {
            lpRoundButton = new SquareButton();

            lpRoundButton->Initialize(rect.mleft + xoffset, rect.mtop + yoffset, 40.0f, 40.0f, UIBUTTON::LIGHT_FOLLOW_CAMERA, wstring(L"Light Follow Camera"), textColor);

            lpBitmapBrush = CreateBitmapBrush(L"Icons\\LightFollowCamera.png", m_pWicImagingFactory, m_pD2DContext);
            lpRoundButton->SetDefaultStatusBitmap(lpBitmapBrush);

            lpBitmapBrush = nullptr;

            // Effects
            lpRoundButton->SetOutlineBrushSize(1.0f);
            lpRoundButton->SetDefaultEffect(DEFAULT_EFFECT::DRAWOUTLINE_D, default_color);
            lpRoundButton->SetHoveringEffect(HOVERING_EFFECT::DRAWOUTLINE_H, hovering_color );
            lpRoundButton->SetClickedEffect(CLICKED_EFFECT::DRAWMASK_C, clicked_color);
            lpRoundButton->SetPermanentlyActiveEffect(ACTIVE_EFFECT::DRAWMASK_A, active_color);

            lpScreen->AddUIElement(lpRoundButton);
            lpRoundButton = nullptr;
        }

        // Point the light to where I'm looking
        {
            yoffset += yIncrement;
            lpRoundButton = new SquareButton();

            lpRoundButton->Initialize( 
                rect.mleft  + xoffset,          // left coordinate of the rectangle
                rect.mtop   + yoffset,          // top coordinate of the rectangle
                40.0f,                          // width of the button
                40.0f,                          // height of the button
                UIBUTTON::LIGHT_TO_EYE,         // Button type specifier
                wstring(L"Light To Surface"),   // Button name
                textColor                       // Text color for the button's name
                );

            lpBitmapBrush = CreateBitmapBrush(L"Icons\\LightToSurface.png", m_pWicImagingFactory, m_pD2DContext);
            lpRoundButton->SetDefaultStatusBitmap(lpBitmapBrush);

            lpBitmapBrush = nullptr;

            // Effects
            lpRoundButton->SetOutlineBrushSize(1.0f);
            lpRoundButton->SetDefaultEffect(DEFAULT_EFFECT::DRAWOUTLINE_D, default_color);
            lpRoundButton->SetHoveringEffect(HOVERING_EFFECT::DRAWOUTLINE_H, hovering_color );
            lpRoundButton->SetClickedEffect(CLICKED_EFFECT::DRAWMASK_C, clicked_color);

            lpScreen->AddUIElement(lpRoundButton);
            lpRoundButton = nullptr;
        }

        // Show/Hide Normals
        {
            yoffset += yIncrement;
            lpRoundButton = new SquareButton();

            lpRoundButton->Initialize(rect.mleft + xoffset, rect.mtop + yoffset, 40.0f, 40.0f, UIBUTTON::NORMALSONOFF, wstring(L"Normals On/Off"), textColor);

            lpBitmapBrush = CreateBitmapBrush(L"Icons\\ShowHideNormalsSmall.png", m_pWicImagingFactory, m_pD2DContext);
            lpRoundButton->SetDefaultStatusBitmap(lpBitmapBrush);

            lpBitmapBrush = nullptr;

            // Effects
            lpRoundButton->SetOutlineBrushSize(1.0f);
            lpRoundButton->SetDefaultEffect(DEFAULT_EFFECT::DRAWOUTLINE_D, default_color);
            lpRoundButton->SetHoveringEffect(HOVERING_EFFECT::DRAWOUTLINE_H, hovering_color );
            lpRoundButton->SetClickedEffect(CLICKED_EFFECT::DRAWMASK_C, clicked_color);
            lpRoundButton->SetPermanentlyActiveEffect(ACTIVE_EFFECT::DRAWMASK_A, active_color);

            lpScreen->AddUIElement(lpRoundButton);
            lpRoundButton = nullptr;
        }

        // Texture On/Off
        {
            yoffset += yIncrement;
            lpRoundButton = new SquareButton();

            lpRoundButton->Initialize(rect.mleft + xoffset, rect.mtop + yoffset, 40.0f, 40.0f, UIBUTTON::TEXTURESONOFF, wstring(L"Textures On/Off"), textColor);

            lpBitmapBrush = CreateBitmapBrush(L"Icons\\TexturesOnOff.png", m_pWicImagingFactory, m_pD2DContext);
            lpRoundButton->SetDefaultStatusBitmap(lpBitmapBrush);

            lpBitmapBrush = nullptr;

            // Effects
            lpRoundButton->SetOutlineBrushSize(1.0f);
            lpRoundButton->SetDefaultEffect(DEFAULT_EFFECT::DRAWOUTLINE_D, default_color);
            lpRoundButton->SetHoveringEffect(HOVERING_EFFECT::DRAWOUTLINE_H, hovering_color );
            lpRoundButton->SetClickedEffect(CLICKED_EFFECT::DRAWMASK_C, clicked_color);
            lpRoundButton->SetPermanentlyActiveEffect(ACTIVE_EFFECT::DRAWMASK_A, active_color);

            lpRoundButton->MouseButtonUp(); // Textures are used by default

            lpScreen->AddUIElement(lpRoundButton);
            lpRoundButton = nullptr;
        }

        // Focus On Mesh
        {
            yoffset += yIncrement;
            lpRoundButton = new SquareButton();

            lpRoundButton->Initialize(rect.mleft + xoffset, rect.mtop + yoffset, 40.0f, 40.0f, UIBUTTON::FOCUS_ON_MESH, wstring(L"Focus On Mesh"), textColor);

            lpBitmapBrush = CreateBitmapBrush(L"Icons\\FocusOnMesh.png", m_pWicImagingFactory, m_pD2DContext);
            lpRoundButton->SetDefaultStatusBitmap(lpBitmapBrush);

            lpBitmapBrush = nullptr;

            // Effects
            lpRoundButton->SetOutlineBrushSize(1.0f);
            lpRoundButton->SetDefaultEffect(DEFAULT_EFFECT::DRAWOUTLINE_D, default_color);
            lpRoundButton->SetHoveringEffect(HOVERING_EFFECT::DRAWOUTLINE_H, hovering_color );
            lpRoundButton->SetClickedEffect(CLICKED_EFFECT::DRAWMASK_C, clicked_color);
            lpRoundButton->SetPermanentlyActiveEffect(ACTIVE_EFFECT::DRAWMASK_A, active_color);

            lpRoundButton->MouseButtonUp(); // 3DMive Focuses on the mesh by default so let's turn it on!

            lpScreen->AddUIElement(lpRoundButton);
            lpRoundButton = nullptr;
        }

        m_pContextMenuScreen = lpScreen;

        // Final step add the main screen to the screens list
        m_ScreensList.push_front(lpScreen);
    }

    // --------------------------|
    // Create BACK-ARROW button  |
    // --------------------------|
    lpBitmapBrush = nullptr;
    RoundButton* Back_Arrow = new RoundButton();
    Back_Arrow->Initialize(float2(0.0f, 0.0f), 16.0f, UIBUTTON::BACKARROW, wstring(), D2D1::ColorF(D2D1::ColorF::White, 1.0f)); 

    lpBitmapBrush = CreateBitmapBrush(L"Icons\\BackArrow.png", m_pWicImagingFactory, m_pD2DContext);
    Back_Arrow->SetDefaultStatusBitmap(lpBitmapBrush);   

    // Effects
    Back_Arrow->SetOutlineBrushSize(1.0f);
    Back_Arrow->SetDefaultEffect(DEFAULT_EFFECT::DRAWOUTLINE_D, default_color);
    Back_Arrow->SetHoveringEffect(HOVERING_EFFECT::DRAWOUTLINE_H, hovering_color );
    Back_Arrow->SetClickedEffect(CLICKED_EFFECT::DRAWMASK_C, clicked_color);
    m_pBackArrow = Back_Arrow;
}

void UserInterface::MouseHoveringAt(int x, int y, uint quadrant) 
{
    // Track where the mouse is at all times

    // First check if the mouse is still inside the current screen
    UIBUTTON button = m_pCurrentScreen->MouseHoveringAt(x, y);

    // If it's outside the current screen then release it
    if(button == UIBUTTON::OUTSIDE_SCREEN)        
    {
        // Make the current screen release the mouse in case it was previously inside it but not anymore
        m_pCurrentScreen->ReleaseMouse();

        // if the context menu is not hidden
        if( m_pContextMenuScreen->IsHidden() == false)
        {
            // check if the mouse is hovering inside the context menu screen
            button = m_pContextMenuScreen->MouseHoveringAt(x, y);

            // if it is inside the context menu Capture the Mouse
            // else release the mouse in case it was previously caught/inside the context menu screen            
            if(button != UIBUTTON::OUTSIDE_SCREEN)
            {
                m_pContextMenuScreen->CaptureMouse();
            }
            else
            {
                m_pContextMenuScreen->ReleaseMouse();
            }
        }
    }
    // Else if it still is inside the current screen check
    else
    {
        // first capture the mouse
        m_pCurrentScreen->CaptureMouse();

        // if the mouse is inside the current screen but it is not hovering on any specific button
        if(button == UIBUTTON::NO_BUTTON)
        {
            // then check if it's hovering inside the back button arraow, which doesn't belong to any screen

            // if it is hovering on the back-arrow button then send the button the message
            if(m_pBackArrow->IsInsideElement(x, y))
            {
                m_pBackArrow->MouseHoveringOnMe(true);
            }
            // if it's not then make sure the button still receives a message letting it know in case it was previously tracking the mouse as being on top of it
            else
            {
                m_pBackArrow->MouseHoveringOnMe(false);
            }
        }
    }
}

UIBUTTON UserInterface::MouseClick(int x, int y, uint quadrant) 
{
    UIBUTTON button = UIBUTTON::NO_BUTTON;
    if(m_pCurrentScreen->HasMouseFocus())
    {
        button = m_pCurrentScreen->MouseClicked(x, y);
        if(button == UIBUTTON::NO_BUTTON)
        {
            if(m_pBackArrow->IsInsideElement(x, y))
            {
                m_pBackArrow->MouseClickedMe();
                button = UIBUTTON::BACKARROW;
            }
        }
    }
    else if(m_pContextMenuScreen->HasMouseFocus())
    {
        button = m_pContextMenuScreen->MouseClicked(x, y);
    }
    return button;
}

void UserInterface::SetCurrentScreen(Screen& pScreen)
{
    // Set it as the current screen
    m_pCurrentScreen            = &pScreen;

    const omi::Rectangle rect   = pScreen.GetScreenRect();    
    D2D1_POINT_2F point;
    point.x                     = rect.mleft + 5.0f;
    point.y                     = rect.mtop  + 5.0f;

    m_pBackArrow->RePositionElement(point);
}

void UserInterface::EmptyMeshList(void)
{
    // Delete all the Meshes
    for_each(m_Importers.begin(), m_Importers.end(), [&] (IOMesh* _ptr)
    {
        SAFE_DELETE( _ptr );
    });
    m_Importers.clear();

    // Delete all the Screens
    for_each(m_MaterialScreenList.begin(), m_MaterialScreenList.end(), [&] (Screen* _ptr)
    {
        SAFE_DELETE( _ptr );
    });
    m_MaterialScreenList.clear();

    // Delete UI Buttons from the material editing screen
    m_pMaterialsScreen->DumpUIElements();    
}

int UserInterface::GetCameraSpeedValue(void)
{
    ScrollBar* SB = reinterpret_cast<ScrollBar*> (m_pCameraMovementSpeedSB);
    return SB->GetCurrentValue();
}

int UserInterface::GetCameraZoomSpeed(void)
{
    ScrollBar* ptr = reinterpret_cast<ScrollBar*>(m_pCameraZoomSpeedSB);
    return ptr->GetCurrentValue();
}

void UserInterface::LoadDefaultValues(void)
{
    m_iCameraMovementSpeed  = 10;
    m_iCameraZoomSpeed      = 10;
}

void UserInterface::LoadSettingsFromFile(const char* filebuffer, size_t file_lenght)
{
    size_t current_index = 0;
    while(current_index < file_lenght)
    {
        const SETTING_TOKEN* token = reinterpret_cast<const SETTING_TOKEN*>( &(filebuffer[current_index]) );

        // Process token
        if(*token == SETTING_TOKEN::CAMERAMOVEMENTSPEED_SETTING )
        {
            current_index += sizeof( SETTING_TOKEN );
            const unsigned short* pshort = reinterpret_cast<const unsigned short*>( &(filebuffer[current_index]) );
            m_iCameraMovementSpeed = *pshort;
        }
        else if(*token == SETTING_TOKEN::CAMERAZOOMSPEED_SETTING)
        {
            current_index += sizeof( SETTING_TOKEN );
            const unsigned short* pshort = reinterpret_cast<const unsigned short*>( &(filebuffer[current_index]) );
            m_iCameraZoomSpeed = *pshort;
        }
        // if there is token that is not recognized then we need not to process any more tokens
        else
        {
            current_index = file_lenght;
        }
        current_index += sizeof( unsigned short );
    }
}

void UserInterface::SetCurrentMaterialBeingEdited(const wstring& name)
{
    m_MaterialBeingEdited = &(m_pIView->VGetMaterialProperties(name));
}

void UserInterface::CreateMaterialsEditingScreens(void)
{
    const Screen* lpMainScreen          = nullptr;
    ID2D1BitmapBrush* lpBitmapBrush     = nullptr;

    for_each(m_ScreensList.begin(), m_ScreensList.end(), [&] (Screen* _ptr)
    {
        if(_ptr->GetScreenType() == SCREEN::MAIN_SCREEN)
            lpMainScreen = _ptr;
    });

    const uint BUTTON_COUNT     = std::distance(m_materials_name.begin(), m_materials_name.end()) + 1;

    // Function's global constants
    const float F_TRANSPARENCY_FACTOR   = 0.5f;
    const float F_SCREEN_TRANSPARENCY   = 0.2f;
    const float F_SCREEN_RADIUS         = 15.0f;
    const float F_BUTTON_RADIUS         = 32.0f;
    const float F_BUTTON_DIAMETER       = F_BUTTON_RADIUS * 2;
    const float F_SB_LENGHT             = m_pCameraMovementSpeedSB->GetRadius()*2;

    const omi::Rectangle rect           = m_pMaterialsScreen->GetScreenRect();
    const float F_WIDTH                 = m_pMaterialsScreen->GetWidth();
    const float F_SEPARATION_FACTOR     = (F_WIDTH - (F_BUTTON_DIAMETER * BUTTON_COUNT)) / BUTTON_COUNT;// Calculate the separation factor between the buttons
    const float F_BRUSH_SIZE            = 2.0f;

    const D2D_COLOR_F default_color     = D2D1::ColorF(D2D1::ColorF::LightGray, 1.0f);
    const D2D_COLOR_F hovering_color    = D2D1::ColorF(D2D1::ColorF::White,     1.0f);
    const D2D_COLOR_F clicked_color     = D2D1::ColorF(D2D1::ColorF::LightCyan, 0.6f);
    const D2D_COLOR_F textColor         = D2D1::ColorF(D2D1::ColorF::White,     1.0f);

    D2D1_RECT_F screen_rect;
    screen_rect.left                    = 15.0f;
    screen_rect.right                   = ceil(m_RenderTargetSize.width  - 15.00f);
    screen_rect.top                     = ceil(m_RenderTargetSize.height * 00.89f);
    screen_rect.bottom                  = ceil(m_RenderTargetSize.height - 10.00f);

    float horizontal_separation         = 0.0f;
    const float F_VERTICAL_SEPARATION   = GetPercentage(m_pMaterialsScreen->GetHeight(), 50) - (FONT_SIZE/2.0f);

    for(auto iter_current = m_materials_name.begin(); iter_current != m_materials_name.end(); ++iter_current)
    {
        horizontal_separation += F_SEPARATION_FACTOR;

        if(iter_current == m_materials_name.begin())    // for the first button add the diameter instead of the radius, this separates the screen better
            horizontal_separation += F_BUTTON_DIAMETER;
        else
            horizontal_separation += F_BUTTON_RADIUS;   // for the rest we can just add the radius of the button

        const Material& pMaterial = m_pIView->VGetMaterialProperties(*iter_current);    // Get this material details

        // For each material
        RoundButton* lpRoundButtonUniqueMaterial  = new RoundButton();        

        lpRoundButtonUniqueMaterial->Initialize(float2(screen_rect.left + horizontal_separation, screen_rect.top + F_VERTICAL_SEPARATION), F_BUTTON_RADIUS, UIBUTTON::MATERIAL_EDIT_BUTTON, *iter_current, textColor);

        lpBitmapBrush = CreateBitmapBrush(L"Icons\\MaterialMenu.png", m_pWicImagingFactory, m_pD2DContext);
        lpRoundButtonUniqueMaterial->SetDefaultStatusBitmap(lpBitmapBrush);
        lpBitmapBrush = nullptr;

        // Effects of the material button
        lpRoundButtonUniqueMaterial->SetOutlineBrushSize(F_BRUSH_SIZE);
        lpRoundButtonUniqueMaterial->SetDefaultEffect(DEFAULT_EFFECT::DRAWOUTLINE_D, default_color);
        lpRoundButtonUniqueMaterial->SetHoveringEffect(HOVERING_EFFECT::DRAWOUTLINE_H, hovering_color );
        lpRoundButtonUniqueMaterial->SetClickedEffect(CLICKED_EFFECT::DRAWMASK_C, clicked_color);

        // Add the material's button to the main material-editing screen
        m_pMaterialsScreen->AddUIElement(lpRoundButtonUniqueMaterial);

        // Current material Screen        
        Screen* pUniqueMaterialScreen = new Screen();
        pUniqueMaterialScreen->InitializeScreen(screen_rect.left, screen_rect.right, screen_rect.top, screen_rect.bottom , F_SCREEN_TRANSPARENCY, F_SCREEN_RADIUS, SCREEN::MATERIALS_TERM_SCREEN);
        pUniqueMaterialScreen->SetParent(m_pMaterialsScreen);

        const float lseparation_factor  = (pUniqueMaterialScreen->GetWidth() - (F_BUTTON_DIAMETER * 5))/5;
        float l_horizontal_separation   = lseparation_factor + F_BUTTON_RADIUS;

        /* Each material will have 3 buttons and 1 scroll bar for each material term (Ambien, Diffuse, Specular) and for the alpaha channel respectively */
        {
            // AMBIENT TERM (Round-Button)
            {
                RoundButton* lpAmbientButton = new RoundButton();

                lpAmbientButton->Initialize(float2(screen_rect.left + l_horizontal_separation, screen_rect.top + F_VERTICAL_SEPARATION), F_BUTTON_RADIUS, UIBUTTON::AMBIENT_TERM, wstring(L"Ambient"), textColor);

                lpBitmapBrush = CreateBitmapBrush(L"Icons\\MaterialMenu.png", m_pWicImagingFactory, m_pD2DContext);
                lpAmbientButton->SetDefaultStatusBitmap(lpBitmapBrush);
                lpBitmapBrush = nullptr;

                // Effects
                lpAmbientButton->SetOutlineBrushSize(F_BRUSH_SIZE);
                lpAmbientButton->SetDefaultEffect(DEFAULT_EFFECT::DRAWOUTLINE_D, default_color);
                lpAmbientButton->SetHoveringEffect(HOVERING_EFFECT::DRAWOUTLINE_H, hovering_color );
                lpAmbientButton->SetClickedEffect(CLICKED_EFFECT::DRAWMASK_C, clicked_color);

                const uint SB_COUNT = 4;                
                const float SEPARATION_SB = (pUniqueMaterialScreen->GetWidth() - (F_SB_LENGHT * SB_COUNT))/SB_COUNT;

                float h_separation = SEPARATION_SB * 2;

                // Add the ambient term button to this material's screen
                pUniqueMaterialScreen->AddUIElement(lpAmbientButton);

                // AMBIENT TERM SCREEN - CONSISTS OF 3 COLOR CHANNELS: RED, GREEN, BLUE (RGB)

                Screen* lpMaterialChannelScreen = new Screen();
                lpMaterialChannelScreen->InitializeScreen(screen_rect.left, screen_rect.right, screen_rect.top, screen_rect.bottom , F_SCREEN_TRANSPARENCY, F_SCREEN_RADIUS, SCREEN::MATERIAL_CHANNEL_SCREEN);
                lpMaterialChannelScreen->SetParent(pUniqueMaterialScreen);
                {
                    // Red channel
                    {
                        ScrollBar* lpRedChannelSB = new ScrollBar();
                        const int RED_CHANNEL_VALUE = static_cast<int>(pMaterial.Ambient.x * 100.0);
                        lpRedChannelSB->Initialize(screen_rect.left + h_separation, screen_rect.top + F_VERTICAL_SEPARATION - 5.0f, UIBUTTON::RED_CHANNEL, wstring(L"Red Color"), RED_CHANNEL_VALUE,    0);

                        lpMaterialChannelScreen->AddUIElement(lpRedChannelSB);
                    }

                    h_separation += SEPARATION_SB + F_SB_LENGHT;
                    // Green channel
                    {
                        ScrollBar* lpGreenChannelSB = new ScrollBar();
                        const int GREEN_CHANNEL_VALUE = static_cast<int>(pMaterial.Ambient.y * 100.0);
                        lpGreenChannelSB->Initialize(screen_rect.left + h_separation, screen_rect.top + F_VERTICAL_SEPARATION - 5.0f, UIBUTTON::GREEN_CHANNEL, wstring(L"Green Color"), GREEN_CHANNEL_VALUE, 0);

                        lpMaterialChannelScreen->AddUIElement(lpGreenChannelSB);
                    }

                    h_separation += SEPARATION_SB + F_SB_LENGHT;
                    // Blue channel
                    {
                        ScrollBar* lpBlueChannelSB = new ScrollBar();
                        const int BLUE_CHANNEL_VALUE = static_cast<int>(pMaterial.Ambient.z * 100.0);
                        lpBlueChannelSB->Initialize(screen_rect.left + h_separation, screen_rect.top + F_VERTICAL_SEPARATION - 5.0f, UIBUTTON::BLUE_CHANNEL, wstring(L"Blue Color"), BLUE_CHANNEL_VALUE,  0);

                        lpMaterialChannelScreen->AddUIElement(lpBlueChannelSB);
                    }

                    // Add the screen to the list of screens in the UI
                    m_MaterialScreenList.push_front(lpMaterialChannelScreen);

                    // Set the material's channels screen as the child screen for the material term
                    lpAmbientButton->SetChildScreen(lpMaterialChannelScreen);
                }
            }

            // DIFFUSE TERM (Round button)
            l_horizontal_separation += lseparation_factor + F_BUTTON_RADIUS;
            {
                RoundButton* lpDiffuseButton = new RoundButton();

                lpDiffuseButton->Initialize(float2(screen_rect.left + l_horizontal_separation, screen_rect.top + F_VERTICAL_SEPARATION), F_BUTTON_RADIUS, UIBUTTON::DIFFUSE_TERM, wstring(L"Diffuse"), textColor);

                lpBitmapBrush = CreateBitmapBrush(L"Icons\\MaterialMenu.png", m_pWicImagingFactory, m_pD2DContext);
                lpDiffuseButton->SetDefaultStatusBitmap(lpBitmapBrush);
                lpBitmapBrush = nullptr;

                // Effects
                lpDiffuseButton->SetOutlineBrushSize(F_BRUSH_SIZE);
                lpDiffuseButton->SetDefaultEffect(DEFAULT_EFFECT::DRAWOUTLINE_D, default_color);
                lpDiffuseButton->SetHoveringEffect(HOVERING_EFFECT::DRAWOUTLINE_H, hovering_color );
                lpDiffuseButton->SetClickedEffect(CLICKED_EFFECT::DRAWMASK_C, clicked_color);

                const uint SB_COUNT = 4;                
                const float SEPARATION_SB = (pUniqueMaterialScreen->GetWidth() - (F_SB_LENGHT * SB_COUNT))/SB_COUNT;

                float h_separation = SEPARATION_SB * 2;

                // Add the ambient term button to this material's screen
                pUniqueMaterialScreen->AddUIElement(lpDiffuseButton);

                // AMBIENT TERM SCREEN - CONSISTS OF 3 COLOR CHANNELS: RED, GREEN, BLUE (RGB)

                Screen* lpMaterialChannelScreen = new Screen();
                lpMaterialChannelScreen->InitializeScreen(screen_rect.left, screen_rect.right, screen_rect.top, screen_rect.bottom , F_SCREEN_TRANSPARENCY, F_SCREEN_RADIUS, SCREEN::MATERIAL_CHANNEL_SCREEN);
                lpMaterialChannelScreen->SetParent(pUniqueMaterialScreen);
                {
                    // Red channel
                    {
                        ScrollBar* lpRedChannelSB = new ScrollBar();
                        const int RED_CHANNEL_VALUE = static_cast<int>(pMaterial.Diffuse.x * 100.0);
                        lpRedChannelSB->Initialize(screen_rect.left + h_separation, screen_rect.top + F_VERTICAL_SEPARATION - 5.0f, UIBUTTON::RED_CHANNEL, wstring(L"Red Color"), RED_CHANNEL_VALUE,    0);

                        lpMaterialChannelScreen->AddUIElement(lpRedChannelSB);
                    }

                    h_separation += SEPARATION_SB + F_SB_LENGHT;
                    // Green channel
                    {
                        ScrollBar* lpGreenChannelSB = new ScrollBar();
                        const int GREEN_CHANNEL_VALUE = static_cast<int>(pMaterial.Diffuse.y * 100.0);
                        lpGreenChannelSB->Initialize(screen_rect.left + h_separation, screen_rect.top + F_VERTICAL_SEPARATION - 5.0f, UIBUTTON::GREEN_CHANNEL, wstring(L"Green Color"), GREEN_CHANNEL_VALUE, 0);

                        lpMaterialChannelScreen->AddUIElement(lpGreenChannelSB);
                    }

                    h_separation += SEPARATION_SB + F_SB_LENGHT;
                    // Blue channel
                    {
                        ScrollBar* lpBlueChannelSB = new ScrollBar();
                        const int BLUE_CHANNEL_VALUE = static_cast<int>(pMaterial.Diffuse.z * 100.0);
                        lpBlueChannelSB->Initialize(screen_rect.left + h_separation, screen_rect.top + F_VERTICAL_SEPARATION - 5.0f, UIBUTTON::BLUE_CHANNEL, wstring(L"Blue Color"), BLUE_CHANNEL_VALUE,  0);

                        lpMaterialChannelScreen->AddUIElement(lpBlueChannelSB);
                    }

                    // Add the screen to the list of screens in the UI
                    m_MaterialScreenList.push_front(lpMaterialChannelScreen);

                    // Set the material's channels screen as the child screen for the material term
                    lpDiffuseButton->SetChildScreen(lpMaterialChannelScreen);
                }
            }

            // SPECULAR TERM (Round button)
            l_horizontal_separation += lseparation_factor + F_BUTTON_RADIUS;
            {
                RoundButton* lpSpecularButton = new RoundButton();

                lpSpecularButton->Initialize(float2(screen_rect.left + l_horizontal_separation, screen_rect.top + F_VERTICAL_SEPARATION), F_BUTTON_RADIUS, UIBUTTON::SPECULAR_TERM, wstring(L"Specular"), textColor);

                lpBitmapBrush = CreateBitmapBrush(L"Icons\\MaterialMenu.png", m_pWicImagingFactory, m_pD2DContext);
                lpSpecularButton->SetDefaultStatusBitmap(lpBitmapBrush);
                lpBitmapBrush = nullptr;

                // Effects
                lpSpecularButton->SetOutlineBrushSize(F_BRUSH_SIZE);
                lpSpecularButton->SetDefaultEffect(DEFAULT_EFFECT::DRAWOUTLINE_D, default_color);
                lpSpecularButton->SetHoveringEffect(HOVERING_EFFECT::DRAWOUTLINE_H, hovering_color );
                lpSpecularButton->SetClickedEffect(CLICKED_EFFECT::DRAWMASK_C, clicked_color);

                const uint SB_COUNT = 4;                
                const float SEPARATION_SB = (pUniqueMaterialScreen->GetWidth() - (F_SB_LENGHT * SB_COUNT))/SB_COUNT;

                float h_separation = SEPARATION_SB;

                // Add the ambient term button to this material's screen
                pUniqueMaterialScreen->AddUIElement(lpSpecularButton);

                // AMBIENT TERM SCREEN - CONSISTS OF 3 COLOR CHANNELS: RED, GREEN, BLUE (RGB)

                Screen* lpMaterialChannelScreen = new Screen();
                lpMaterialChannelScreen->InitializeScreen(screen_rect.left, screen_rect.right, screen_rect.top, screen_rect.bottom , F_SCREEN_TRANSPARENCY, F_SCREEN_RADIUS, SCREEN::MATERIAL_CHANNEL_SCREEN);
                lpMaterialChannelScreen->SetParent(pUniqueMaterialScreen);
                {
                    // Red channel
                    {
                        ScrollBar* lpRedChannelSB = new ScrollBar();
                        const int RED_CHANNEL_VALUE = static_cast<int>(pMaterial.Specular.x * 100.0);
                        lpRedChannelSB->Initialize(screen_rect.left + h_separation, screen_rect.top + F_VERTICAL_SEPARATION - 5.0f, UIBUTTON::RED_CHANNEL, wstring(L"Red Color"), RED_CHANNEL_VALUE, 0);

                        lpMaterialChannelScreen->AddUIElement(lpRedChannelSB);
                    }

                    h_separation += SEPARATION_SB + F_SB_LENGHT;
                    // Green channel
                    {
                        ScrollBar* lpGreenChannelSB = new ScrollBar();
                        const int GREEN_CHANNEL_VALUE = static_cast<int>(pMaterial.Specular.y * 100.0);
                        lpGreenChannelSB->Initialize(screen_rect.left + h_separation, screen_rect.top + F_VERTICAL_SEPARATION - 5.0f, UIBUTTON::GREEN_CHANNEL, wstring(L"Green Color"), GREEN_CHANNEL_VALUE,0);

                        lpMaterialChannelScreen->AddUIElement(lpGreenChannelSB);
                    }

                    h_separation += SEPARATION_SB + F_SB_LENGHT;
                    // Blue channel
                    {
                        ScrollBar* lpBlueChannelSB = new ScrollBar();
                        const int BLUE_CHANNEL_VALUE = static_cast<int>(pMaterial.Specular.z * 100.0);
                        lpBlueChannelSB->Initialize(screen_rect.left + h_separation, screen_rect.top + F_VERTICAL_SEPARATION - 5.0f, UIBUTTON::BLUE_CHANNEL, wstring(L"Blue Color"), BLUE_CHANNEL_VALUE, 0);

                        lpMaterialChannelScreen->AddUIElement(lpBlueChannelSB);
                    }

                    // Alpha Channel
                    h_separation += SEPARATION_SB + F_SB_LENGHT;
                    {
                        ScrollBar* lpBlueChannelSB = new ScrollBar();
                        const int SPEC_FACTOR_VALUE = static_cast<int>(pMaterial.Specular.w/4);
                        lpBlueChannelSB->Initialize(screen_rect.left + h_separation, screen_rect.top + F_VERTICAL_SEPARATION - 5.0f, UIBUTTON::SPECULAR_FACTOR, wstring(L"Spec Factor"), SPEC_FACTOR_VALUE, 0, L"*4");

                        lpMaterialChannelScreen->AddUIElement(lpBlueChannelSB);
                    }

                    // Add the screen to the list of screens in the UI
                    m_MaterialScreenList.push_front(lpMaterialChannelScreen);

                    // Set the material's channels screen as the child screen for the material term
                    lpSpecularButton->SetChildScreen(lpMaterialChannelScreen);
                }
            }

            // Alpha channel
            l_horizontal_separation += lseparation_factor + F_BUTTON_RADIUS;
            {
                ScrollBar* lpRedChannelSB = new ScrollBar();
                const int RED_CHANNEL_VALUE = static_cast<int>(pMaterial.Diffuse.w * 100.0);
                lpRedChannelSB->Initialize(screen_rect.left + l_horizontal_separation, screen_rect.top + F_VERTICAL_SEPARATION - 5.0f, UIBUTTON::ALPHA_CHANNEL, wstring(L"Alpha Blend"), RED_CHANNEL_VALUE, 0);

                pUniqueMaterialScreen->AddUIElement(lpRedChannelSB);
            }
            // Add the screen to the list of screens in the UI
            m_MaterialScreenList.push_front(pUniqueMaterialScreen);

            // Set the main screen as the parent to this screen
            pUniqueMaterialScreen->SetParent(m_pMaterialsScreen);

            // Set the material properties as the child screen of the material button
            lpRoundButtonUniqueMaterial->SetChildScreen(pUniqueMaterialScreen);
        }
    }
}

void UserInterface::SaveUserSettingsToFile(void)
{
    PWSTR szUserAppLocal_path;
    HRESULT hr = SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_CREATE, nullptr, &szUserAppLocal_path);
    if(SUCCEEDED(hr))
    {
        wstring wzFile = szUserAppLocal_path;
        wzFile += wsSETTINGS_FILENAME;

        FILE* file  = nullptr;
        unsigned short  value   = 0;

        // Open the file
        _wfopen_s(&file, wzFile.c_str(), L"wb");
        assert(file);
        if(file)
        {
            // Write camera movement speed
            SETTING_TOKEN token = SETTING_TOKEN::CAMERAMOVEMENTSPEED_SETTING;
            value = static_cast<unsigned short>(GetCameraSpeedValue());
            fwrite(&token, sizeof(uint), 1, file);
            fwrite(&value, sizeof(short), 1, file);

            // Write camera zoom speed
            token = SETTING_TOKEN::CAMERAZOOMSPEED_SETTING;
            value = static_cast<unsigned short>(GetCameraZoomSpeed());
            fwrite(&token, sizeof(uint),    1, file);
            fwrite(&value, sizeof(short),   1, file);
        }
        CoTaskMemFree(szUserAppLocal_path);
    }
}

void UserInterface::EditMaterialChannel(const IUIElement* _PSBChannel, UIBUTTON _channel)
{
    assert(m_materials_name.empty() == false);

    assert(
        (m_MaterialTermBeingEdited == UIBUTTON::DIFFUSE_TERM) || 
        (m_MaterialTermBeingEdited == UIBUTTON::AMBIENT_TERM) || 
        (m_MaterialTermBeingEdited == UIBUTTON::SPECULAR_TERM) || 
        (_channel == ALPHA_CHANNEL)
        );

    assert((_channel == RED_CHANNEL) || (_channel == BLUE_CHANNEL) || (_channel == GREEN_CHANNEL) || (_channel == ALPHA_CHANNEL) || (_channel == SPECULAR_FACTOR));

    // Only get a float4 pointer if it's not the alpha channel deing edited
    float4* f4 = nullptr;
    if(_channel != UIBUTTON::ALPHA_CHANNEL)
    {
        switch (m_MaterialTermBeingEdited)
        {
        case AMBIENT_TERM:
            f4 = &(m_MaterialBeingEdited->Ambient);
            break;
        case DIFFUSE_TERM:
            f4 = &(m_MaterialBeingEdited->Diffuse);
            break;
        case SPECULAR_TERM:
            f4 = &(m_MaterialBeingEdited->Specular);
            break;
        }
    }
    else
        f4 = &(m_MaterialBeingEdited->Diffuse);

    // RGB
    float* f = nullptr;
    switch (_channel)
    {
    case UIBUTTON::ALPHA_CHANNEL:
    case UIBUTTON::SPECULAR_FACTOR:
        f = &(f4->w);
        break;
    case UIBUTTON::RED_CHANNEL:
        f = &(f4->x);
        break;
    case UIBUTTON::BLUE_CHANNEL:
        f = &(f4->z);
        break;
    case UIBUTTON::GREEN_CHANNEL:
        f = &(f4->y);
        break;
    }

    const ScrollBar* ptr = reinterpret_cast<const ScrollBar*>(_PSBChannel);
    const int current_value  = ptr->GetCurrentValue();
    if(_channel == UIBUTTON::SPECULAR_FACTOR)
    {
        *f = static_cast<float>((4 * current_value));
    }
    else
    {
        *f = static_cast<float>(current_value/100.0f);
    }
}