#include "Importer.h"

// Loaders
//#include "FbxLoader.h"
#include "OBJIO.h"
//#include "HNDLoader.h"


IOMesh::IOMesh(const wstring& input_filename, IView* pview) : m_pMesh(nullptr), m_pImporter(nullptr)
{
    Mesh* temp = nullptr;
    const FILE_FORMAT format = GetFileFormatEnum(input_filename);

    switch(format)
    {
    case FILE_FORMAT::FBX:
        {
            pview->PrintMessage(UI_MSG_TYPE::CRITICAL_ERROR, L"FBX Meshes are not supported yet.");
            /*
            FBXMeshIO* pfbxloader   = new FBXMeshIO();
            temp                    = pfbxloader->VProcessfile(input_filename, pview);
            m_pImporter             = pfbxloader;
            m_type                  = FILE_FORMAT::FBX;*/
        }
        break;
    case FILE_FORMAT::OBJ:
        {
            OBJMeshIO* pObjLoader   = new OBJMeshIO();
            temp                    = pObjLoader->VProcessfile(input_filename, pview);
            m_pImporter             = pObjLoader;
            m_type                  = FILE_FORMAT::OBJ;
        }
        break;
    case FILE_FORMAT::HND:
        {
            pview->PrintMessage(UI_MSG_TYPE::CRITICAL_ERROR, L"FBX Meshes are not supported yet.");
            /*HNDLoader* loader   = new HNDLoader();
            temp                = loader->VProcessfile(input_filename, pview);
            m_pImporter         = loader;
            m_type              = FILE_FORMAT::HND;*/
        }
    default:
        break;
    }

    // If the mesh was successfully loaded then set it as the variable
    if(temp)
    {
        m_pMesh = temp;
    }
}

IOMesh::~IOMesh(void) 
{
    SAFE_DELETE( m_pMesh );
    switch (m_type)
    {
    case OBJ:
        {
            OBJMeshIO* ptr = reinterpret_cast<OBJMeshIO*>(m_pImporter);
            SAFE_DELETE(ptr);
        }
        break;
    case OBJE:
        break;
    case HND:
        break;
    case FBX:
        {
            // FBXMeshIO* ptr = reinterpret_cast<FBXMeshIO*>(m_pImporter);
            // SAFE_DELETE( ptr );
        }
        break;
    case FORMAT_UNKNOWN:
        break;
    default:
        break;
    }    
}

const Mesh* IOMesh::FlipMeshFaces(void)
{
    // First delete the mesh that is saved preventing memory leaks
    SAFE_DELETE(m_pMesh);

    // Then flip the faces 
    switch (m_type)
    {
    case OBJ:
        {
            OBJMeshIO* ptr  = reinterpret_cast<OBJMeshIO*>(m_pImporter);
            m_pMesh         = ptr->FlipFaces();
        }
        break;
    case OBJE:
        break;
    case HND:
        break;
    case FBX:
        {
            //  FBXMeshIO* ptr  = reinterpret_cast<FBXMeshIO*>(m_pImporter);
            //  m_pMesh         = ptr->FlipFaces();
        }
        break;
    case FORMAT_UNKNOWN:
        break;
    default:
        break;
    }

    // then return the newly created Mesh to be added
    return m_pMesh;
}

void IOMesh::FlipUVsONLY(void)
{

}

bool IOMesh::isMeshFlipable(void) const
{
    MeshIOBase* ptr = reinterpret_cast<MeshIOBase*>(m_pImporter);
    return ptr->isFlipable();
}

void IOMesh::ExportMesh(const wstring& file, FILE_FORMAT filetype)
{
    MeshIOBase* ptr = reinterpret_cast<MeshIOBase*>(m_pImporter);
    ptr->ExportMexhToFile(file, filetype);
}

forward_list<wstring> IOMesh::GetMaterialsName(void)
{
    assert(m_pMesh);

    const MeshIOBase* ptr = reinterpret_cast<MeshIOBase*>(m_pImporter);
    return ptr->GetMaterialNames();
}

Material& IOMesh::GetMaterialProperties(const wstring& name)
{
    assert(m_pMesh);

    MeshIOBase* ptr = reinterpret_cast<MeshIOBase*>(m_pImporter);
    return ptr->GetMaterialProperties(name);
}