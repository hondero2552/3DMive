#pragma once
#include <string>
#include <cstdio>
using std::wstring;
using std::string;

#include "Mesh.h"
using namespace omi;


class IExporterBase
{
public:
    IExporterBase(void) { }
    ~IExporterBase(void) { }

    virtual void VExportMeshToFile(const Mesh& M, const wstring& exportFile) = 0;

protected:    
    virtual void VWriteVertexData(void) const   = 0;
    virtual void VWriteIndexData(void) const    = 0;
    virtual void VWriteHeader(void) const       = 0;
    virtual void VWriteMaterial(void) const     = 0;
    virtual void VWriteTextureData(void) const  = 0;

private:
    IExporterBase(IExporterBase& I) { }
};

class ExporterBase : public IExporterBase
{
protected:
    Mesh* m_pMesh;
    FILE* m_pOutputFile;
public:
    ExporterBase(void) : m_pMesh(nullptr), m_pOutputFile(nullptr) { }
    ~ExporterBase(void) { m_pMesh = nullptr; }
};

class IExporter
{
public:
    IExporter(void) { }
    virtual ~IExporter(void) { }

protected:
    virtual void ExportFile(const Mesh& M, const wstring& output_filename) = 0;

private:
    IExporter(IExporter& I) { }
};