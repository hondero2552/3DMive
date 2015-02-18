#if defined(WINDOWS) || defined(_WINDOWS)
#pragma once
#endif

#ifndef MESHIO_UTILITY_H
#define MESHIO_UTILITY_H

#include "math_funcs.h"
#include "common_data_attributes.h"
#include <cstdio>
using namespace omi;
using std::min;
using std::max;

struct UV_NORMAL_INDEX
{
    uint m_tex_coord;
    uint m_normal_coord;
    uint m_index;
};

struct positions_indices
{
    vector<UV_NORMAL_INDEX> position_index;
};

struct POS_NORMAL_UV
{
    uint vertex_index;
    uint uv_index;
    uint normal_index;    
    inline void operator=(const POS_NORMAL_UV& F) 
    {
        vertex_index    = F.vertex_index; 
        normal_index    = F.normal_index; 
        uv_index        = F.uv_index; 
    }
    void operator ()(uint _vertex_index, uint _uv_index, uint _normal_index) 
    { 
        vertex_index    = _vertex_index; 
        uv_index        = _uv_index;
        normal_index    = _normal_index;        
    }
};

struct face
{
    uint v0, v1, v2;
    double3 faceNormal;
    double area;
    void operator() (size_t _V0, size_t _V1, size_t _V2)
    {
        v0 = _V0;
        v1 = _V1;
        v2 = _V2;
    }
    const face& operator=(const face& LH)
    {
        v0          = LH.v0;
        v1          = LH.v1;
        v2          = LH.v2;
        faceNormal  = LH.faceNormal;
        area        = LH.area;
        return *this;
    }
};

struct SMOOTHINGGROUP
{
    string m_name;
    vector<POS_NORMAL_UV> mPOS_NORMAL_UV;
    uint mPolygonCount;

    SMOOTHINGGROUP(void) : mPolygonCount(0){ }    

    // copy constructor; copies the name only
    SMOOTHINGGROUP(const SMOOTHINGGROUP& SG) : mPolygonCount(0)
    {
        m_name = SG.m_name;
    }
    SMOOTHINGGROUP(const string& _name) : mPolygonCount(0)
    {
        m_name = _name;
    }
    void insert_face(const POS_NORMAL_UV& F)
    {
        mPOS_NORMAL_UV.push_back(F);
    }
};
struct SGNAME
{
    string name;
    vector<face> mfaces;
    uint msize;
    SGNAME(void) : msize(0) { }
};

struct Lockable_Normal
{    
    std::mutex m_mutex;
    omi::double3 m_normal;

    Lockable_Normal(void) { }
    Lockable_Normal(const Lockable_Normal& LN) { m_normal = LN.m_normal; }

    Lockable_Normal& operator=(const Lockable_Normal& LN) { m_normal = LN.m_normal; return *this;}
};

// WEIGHTED AVERAGE METHOD WITH LEGS' ANGLE
static void ComputeNormalsSmoothest(
    uint begin_index,                       //
    uint count,                             //
    const vector<face>& polygonFaces,       // 
    const vector<double3>& positions,       //
    vector<Lockable_Normal>& normals_out    //
    )

