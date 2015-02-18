#pragma once
#include "MeshIOBase.h"
#include "HndHeader.h"

class HNDLoader : public MeshIOBase
{
public:
    HNDLoader(void) { }
    ~HNDLoader(void) { }

    Mesh* VProcessfile(const wstring& inputFile, IView* pIView);
    void VExportMeshToFile(const wstring& file) { }
};