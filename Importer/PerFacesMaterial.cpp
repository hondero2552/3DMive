#include "PerFaceMaterial.h"

void PERFACEMATERIAL::FormatIndexData(uint highestvalue)
{
    mIndicesFormat = highestvalue > USHRT_MAX ? INDEX_FORMAT::INT_TYPE: INDEX_FORMAT::SHORT_TYPE;
    const auto INDEX_COUNT = mIndicesCount;

    // Set the internal variable, but first realease it if necessary
    ReleaseIndexPtrMemory(mIndicesFormat, m_pIndices);
    m_pIndices  = mIndicesFormat == SHORT_TYPE ? reinterpret_cast<void*>(new unsigned short[INDEX_COUNT]) : reinterpret_cast<void*>(new uint[INDEX_COUNT]);

    if(mIndicesFormat == SHORT_TYPE)
    {
        unsigned short* ptr = reinterpret_cast<unsigned short*>(m_pIndices);
        for(uint i = 0; i < INDEX_COUNT; ++i)
        {
            ptr[i] = mIndices[i];
        }
    }
    else
    {
        uint* ptr = reinterpret_cast<uint*>(m_pIndices);
        for(uint i = 0; i < INDEX_COUNT; ++i)
        {
            ptr[i] = mIndices[i];
        }
    }

    // Clean unnecessary memory
    mIndices.clear();
    vector<uint>().swap(mIndices);
}

void PERFACEMATERIAL::push_vertex(size_t pos_index, size_t normal_index, size_t uv_index)
{
    assert(m_pCurrentSmoothingGroup != nullptr);

    ++mIndicesCount;
    POS_NORMAL_UV local;
    local.vertex_index  = pos_index;
    local.normal_index  = normal_index;
    local.uv_index      = uv_index;
    m_pCurrentSmoothingGroup->insert_face(local);
}

PERFACEMATERIAL::~PERFACEMATERIAL(void)
{
    ReleaseIndexPtrMemory(mIndicesFormat, m_pIndices);
    EmptyPointersList(mTexturesList);
    EmptyPointersList(mSmoothingGroupList);
}

PERFACEMATERIAL::PERFACEMATERIAL(const string& _name) : m_name(_name), m_wzName(_name.begin(), _name.end()), m_pIndices(nullptr), m_pCurrentSmoothingGroup(nullptr), mIndicesCount(0)
{    
    // this smoothing group is the default in case the OBJ file does not specify it in the file
    // and if it does, it doesn't affect this.
    SMOOTHINGGROUP* s_off = new SMOOTHINGGROUP();
    s_off->m_name = "off";
    mSmoothingGroupList.push_front(s_off);
    m_pCurrentSmoothingGroup = mSmoothingGroupList.front();
}

void PERFACEMATERIAL::CleanUp(void)
{    
    mIndices.clear();
    vector<uint>().swap(mIndices);
}

void PERFACEMATERIAL::SetMaterialAmbient(const float4 _ambient)
{
    mMaterial.Ambient   = _ambient;
}
void PERFACEMATERIAL::SetMaterialDiffuse(const float4 _diffuse)
{
    mMaterial.Diffuse(_diffuse.x, _diffuse.y, _diffuse.z, mMaterial.Diffuse.w);
}
void PERFACEMATERIAL::SetMaterialSpecular(const float4 _specular)
{
    mMaterial.Specular(_specular.x, _specular.y,_specular.z, mMaterial.Specular.w);
}
void PERFACEMATERIAL::SetMaterialSpecularTerm(const float _factor)
{
    mMaterial.Specular.w = _factor;
}
void PERFACEMATERIAL::SetMaterialAlphaChannel(const float _AlphaFactor)
{
    mMaterial.Diffuse.w = _AlphaFactor;
}

void PERFACEMATERIAL::AddSmoothingGroup(const string& SGname)
{
    auto found = std::find(mSmoothingGroupList.begin(), mSmoothingGroupList.end(), SGname);
    
    // if it was found set it as the current smoothing group.
    if(found != mSmoothingGroupList.end())
    {
        m_pCurrentSmoothingGroup = (*found);
    }
    // if not found add it AND then set it as the current smoothing group.
    else
    {
        mSmoothingGroupList.push_front( new SMOOTHINGGROUP(SGname) );                                   // FIX THIS.... THIS MIGHT THROW AN EXCEPTION
        m_pCurrentSmoothingGroup = mSmoothingGroupList.front();
    }
}