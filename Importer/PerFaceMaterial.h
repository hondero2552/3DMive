#if defined(_WINDOWS) || defined(WINDOWS)
#pragma once
#endif

#ifndef PERFACE_MATERIAL_H
#define PERFACE_MATERIAL_H

#include "MeshIO Utility.h"
#include "Texture.h"
#include "3DMesh Helper.h"

class PERFACEMATERIAL
{
    forward_list<Texture*>          mTexturesList;
    forward_list<SMOOTHINGGROUP*>   mSmoothingGroupList;

    Material                mMaterial;
    const string            m_name;
    const wstring           m_wzName;
    vector<uint>            mIndices;
    INDEX_FORMAT            mIndicesFormat;
    uint                    mIndicesCount;
    void*                   m_pIndices;

    SMOOTHINGGROUP* m_pCurrentSmoothingGroup;
public:    
    
    PERFACEMATERIAL(const string& _name);
    ~PERFACEMATERIAL(void);
    const forward_list<Texture*>& GettexturesList(void) const { return mTexturesList; }
    const string& GetName(void) const { return m_name; }
    const wstring& GetNameW(void) const { return m_wzName; }
    const 
    INDEX_FORMAT GetIndicesFormat(void) { return mIndicesFormat; }
    uint GetIndicesCount(void) { return mIndicesCount; }
    void push_index(const uint& index)
    {
        mIndices.push_back(index);
    }

    void push_vertex(size_t pos_index, size_t normal_index, size_t uv_index);
    
    bool operator==(const std::string& str)
    {
        return (m_name == str); 
    }

    const Material& GetMaterial(void) const { return mMaterial; }
    Material& GetMaterial(void) { return mMaterial; }
    const forward_list<SMOOTHINGGROUP*>& GetSmoothingGroupList(void) const { return mSmoothingGroupList; }
    
    void FormatIndexData(uint highestvalue);

    void CleanUp(void);             // Releases unnecessary memory, i.e. pointer to the array on indices, vector of the indices.
    void ReleaseIndicesOwnerShip(void) { m_pIndices = nullptr; }
    void* GetIndicesPointer(void) { return m_pIndices; }

    void AddTexture(Texture* ptexture) { mTexturesList.push_front( ptexture ); }

    void SetMaterialAmbient(const float4 _ambient);
    void SetMaterialDiffuse(const float4 _diffuse);
    void SetMaterialSpecular(const float4 _specular);
    void SetMaterialSpecularTerm(const float _factor);
    void SetMaterialAlphaChannel(const float _AlphaFactor);
    void SetMaterial(const Material& material) { mMaterial = material; }
    void AddSmoothingGroup(const string& SGname);
private:
    PERFACEMATERIAL(const PERFACEMATERIAL& MAT) : m_pIndices(nullptr) { }   // <- Private copy constructor
};

// Equals operator
static bool operator==(PERFACEMATERIAL* pMaterial, const string& str)
{
    return (pMaterial->GetName() == str);
}

#endif