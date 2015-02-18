#include "FbxLoader.h"

FBXMeshIO::FBXMeshIO(void) : m_FbxManger(nullptr) 
{

}

FBXMeshIO::~FBXMeshIO(void) 
{
    m_FbxManger->Destroy();
}

void FBXMeshIO::ValidateNormalsAndUVs(FbxMesh* pMesh)
{
    m_NormalsExist  = pMesh->GetElementNormalCount() > 0;
    m_UVsExist      = pMesh->GetElementUV() > 0; 

    // This is to track if data is per control point(i.e. normals and UVs are per control point
    // regardless of how many polygons share the same control point
    m_AllDataIsPerVertex = true; 

    // validate normals data before it is copied into memory
    if(m_NormalsExist)
    {
        FbxLayerElement::EMappingMode normalsMappMode = pMesh->GetElementNormal()->GetMappingMode();

        // Check if the the mapping mode is defined
        if(normalsMappMode == FbxLayerElement::EMappingMode::eNone)
        {
            m_NormalsExist = false;
        }
        // if mapping mode is defined and it is per control point for every polygon that shares a control point
        // set the variable to false
        if(m_NormalsExist && normalsMappMode != FbxLayerElement::EMappingMode::eByControlPoint)
        {
            m_AllDataIsPerVertex = false;
        }
    }
    // validate UV data before it is copied into the or file
    if(m_UVsExist)
    {
        FbxLayerElement::EMappingMode UVsMappMode = pMesh->GetElementUV()->GetMappingMode();
        // Check if the the mapping mode is defined
        if(UVsMappMode == FbxLayerElement::EMappingMode::eNone)
        {
            m_UVsExist = false;
        }
        // if mapping mode is defined and it is per control point for every polygon that shares a control point
        // set the appropriate variable to false
        if(m_UVsExist && UVsMappMode != FbxLayerElement::EMappingMode::eByControlPoint)
        {
            m_AllDataIsPerVertex = false;
        }
    }
}

FbxScene* FBXMeshIO::InitializeFBXSdk(void) const
{
    // get the file name in ASCII format
    std::string filename(m_InputFileFullPath.begin(), m_InputFileFullPath.end());

    // I/O Settings
    FbxIOSettings* pIosettings = FbxIOSettings::Create(m_FbxManger, IOSROOT);
    m_FbxManger->SetIOSettings(pIosettings);

    // IOMesh
    FbxImporter* pImporter = FbxImporter::Create(m_FbxManger, "");    
    if(pImporter->Initialize(filename.c_str(), -1, pIosettings))
    {
        // Scene
        FbxScene* pScene = FbxScene::Create(m_FbxManger,"Scene");
        bool bImported = pImporter->Import(pScene);
        pImporter->Destroy();

        if (bImported)
        {
            // triangulate the scene
            FbxGeometryConverter mesh_converter(m_FbxManger);
            mesh_converter.Triangulate(pScene, true);

            // Split meshes per material, so that we only have one material per mesh
            mesh_converter.SplitMeshesPerMaterial(pScene, true);
            return pScene;
        }
    }
    return nullptr;
}

Mesh* FBXMeshIO::VProcessfile(const wstring& inputfile, IView* pIView)
{
    m_pIView = pIView;
    ExtractFileSNameDirectoryAndFormat(inputfile);
    m_format = FILE_FORMAT::FBX;

    // create FBX Manager
    m_FbxManger = FbxManager::Create();
    if (!m_FbxManger)
        return nullptr; //// ADD UI ERROR MESSAGE

    FbxScene* pScene = InitializeFBXSdk();

    if(!pScene)
        return nullptr;

    // Set the appropiate flag so the normals and textures can be re-computed
    if(pScene->GetGlobalSettings().GetAxisSystem().GetCoorSystem() == FbxAxisSystem::ECoordSystem::eRightHanded)
        m_windingCCW = true;

    FbxMesh * pMesh = pScene->GetRootNode()->GetChild(0)->GetMesh();
    if(!pMesh)
        return nullptr;     // Add UI code here to return an error due to no mesh availability

    // if the mesh is not composed of triangles ONLY return
    if(!pMesh->IsTriangleMesh())
        return false;

    // check if uvs or normals exist
    ValidateNormalsAndUVs(pMesh);

    if(!CopyVerticesPositions(pMesh))
        return nullptr;

    if(!CopyVerticesNormals(pMesh))
        return nullptr;

    if(!CopyVerticesUVs(pMesh))
        return nullptr;

    if(!GetTexturesInfo(pMesh))
        return nullptr;

    if(!GetSurfaceMaterial(pMesh))
        return nullptr;

    return FlipFaces();
}

bool FBXMeshIO::CopyVerticesPositions(const FbxMesh* pMesh)
{
    // copy vertex positions
    const size_t vertex_count   = pMesh->GetControlPointsCount();
    m_original_positions.reserve(vertex_count);                                                                                         // ADD CATCH EXCEPTION CODE
    for(uint index = 0; index < vertex_count; ++index)
    {
        const FbxVector4 lValue = pMesh->GetControlPointAt(index);
        const double3 vertex_value(lValue[0], lValue[1], lValue[2]); // FOR NOW WE WILL FLIP THE Z COORDINATE FOR DIRECTX
        m_original_positions.push_back(vertex_value);
    }

    // We also copy the Indices
    const uint lPolygonCount    = pMesh->GetPolygonCount();
    const bool byPolygonVertex  = pMesh->GetElementUV()->GetMappingMode() == FbxLayerElement::EMappingMode::eByPolygonVertex ? true : false;
    m_polygon_indices.reserve(lPolygonCount*3);// three indices per polyong face
    for (uint lPolygonIndex     = 0; lPolygonIndex < lPolygonCount; ++lPolygonIndex)
    {
        for (uint lVerticeIndex = 0; lVerticeIndex < TRIANGLE_VERTEX_COUNT; ++lVerticeIndex)
        {
            POS_NORMAL_UV face;
            face.vertex_index   = pMesh->GetPolygonVertex(lPolygonIndex, lVerticeIndex);
            face.normal_index   = face.vertex_index;
            face.uv_index       = byPolygonVertex ? const_cast<FbxMesh *>(pMesh)->GetTextureUVIndex(lPolygonIndex, lVerticeIndex) : face.vertex_index;  // This functions only works for PerPolygonVertex
            m_polygon_indices.push_back(face);
        }
    }
    return true;
}