{
    const size_t max_index = begin_index + count;

    for(size_t current_face_index = begin_index; current_face_index < max_index; ++current_face_index)
    {
        const face& current_face = polygonFaces[current_face_index];

        // Get the triangle's index
        for(size_t x = 0; x < polygonFaces.size(); ++x)
        {
            const face& neighbor_face = polygonFaces[x];

            // skip self
            if(
                (neighbor_face.v0 != current_face.v0) && 
                (neighbor_face.v1 != current_face.v1) && 
                (neighbor_face.v1 != current_face.v1)
                )
            {
                size_t first_vector   = 0;
                size_t second_vector  = 0;
                size_t third_vector   = 0;

                const size_t& v0 = neighbor_face.v0;
                const size_t& v1 = neighbor_face.v1;
                const size_t& v2 = neighbor_face.v2;

                if((v0 == current_face.v0 || v0 == current_face.v1 || v0 == current_face.v2)) // they share vertex 1
                {
                    first_vector    = v0;
                    second_vector   = v1;
                    third_vector    = v2;

                    double3 vector0 = positions[first_vector] - positions[second_vector];
                    double3 vector1 = positions[first_vector] - positions[third_vector];

                    MLVectorNormalize(vector0);
                    MLVectorNormalize(vector1);                    
                    const double angle_between_vectors = acos( MLVectorDot(vector0, vector1) );

                    const double3 AVERAGE_VALUE = (neighbor_face.faceNormal * neighbor_face.area * angle_between_vectors);

                    // Add to vertex 0
                    {
                        std::lock_guard<std::mutex> lck(normals_out[v0].m_mutex);
                        normals_out[v0].m_normal += AVERAGE_VALUE;
                    }
                    // Add to vertex 1
                    {
                        std::lock_guard<std::mutex> lck(normals_out[v1].m_mutex);
                        normals_out[v1].m_normal += AVERAGE_VALUE;
                    }
                    // Add to vertex 2
                    {
                        std::lock_guard<std::mutex> lck(normals_out[v2].m_mutex);
                        normals_out[v2].m_normal += AVERAGE_VALUE;
                    }
                }
                if((v1 == current_face.v0 || v1 == current_face.v1 || v1 == current_face.v2)) // they share vertex 2
                {
                    first_vector    = v1;
                    second_vector   = v0;
                    third_vector    = v2;

                    double3 vector0 = positions[first_vector] - positions[second_vector];
                    double3 vector1 = positions[first_vector] - positions[third_vector];

                    MLVectorNormalize(vector0);
                    MLVectorNormalize(vector1);
                    const double angle_between_vectors = acos( MLVectorDot(vector0, vector1) );

                    const double3 AVERAGE_VALUE = (neighbor_face.faceNormal * neighbor_face.area * angle_between_vectors);

                    // Add to vertex 0
                    {
                        std::lock_guard<std::mutex> lck(normals_out[v0].m_mutex);
                        normals_out[v0].m_normal += AVERAGE_VALUE;
                    }
                    // Add to vertex 1
                    {
                        std::lock_guard<std::mutex> lck(normals_out[v1].m_mutex);
                        normals_out[v1].m_normal += AVERAGE_VALUE;
                    }
                    // Add to vertex 2
                    {
                        std::lock_guard<std::mutex> lck(normals_out[v2].m_mutex);
                        normals_out[v2].m_normal += AVERAGE_VALUE;
                    }
                }
                if((v2 == current_face.v0 || v2 == current_face.v1 || v2 == current_face.v2)) // they share vertex 3
                {
                    first_vector    = v2;
                    second_vector   = v0;
                    third_vector    = v1;

                    double3 vector0 = positions[first_vector] - positions[second_vector];
                    double3 vector1 = positions[first_vector] - positions[third_vector];

                    MLVectorNormalize(vector0);
                    MLVectorNormalize(vector1);
                    const double angle_between_vectors = acos( MLVectorDot(vector0, vector1) );

                    const double3 AVERAGE_VALUE = (neighbor_face.faceNormal * neighbor_face.area * angle_between_vectors);

                    // Add to vertex 0
                    {
                        std::lock_guard<std::mutex> lck(normals_out[v0].m_mutex);
                        normals_out[v0].m_normal += AVERAGE_VALUE;
                    }
                    // Add to vertex 1
                    {
                        std::lock_guard<std::mutex> lck(normals_out[v1].m_mutex);
                        normals_out[v1].m_normal += AVERAGE_VALUE;
                    }
                    // Add to vertex 2
                    {
                        std::lock_guard<std::mutex> lck(normals_out[v2].m_mutex);
                        normals_out[v2].m_normal += AVERAGE_VALUE;
                    }
                }
            }
        }
    }
}

