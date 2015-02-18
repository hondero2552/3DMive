#if defined(_WINDOWS) || defined(WINDOWS)
#pragma once
#endif

#ifndef MESH_IO_BASE_H
#define MESH_IO_BASE_H

#include "IView.h"
#include "Texture.h"
#include "Mesh.h"
#include <cstdio>
#include <stdarg.h>
#include "PerFaceMaterial.h"


const size_t MAX_FORMAT_LENGHT = 4U;
using namespace omi;

const int TRIANGLE_VERTEX_COUNT = 3;

// Saves the unique vertex-index with it's corresponding normal/uv index

class MeshIOBase
{
protected:
    forward_list<PERFACEMATERIAL*> m_PerFaceMaterialList;
    PERFACEMATERIAL* m_currentPerFaceMaterial;

    // Variables to track correct uv/vertex coordinates
    vector<double3> m_original_positions;
    vector<double3> m_original_normals;
    vector<double2> m_original_uvs;

    vector<Vertex>  m_vertices;
    vector<uint>    m_indices;

    vector<Lockable_Normal> m_temp_normals;

    // finalized data

    vector<POS_NORMAL_UV>   m_polygon_indices;
    vector<face>            m_polygon_faces;

    wstring m_InputFileDirectory;
    wstring m_InputFileName;
    wstring m_InputFileFormat;

    wstring m_OutputFileDirectory;
    wstring m_OutputFileName;
    wstring m_OutputFileFormat;

    wstring m_InputFileFullPath;

    forward_list<Texture*> m_pTextures;
    uint            m_next_available_index;
    bool            m_bNormalsCalculated;
    bool            m_AllDataIsPerVertex;
    bool            m_bUseOriginalNormals;
    bool            m_bMeshWasFlipped;
    bool            m_bFlippedUVsOnly;
    Material        m_material;
    HANDEDNESS      m_handedness;
    bool            m_windingCCW;
    unsigned short  m_material_number;

    IView* m_pIView;

    // Bounding Box for this mesh
    AABB m_AABB;

    //
    FILE_FORMAT m_format;
    //
    bool m_bnormalMap, m_bLightMap, m_bDiffuseMap, m_bShadowMap, m_bHeightMap, m_bSpecularMap;

    // Funcions
    uint GetVertexIndex(const uint& _posIndex, const uint& _textureIndex, const uint& _normalIndex);
    void AddTexture(const wstring& fullpath, TEXTURE_TYPE type);
    void AddTexture(const string& fullpath, TEXTURE_TYPE type);
    void AddTexture(const char* fullpath, TEXTURE_TYPE type);

    void ComputeNormals(void);
    void ComputeTangentVectors(void);
    void ZeroNormals(void);

    void CompareForAABB(const float3& F3);
    void CompareForAABB(const double3& vertex) 
    { 
        CompareForAABB(
            float3(
            static_cast<float>(vertex.x), 
            static_cast<float>(vertex.y),
            static_cast<float>(vertex.z)
            )
            );
    }

    void FlipIndices(bool originals = false);
    // 
    Mesh* CreateMesh(void);

    // 
    Mesh* RebuildMesh(void);

    //
    void ExtractFileSNameDirectoryAndFormat(const wstring& inputFile);

    //
    void SetDefaultMaterial(void);

    void CreateIndices(void);

    void BuildFacesNormals(void);

    void ReleaseUnnecessaryMemory(void); // Unnecessary?

    // Flips the UV coords in the m_vertices vector, not the original ones
    void FlipUVs(void);

private:
    void SetVerticesData(Mesh* pMesh) const;
    void SetPerFaceNormalsData(Mesh* pMesh);
    void SetPerPolygonGroupData(Mesh* pMesh);

    MeshIOBase(MeshIOBase& B) { }

    vector<positions_indices> m_unique_vertices;

    vector<FaceNormal>  m_faces_normals;

    // Exporting functions

    // OBJ
    void ExportToObjFile(void);
    void WriteOBJVerticesData(void);
    void WriteOBJNormalsData(void);
    void WriteOBJUVsData(void);
    void WriteOBJIndicesData(void);
    void WriteOBJMaterials(void);
    string m_VerticesToFile;
    string m_UVsToFile;
    string m_NormalsToFile;
    string m_IndicesToFile;
    
    // FBX
    void ExportToFbxFile(void);

    // OBJE
    void ExportToObjeFile(void);

    // HND
    void ExportToHndFile(void);

public:
    MeshIOBase(void);
    ~MeshIOBase(void);
    
    virtual Mesh* VProcessfile(const wstring& inputFile, IView* pIView) = 0;     // Every loader must override this parent function
    void ExportMexhToFile(const wstring& filename, FILE_FORMAT type);
    
    //
    Mesh* FlipFaces(void);
    bool isFlipable(void) 
    { 
        return (m_original_positions.size() == m_original_normals.size());
    }
    forward_list<wstring> GetMaterialNames(void) const;
    Material& GetMaterialProperties(const wstring& material_name);
};

#endif