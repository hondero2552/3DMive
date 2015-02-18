#if defined(WINDOWS) || defined(_WINDOWS)
#pragma once
#endif

#ifndef VIEW_H
#define VIEW_H

#if defined(WINDOWS) || defined(_WINDOWS)
#include "RendererDX.h"
#include <Shlobj.h>// what is this for? We need comments here
#include <ShObjIdl.h>// what is this for? We need comments here
#endif

#include "IView.h"
#include "timer.h"
#include "User Interface.h"

typedef float (*FUNCTION_PTR)(int);
enum SCALE { METERS, FEET, INCHES, CM, MM };

class View : public IView
{
    double m_timerFrequency;
    int m_fps;
private:
    UserInterface* m_pUserInterface;
    bool m_UIVisible;

#if defined(WINDOWS) || defined(_WINDOWS)
    RendererDx* m_pRenderer;
    HWND m_Hwnd;
#if defined(DEBUG) || defined(_DEBUG)
    IDXGIDebug* m_DebugDevice;
#endif
#else
    /*none Windows systems*/
#endif
    void CheckForUpdates(void);
public:
    View(void);
    ~View(void);
    void RenderScene(void);
    inline void SetTimerFrequency(double freq) { m_timerFrequency = freq;}
    void VExportTextures(const wstring& _in_PathNameFormat, const wstring& _out_Fullpath, const wstring& _out_Format);
    omi::Material& VGetMaterialProperties(const wstring& _materialName) { return m_pRenderer->GetMaterial(_materialName); }

    bool AddMesh(const Mesh* M);
    void PrintMessage(UI_MSG_TYPE type, const wchar_t* msg);

    inline void ZoomIn(void) { m_pRenderer->ZoomIn(); }
    inline void ZoomOut(void) { m_pRenderer->ZoomOut(); }

    inline void MoveCamera(int vertically, int horizontally) { m_pRenderer->MoveCamera(vertically, horizontally); }
    inline void MoveLight(int vertically, int horizontally) { m_pRenderer->MoveLight(vertically, horizontally); }

    bool InitDevices(void);
    bool CreateRenderTargets(const HWND& hwnd, int width, int height);
    inline float2  GetDPI(void) { return m_pUserInterface->GetDPI(); }

    inline void MouseHoveringAt(int x, int y, uint quadrant) { m_pUserInterface->MouseHoveringAt(x, y, quadrant); }
    inline UIBUTTON MouseClick(int x, int y, uint quadrant) { return m_pUserInterface->MouseClick(x, y, quadrant); }
    inline UIBUTTON MouseButtonUp(int x, int y, uint quadrant) { return m_pUserInterface->MouseButtonUp(x, y, quadrant); }

    inline void ShowNormals(void) { m_pRenderer->ShowNormals(); }
    inline void ShowGrid(void) { m_pRenderer->ShowWorldGrid(); }
    inline void ShowAxes(void) { m_pRenderer->ShowWorldAxes(); }

    inline void WireFrameMode(void) { m_pRenderer->WireFrameMode(); }
    inline void BackFaceCulling(void) { m_pRenderer->BackFaceCulling(); }
    inline void TexturesOnOff(void) { m_pRenderer->TexturesOnOff(); }
    inline void DeleteMesh(void) { m_pRenderer->DeleteMesh(); }

    inline int GetCameraMovementSpeedValue(void) { return m_pUserInterface->GetCameraSpeedValue(); }
    inline int GetCameraZoomSpeed(void) { return m_pUserInterface->GetCameraZoomSpeed(); }

    inline void LightFollowCamera(void) { m_pRenderer->LightFollowCamera(); }
    inline void StopFollowingCamera(void) { m_pRenderer->StopFollowingCamera(); }

    inline void SetCameraMovementSpeedValue(int value) { m_pRenderer->SetCameraMovementSpeed(value); }    
    inline void SetCameraZoomSpeed(int value) { m_pRenderer->SetCameraZoomSpeed(value); }

    inline uint GetTriangleCount(void) { return m_pRenderer->GetTriangleCount(); }
    inline void SetTriangleCount(uint count) { m_pUserInterface->SetTraingleCount(count); }
    inline void SetLookAtPoint(const float4& _lookAt) { m_pRenderer->SetLookAtPoint(_lookAt); }
    inline void FocusOnMesh(void) { m_pRenderer->FocusOnMesh(); }
    void SetAABB(void);
    const Mesh* FlipFaces(void);

    bool IsMeshFlipable(void) const;
};

#endif