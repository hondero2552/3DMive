#if defined(_WINDOWS) || defined (WINDOWS)
#pragma once
#endif

//#include <objbase.h>
#include <sstream>
#include "IView.h"
#include "DebugDX.h"

#include "math_funcs.h"
#include "Quadrant.h"
#include "common_data_attributes.h"
#include "Screen.h"
#include "Importer.h"

static const wchar_t* wsSETTINGS_FILENAME = L"\\3DMiveSettings.config";
const float UI_STATS_FONT_SIZE = 14.0f;
using namespace omi;
enum SETTING_TOKEN { CAMERAMOVEMENTSPEED_SETTING = 0x11111111, CAMERAZOOMSPEED_SETTING};
typedef forward_list<Screen*> ScreensList;

class UserInterface
{
    // These are used to quickly access certain UI buttons that need to be called directly, i.e. Scroll-Bars
    IUIElement* m_pCameraMovementSpeedSB;
    IUIElement* m_pCameraZoomSpeedSB;   
    
    ID2D1Factory1*          m_pD2DFactory;
    ID2D1Device*            m_pD2DDevice;
    ID2D1DeviceContext*     m_pD2DContext;
    ID2D1Bitmap1*           m_pD2DTargetBitmap;
    ID2D1SolidColorBrush*   m_pTextBrush;
    ComPtr<ID2D1SolidColorBrush> m_pTextBrushRed;
    IDWriteTextFormat*      m_pTextFormat;
    IDWriteFactory1*        m_pDWriteFactory;
    
    ComPtr<IDWriteTextLayout> m_pTextLayoutFPS;
    ComPtr<IDWriteTextLayout> m_pTextLayoutPotentialFPS;
    ComPtr<IDWriteTextLayout> m_pTextLayoutCorrdinateSystem;
    ComPtr<IDWriteTextLayout> m_pTextLayoutTriangleCount;
    ComPtr<IDWriteTextLayout> m_pTextLayoutMeshLength;
    ComPtr<IDWriteTextLayout> m_pTextLayoutMeshWidth;
    ComPtr<IDWriteTextLayout> m_pTextLayoutMeshHeight;

    IWICImagingFactory*  m_pWicImagingFactory;
 
    forward_list<IOMesh*> m_Importers;
    forward_list<wstring> m_materials_name;

    Screen* m_pCurrentScreen;
    Screen* m_pContextMenuScreen;
    Screen* m_pMaterialsScreen;
    IUIElement* m_pBackArrow;
    ScreensList m_ScreensList;
    ScreensList m_MaterialScreenList;
    
    D2D1_SIZE_F         m_RenderTargetSize;
    
    wstring m_ImportFilename;
    wstring m_ExportFilename;
    IView*  m_pIView;
    HWND    m_hwnd;    

    IFileDialog* m_dlg_OpenFile;
    IFileDialog* m_dlg_SaveFile;
    
    // Timer
    int     m_iPotentialFPS;
    int     m_iCurrentFPS;
    int     m_frameCount;  
    
    // DPI
    float   m_dpiX;
    float   m_dpiY;    

    bool    m_bMouseIsCaptured;

    int     m_iWidth, m_iHeight;
    bool    m_bVisible;

    // Camera movement
    int     m_iCameraMovementSpeed, m_iCameraZoomSpeed;
    
    uint    m_iTriangleCount;

    Material*   m_MaterialBeingEdited;
    UIBUTTON    m_MaterialTermBeingEdited;

public:
    UserInterface(void);
    ~UserInterface(void);
    
    // Potential FPS is how many frames per second could be rendered if the application wasn't 
    // limiting the FPS to approximately 60 fps
    void SetPotentialFPS(int potentialFPS);
    void SetCurrentFPS(int fps);
    
    // Initializes the device and context: ID2DDevice & ID2DContext
    bool InitD2DDevices(IView* pview, IDXGIDevice2* d3dDevice);

    // Once the device and context have been created we need to bind
    // the Direct2D render target with the Direct3D back buffer
    bool InitRenderTarget(const HWND& hwnd, IDXGISwapChain1* pSwapChain);

    // This is where the UIElements will be initialize, Buttons, menus, file dialogs, etc...
    bool InitDeviceDepentResources(void);

    // This is to keep track of size changes on the swapchain's back buffer and also
    // to divice the render target into quadrants for UI input and processing
    void SetRenderTargetSize(int width, int height);

    // This funciton is called every time the UI needs to be rendered, i.e. 60 fps
    // it also updates the frame statistics
    void DrawScene(void);

    const Mesh* FlipFaces(void) { return (m_Importers.front()->FlipMeshFaces()); }

    // This function is called for when the Windows' window is being created the DPI of the screen 
    // can be properly used to size the window
    float2 GetDPI(void);
    
    void SetCurrentScreen(Screen& pScreen);
    void SetTraingleCount(uint count);
    //
    void MouseHoveringAt(int x, int y);
    UIBUTTON MouseClickAt(int x, int y);
    UIBUTTON MouseButtonUp(int x, int y);
    
    int GetCameraSpeedValue(void);
    int GetCameraZoomSpeed(void);
    
    bool IsFlipable(void) const { return m_Importers.front()->isMeshFlipable(); }
    void SetMeshAABB(const AABB& _aabb);
private:
    // Loads the default values for the application settings, i.e. camera movement/zoom speeds
    void LoadDefaultValues(void);

    // loads the settings after the user has used the program at least 1 time
    void LoadSettingsFromFile(const char* filebuffer, size_t file_lenght);

    // Saves the settings the current settings for the user so they will be available
    // next time the application is used under the same machine
    void SaveUserSettingsToFile(void);
    
    // Get rid of the current Mesh
    void EmptyMeshList(void);

    // This function is called from InitRenderTarget() and it initializes the Common-Dialog Boxes
    void CreateDialogBoxes(void);

    // Gets the file name of the file to import. This prompts the user with a OpenFile 
    // dialog so the user can find the file and select a supported file from a list
    void GetInputFileName(void);
    
    // Gets the file name of the file to export the mesh to. This prompts the user with a SaveFile 
    // dialog so the user can find the file and select a supported file from a list 
    void GetOutputFileName(void);

    void SetCurrentMaterialBeingEdited(const wstring& name);
    void CreateMaterialsEditingScreens(void);
    void EditMaterialChannel(const IUIElement* _pSBChannel, UIBUTTON _channel);
    // Private copy conpstructor since we do not want the UI to be constructed from
    // another UI by mistake. If tried this will raise a compilation flag.
    UserInterface(const UserInterface& UI) : m_MaterialBeingEdited(nullptr){ }

    void CreateMainScreen(void);
    void CreateContexMenu(void);    
};