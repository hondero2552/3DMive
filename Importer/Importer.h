#if defined(_WINDOWS) || defined(WINDOWS)
#pragma once
#endif

#ifndef IOMESH_H
#define IOMESH_H
#include "Mesh.h"
#include "IView.h"
#include <string>
using std::wstring;


class IOMesh
{
private:
    IOMesh(IOMesh& I) { }
    Mesh * m_pMesh;
    void * m_pImporter;
    FILE_FORMAT m_type;

public:
    IOMesh(const wstring& input_filename, IView* view);
    ~IOMesh(void);
    
    const Mesh* const GetImportedMesh(void) const { return m_pMesh; }
    
    void * GetImporter(void) { return m_pImporter; }
    const Mesh* FlipMeshFaces(void);
    void FlipUVsONLY(void);
    
    FILE_FORMAT GetFormatEnum(void) const { return m_type; }

    bool isMeshFlipable(void) const;
    void ExportMesh(const wstring& file, FILE_FORMAT format);
    forward_list<wstring> GetMaterialsName(void);
    Material& GetMaterialProperties(const wstring& name);
};

#endif