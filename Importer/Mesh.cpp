#include "Mesh.h"
#include <algorithm>
#include <cstdio>
using std::for_each;

Mesh::Mesh(void) : m_vertexFormat(DATA_FORMAT::UNFORMATTED), m_indexFormat(INDEX_FORMAT::INT_TYPE),
    m_vertices(nullptr),
    m_indices(nullptr),    
    m_data(nullptr),     
    m_vertexcount(0),
    m_iPolygonCount(0)
{

}

Mesh::~Mesh(void)
{
    if(m_data)
    {
        SAFE_DELETE_ARRAY(m_data);
    }
    else
    {
        bool bshort = m_indexFormat == SHORT_TYPE ? true : false;
        if(bshort)
        {
            short* ptr = reinterpret_cast<short*>(m_indices);
            SAFE_DELETE_ARRAY(ptr);
            uint debug = 0;
        }
        else
        {
            uint* ptr = reinterpret_cast<uint*>(m_indices);
            SAFE_DELETE_ARRAY(ptr);
            uint debug = 0;
        }

        SAFE_DELETE_ARRAY(m_vertices);
    }
    EmptyPointersList(m_pTexturesList);
    EmptyPointersList(m_PergroupDataList);
}

uint Mesh::GetVertexStride(void) const
{
    switch (m_vertexFormat)
    {
    case UNFORMATTED:
        return 0;
        break;
    case POS3_NORM3:
        return 6*4;
        break;
    case POS3_TEX2:
        return 5*4;
        break;
    case POS3_TEX2_NORM3:
        return 8*4;
        break;
    case POS3_COLOR3_NORM3: ////////////////////NOT USED YET
        return 9*4;
        break;
    case POS3_TEX2_NORM3_BITAN4:
        return 12*4;
        break;
    }
    return 0;
}

uint Mesh::GetVertexCount(void) const
{
    return m_vertexcount;
}

const void* Mesh::GetVertexData(void) const
{
    return m_vertices;
}

const void* Mesh::GetIndexData(void) const
{
    return m_indices;
}

void Mesh::SetIndices(void*& indices, const uint& count, const INDEX_FORMAT& format)
{
    m_indices       = indices;
    //m_indexcount    = count;
    m_indexFormat   = format;
}

void Mesh::SetPerVertexData(float*& verts, const uint& count, const DATA_FORMAT& format)
{
    m_vertices      = verts;
    m_vertexcount   = count;
    m_vertexFormat  = format;
}

bool Mesh::FindTextureType(const TEXTURE_TYPE& type) const
{
    bool btrue = false;
    for_each(m_pTexturesList.begin(), m_pTexturesList.end(), [&](Texture* ptexture)
    {
        if (ptexture->GetTextureType() == type)
            btrue = true;
    });
    return btrue;
}

void Mesh::SetMaterial(const Material& M)
{
    m_material = M;
}

INDEX_FORMAT Mesh::GetIndexFormat(void) const
{
    return m_indexFormat;
}

DATA_FORMAT Mesh::GetVertexFormat(void) const
{
    return m_vertexFormat;
}

void Mesh::SetPerPolygonGroupData(const std::forward_list<PERFACEMATERIAL*>& Materials)
{
    for_each(Materials.begin(), Materials.end(), [&] (PERFACEMATERIAL* _lpMaterial)
    {
        PERGROUPDATA* pGroupData    = new PERGROUPDATA();

        // Copy the material name
        pGroupData->mMaterialName   = _lpMaterial->GetNameW();
        
        pGroupData->mMaterial       = _lpMaterial->GetMaterial();
        
        pGroupData->mIndicesFormat  = _lpMaterial->GetIndicesFormat();
        pGroupData->mIndicesCount   = _lpMaterial->GetIndicesCount();

        // Make a copy of the textures and save it 
        CopyPointersList(_lpMaterial->GettexturesList(),  pGroupData->mTexturesList);
        
        // Copy the the pointer to the indices array and own up to it by setting it to nullptr
        pGroupData->mpIndices       = _lpMaterial->GetIndicesPointer();

        _lpMaterial->ReleaseIndicesOwnerShip();
        
        m_PergroupDataList.push_front( pGroupData );
    });
}