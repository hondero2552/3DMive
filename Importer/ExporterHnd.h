#pragma once

#include "HndHeader.h"
#include "ExporterBase.h"

class HNDExpoter : public ExporterBase
{
public:
    HNDExpoter(void) { }
    ~HNDExpoter(void) { }

    void VExportMeshToFile(const Mesh& M, const wstring& exportFile);

private:
    void VWriteVertexData(void) const;
    void VWriteIndexData(void) const;
    void VWriteHeader(void) const;
    void VWriteMaterial(void) const;
    void VWriteTextureData(void) const;
};