static void MoveToNextSpace(const char* buffer, size_t& start_index)
{
    while(buffer[start_index] != ' ')
        ++start_index;
}

static void MoveToNextLine(const char* buffer, size_t& start_index)
{
    // First get to the end of the line....
    while(buffer[start_index] != '\n')
        ++start_index;

    // then place the character index at the begining of the next line.
    start_index += 1;
}

static void SkipAllSpaces(const char* buffer, size_t& start_index)
{
    while(buffer[start_index] == ' ' && buffer[start_index] != EOF)
        ++start_index;
}

static size_t CountValidCharacters(const char* buffer, size_t start_index)
{
    size_t lCount = 0;
    // Read only valid characters
    while( (buffer[start_index] != ' ')    &&
        (buffer[start_index] != '\r')   &&
        (buffer[start_index] != '\n')
        )
    {
        ++lCount;
        ++start_index;
    }

    return lCount;
}

static FILE_FORMAT GetFileFormatEnum(const wstring& file)
{
    // Locate the last dot
    uint last_dot = 0;
    for(size_t i = 0; i < file.size(); ++i)
    {
        const wchar_t ch = file[i];
        if(ch == L'.')
            last_dot = i;
    }

    uint index = file.find('.') + 1;

    const wchar_t* begin  = &(file[index]);
    const wchar_t* last = &(file[file.size()]);

    wstring format(begin, last);

    // return the appropriate format
    if(format == L"obje")
        return FILE_FORMAT::OBJE;

    else if(format == L"fbx")
        return FILE_FORMAT::FBX;

    else if(format == L"hnd")
        return FILE_FORMAT::HND;

    else if(format == L"obj")
        return FILE_FORMAT::OBJ;

    else
        return FILE_FORMAT::FORMAT_UNKNOWN;
}

static wstring GetFileFormatFromEnum(FILE_FORMAT type)
{
    wstring format;
    switch (type)
    {
    case OBJ:
        format = L"obj";
        break;
    case OBJE:
        format = L"obje";
        break;
    case HND:
        format = L"hnd";
        break;
    case FBX:
        format = L"fbx";
        break;
    }
    return format;
}

static void FlipFace(face& _face)
{
    uint v1     = _face.v1;
    _face.v1    = _face.v2;
    _face.v2    = v1;
}

static bool operator==( const SGNAME* left, const SMOOTHINGGROUP& right)
{
    return (left->name == right.m_name);
}

// comparison operator
static bool operator==(const SMOOTHINGGROUP* SG, const std::string& str)
{
    return (SG->m_name == str);
}

static uint GetDigitsAmount(const float& numbers)
{
    if(numbers>-1 && numbers < 10)
        return 1;
    else if(numbers > 9.99 &&  numbers < 100)
        return 2;
    else if(numbers > 99.99 &&  numbers < 1000)
        return 3;
    else if(numbers > 999.99 &&  numbers < 10000)
        return 4;
    else if(numbers > 9999.999 &&  numbers < 100000)
        return 5;
    else if(numbers > 99999.999 &&  numbers < 1000000)
        return 6;
    else return 7;
}

static size_t GetLongestPositionDouble(const AABB& m_AABB)
{
    uint max_character = 0;
    float lowest    = min(min(m_AABB.m_lowest.x, m_AABB.m_lowest.y), m_AABB.m_lowest.z);
    float highest   = max(max(m_AABB.m_highest.x, m_AABB.m_highest.y), m_AABB.m_highest.z);
    if(lowest < 0)
        lowest *= -1;
    if(highest < 0)
        highest *= -1;
    float maximum = max(lowest, highest);
    max_character = GetDigitsAmount(maximum);

    if(
        (min(min(m_AABB.m_lowest.x, m_AABB.m_lowest.y), m_AABB.m_lowest.z)) < 0 ||
        (max(max(m_AABB.m_highest.x, m_AABB.m_highest.y), m_AABB.m_highest.z) < 0)
        )
    {
        max_character += 1;
    }

    return max_character;
}

#endif