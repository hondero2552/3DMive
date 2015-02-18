#if defined(_WINDOWS) || defined(WINDOWS)
#pragma once
#endif

#ifndef MESH_H
#define MESH_H

#include "Debug_ALL.h"
#include "Texture.h"
#include <forward_list>
#include <algorithm>
#include "PerFaceMaterial.h"

using namespace omi;

class Mesh
{
private:
    float*  m_vertices;             // Pointer to the raw per-vertex data; NOTE: this data changes depending on the vertex format
    
    void*   m_indices;              // void because we don' know if they are 16 or 32 bits integers
    char*   m_data;                 // This is used when the data is obtained from a HND file. HND files save data continuously in memory.
    
    //uint    m_indexcount;           // Mesh's indices count (Legacy?)
    uint    m_vertexcount;          // Mesh's vertices count
    uint    m_iPolygonCount;        // Mesh's triangle count

    DATA_FORMAT     m_vertexFormat; // enum VERTEX_FORMAT
    INDEX_FORMAT    m_indexFormat;  // enum INDEX_FORMAT
    
    AABB m_AABB;                    // Axis-aligned bounding box of the mesh

    Material m_material;            // Kept for legacy compatibility?
    
    bool FindTextureType(const TEXTURE_TYPE& type) const;

    std::forward_list<Texture*>         m_pTexturesList;        // This is kept for legacy compatibility
    std::forward_list<PERGROUPDATA*>    m_PergroupDataList;     // This is the NEW way in which data will be saved 
    vector<FaceNormal> mFacesnormals;                           // Per-face-normal vectors

public:
    Mesh(void);
    ~Mesh(void);

    uint GetIndexCount(void) const;
    uint GetVertexCount(void) const;
    uint GetVertexStride(void) const;

    INDEX_FORMAT GetIndexFormat(void) const;
    DATA_FORMAT GetVertexFormat(void) const;

    // Vertex Data
    const void* GetVertexData(void) const;
    const void* GetIndexData(void) const;

    // Indices  /*LEGACY FEATURE, IT NEEDS TO BE REMOVED*/
    void SetIndices(void*& indices, const uint& count, const INDEX_FORMAT& format);
    void SetPerVertexData(float*& verts, const uint& count, const DATA_FORMAT& format);

    // faces' normal
    void SetFacesNormal(const vector<FaceNormal>& _facenormals) { mFacesnormals = _facenormals; m_iPolygonCount = _facenormals.size();}
    const vector<FaceNormal>& GetFacesNormal(void) const { return mFacesnormals; }
    uint GetPlygonCount(void) const { return m_iPolygonCount; }
    
    // these functions are mainly for HND files
    void AddTexture(Texture*& T) { m_pTexturesList.push_front(T); }
    void SetRawData(char* data) { m_data = data;}
    
    const std::forward_list<PERGROUPDATA*>& GetPerPolygonGroupData(void) const { return m_PergroupDataList; }
    void SetMaterial(const Material& M);
    void SetPerPolygonGroupData(const std::forward_list<PERFACEMATERIAL*>& Materials);
    const Material& GetMaterial(void) const { return m_material; }
    
    void SetAxisAlignedBoundingBox(const AABB& _aabb) { m_AABB = _aabb; }
    const AABB& GetAxisAlignedBoundingBox(void) const { return m_AABB; }
    inline bool HasNormalMap(void) const { return FindTextureType(TEXTURE_TYPE::NORMAL_MAP_TANGENT); }
    inline bool HasLightmap(void) const { return FindTextureType(TEXTURE_TYPE::LIGHT_MAP); }
    inline bool HasDiffuseMap(void) const { return FindTextureType(TEXTURE_TYPE::DIFFUSE_MAP); }
};

#endif