bool FBXMeshIO::CopyVerticesNormals(const FbxMesh* pMesh)
{    
    // copy normals
    if(m_NormalsExist)
    {
        const FbxLayerElementNormal* NormalElement      = pMesh->GetElementNormal();
        const uint normalcount                          = NormalElement->GetDirectArray().GetCount();
        FbxArray<FbxVector4> normals;
        pMesh->GetPolygonVertexNormals(normals);
        m_original_normals.reserve(normalcount);                                                                                                               // ADD CATCH EXCEPTION CODE
        
        for(uint i = 0; i < normalcount; ++i)
        {
            const double3 lNormal(normals[i][0], normals[i][1], normals[i][2]);
            m_original_normals.push_back(lNormal);
        }
    }
    return true;
}

bool FBXMeshIO::CopyVerticesUVs(const FbxMesh* pMesh)
{
    // copy UVs
    const FbxLayerElementUV* UVElement = pMesh->GetElementUV();
    const FbxLayerElement::EMappingMode mapmode = UVElement->GetMappingMode();
    const bool bIndirectRef = UVElement->GetReferenceMode() == FbxLayerElement::EReferenceMode::eIndexToDirect? true : false;
    const uint uvcount = m_UVsExist ? UVElement->GetDirectArray().GetCount() : 0;

    m_original_uvs.reserve(uvcount);                                // ADD CATCH EXCEPTION CODE
    for(uint index = 0; index < uvcount; ++index)
    {
        uint lUVIndex = index;
        // Get the actual UV Value and push it into the temp_uvs
        const FbxVector2 UV = UVElement->GetDirectArray().GetAt(lUVIndex);
        const double2 UV_Value(UV[0], UV[1]);
        m_original_uvs.push_back(UV_Value);
    }

    return true;
}

bool FBXMeshIO::GetTexturesInfo(FbxMesh* pMesh)
{
    const uint texture_count = pMesh->GetNode()->GetSrcObjectCount<FbxTexture>();
    for(uint index = 0; index < texture_count; ++index)
    {
        FbxTexture* pTexture = pMesh->GetNode()->GetSrcObject<FbxTexture>(index);
        if(pTexture)
        {
            string name(pTexture->GetName());
            TEXTURE_TYPE type = GetTextureType( pTexture->GetTextureUse() );
            FbxTexture::EMappingType maptype = pTexture->GetMappingType();      // FIX THIS TO HANDLE OTHER MAPPING TYPES, UV-ALWAYS ASSUME FOR NOW
            AddTexture(name, type);                                                 // FIX THIS TO HANDLE WHEN THE TEXTURE IS UNDEFINED
        }
    }
    return true;
}

TEXTURE_TYPE FBXMeshIO::GetTextureType(const FbxTexture::ETextureUse& FBXType)
{
    switch(FBXType)
    {
    case FbxTexture::ETextureUse::eBumpNormalMap:
        return TEXTURE_TYPE::NORMAL_MAP_TANGENT;
        break;

    case FbxTexture::ETextureUse::eStandard:
        return TEXTURE_TYPE::DIFFUSE_MAP;
        break;

    case FbxTexture::ETextureUse::eLightMap:
        return TEXTURE_TYPE::LIGHT_MAP;
        break;

    case FbxTexture::ETextureUse::eShadowMap:
        return TEXTURE_TYPE::SHADOW_MAP;
        break;
    }
    return TEXTURE_TYPE::UNDEFINED;
}

bool FBXMeshIO::GetSurfaceMaterial(FbxMesh* pMesh)
{
    const uint material_count = pMesh->GetNode()->GetSrcObjectCount<FbxSurfaceLambert>();
    if(material_count > 1 || material_count == 0)
    {
        return false; // Add code for UIMessage
    }

    FbxSurfacePhong* pMaterial = pMesh->GetNode()->GetSrcObject<FbxSurfacePhong>();
    if(pMaterial)
    {
        Material &M = m_material;
        M.Ambient
            (
            static_cast<float>(pMaterial->Ambient.Get()[0]),
            static_cast<float>(pMaterial->Ambient.Get()[1]),
            static_cast<float>(pMaterial->Ambient.Get()[2]),
            static_cast<float>(pMaterial->AmbientFactor.Get())
            );
        M.Diffuse
            (
            static_cast<float>(pMaterial->Diffuse.Get()[0]),
            static_cast<float>(pMaterial->Diffuse.Get()[1]),
            static_cast<float>(pMaterial->Diffuse.Get()[2]),
            static_cast<float>(pMaterial->DiffuseFactor)
            );
        M.Specular
            (
            static_cast<float>(pMaterial->Specular.Get()[0]),
            static_cast<float>(pMaterial->Specular.Get()[1]),
            static_cast<float>(pMaterial->Specular.Get()[2]),
            static_cast<float>(pMaterial->SpecularFactor.Get())
            );
        M.Reflection(0.0f, 0.0f, 0.0f, 0.0f);
    }
    if(m_material.Specular.w < 2.0f)
        m_material.Specular.w = 32.0f;
    return true;
}