#pragma once
#include "MeshIOBase.h"
#include "MathTypes.h"
#include <fbxsdk.h>

class FBXMeshIO : public MeshIOBase
{
public:
    FBXMeshIO(void);
    ~FBXMeshIO(void);    
    // 
    Mesh* VProcessfile(const wstring& inputFile, IView* pIView);
    void VExportMeshToFile(const wstring& file) { }
private:
    bool m_NormalsExist;
    bool m_UVsExist;
    FbxManager* m_FbxManger;

    // FUNCTIONS
    void ValidateNormalsAndUVs(FbxMesh* pMesh);
    FbxScene* InitializeFBXSdk(void) const;    

    //
    bool CopyVerticesPositions(const FbxMesh* M);
    bool CopyVerticesNormals(const FbxMesh* M);
    bool CopyVerticesUVs(const FbxMesh* M);

    bool GetTexturesInfo(FbxMesh* M);
    bool GetSurfaceMaterial(FbxMesh* M);

    TEXTURE_TYPE GetTextureType(const FbxTexture::ETextureUse& fbxtype);
};

