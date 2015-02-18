#if defined(_WINDOWS) || defined(WINDOWS)
#pragma once
#endif

#ifndef IVIEW_H
#define IVIEW_H

#include "Mesh.h"

enum UI_MSG_TYPE {CRITICAL_ERROR, WARNING, INFORMATION, STATS };

class IView
{
public:
    IView(void) { }
    virtual ~IView(void) { }
    inline virtual void RenderScene(void) = 0;
    virtual void PrintMessage(UI_MSG_TYPE type, const wchar_t* message) = 0;

    virtual bool AddMesh(const Mesh* M) = 0;

    inline virtual void ZoomIn(void) = 0;
    inline virtual void ZoomOut(void) = 0;
    inline virtual void MoveCamera(int vertically, int horizontally) = 0;
    inline virtual void MoveLight(int vertically, int horizontally) = 0;
    virtual void VExportTextures(const wstring& _in_PathNameFormat, const wstring& _out_Fullpath, const wstring& _out_Format) = 0;
    virtual omi::Material& VGetMaterialProperties(const wstring& _materialName) = 0;
private:
    explicit IView(const IView& v) { } // private copy constructor
};

#endif