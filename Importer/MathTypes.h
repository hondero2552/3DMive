#if defined(WINDOWS) || defined(_WINDOWS)
#pragma once
#endif

#ifndef MATH_TYPES_H
#define MATH_TYPES_H
#include "E:\\Projects\Importer\Importer\Debug_ALL.h"
#include <cmath>
#include <climits>
#include <float.h>

namespace omi
{
    //Constants
    const float PI          = 3.141592654f;
    const double PI_DOUBLE  = 3.14159265358979323;
    const float PI_OVER_TWO = PI/2.0f;
    const float TWO_PI      = 2.0f * PI;
    const float SQUARE_SIZE_INCHES = 1.5f;

    //Structures
    struct Square { uint Owner; uint ID; };

    struct float2
    {
        float u;
        float v;

        float2() { }
        explicit float2(const float& _u, const float& _v) : u(_u), v(_v) { }

        void operator =(const float2& _rh) { u = _rh.u; v = _rh.v; }

    };//For UV Coordinates
    struct float4
    {
        float x;
        float y;
        float z;
        float w;

        float4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
        explicit float4(const float& _x, const float& _y, const float& _z, const float& _w) : x(_x), y(_y), z(_z), w(_w) { }
        float4(const float4& _f4) : x(_f4.x), y(_f4.y), z(_f4.z), w(_f4.w) { }

        void operator-()
        {
            x*=-1;
            y*=-1;
            z*=-1;
            w*=-1;
        }
        float4 operator-(const float4& _rh)
        {
            float4 f4;
            f4.x = x - _rh.x;
            f4.y = y - _rh.y;
            f4.z = z - _rh.z;
            f4.w = w - _rh.w;
            return f4;
        }
        float4 operator+(const float4& _rh)
        {
            float4 f4;
            f4.x = x + _rh.x;
            f4.y = y + _rh.y;
            f4.z = z + _rh.z;
            f4.w = w + _rh.w;
            return f4;
        }

        float4& operator = (const float4& _f) { x = _f.x; y = _f.y; z = _f.z; w = _f.w; return *this; }

        inline void operator()(const float& _x, const float& _y,const float& _z, const float& _w) { x = _x; y = _y; z = _z; w = _w; } 

        inline void operator+=(const float4& _rh)
        {
            x += _rh.x;
            y += _rh.y;
            z += _rh.z;
            w += _rh.w;
        }

        inline void operator-=(const float4& _rh)
        {
            x -= _rh.x;
            y -= _rh.y;
            z -= _rh.z;
            w -= _rh.w;
        }
    };//4D Vector : 32-bit aligned
    struct float3
    {
        float x;
        float y;
        float z;

        float3() {}
        float3(const float& f) : x(f), y(f), z(f) { }
        explicit float3(const float& _x, const float& _y, const float& _z) : x(_x), y(_y), z(_z) { }
        float3(const float4& _f4) : x(_f4.x), y(_f4.y), z(_f4.z) { }	

        void operator-()
        {
            x*=-1;
            y*=-1;
            z*=-1;
        }
        float3 operator-(const float3& _rh)
        {
            float3 f3;
            f3.x = x - _rh.x;
            f3.y = y - _rh.y;
            f3.z = z - _rh.z;
            return f3;
        }
        inline void operator=(const float3& _rh)
        {
            x = _rh.x;
            y = _rh.y;
            z = _rh.z;
        }
        inline void operator=(const float& _rh)
        {
            x = _rh;
            y = _rh;
            z = _rh;
        }
        inline void operator+=(const float3& _rh)
        {
            x += _rh.x;
            y += _rh.y;
            z += _rh.z;
        }
        inline void operator-=(const float3& _rh)
        {
            x -= _rh.x;
            y -= _rh.y;
            z -= _rh.z;
        }
        inline float3 operator*(const float& _rh)
        {
            float3 f3;
            f3.x = x * _rh;
            f3.y = y * _rh;
            f3.z = z * _rh;
            return f3;
        }
        inline void operator()(const float& _x, const float& _y,const float& _z) { x = _x; y = _y; z = _z; }

    };//
    struct UINT3
    {
        uint x, y, z;
        UINT3() : x(0), y(0), z(0) { }//Empty constructor
    };
    struct UINT4
    {
        uint x, y, z, w;
        UINT4() : x(0), y(0), z(0), w(0) { }//Empty constructor
    };

    // DOUBLE PRECISION
    struct double2
    {
        double u;
        double v;

        double2() { }
        explicit double2(const double& _u, const double& _v) : u(_u), v(_v) { }

        void operator =(const double2& _rh) { u = _rh.u; v = _rh.v; }

    };//For UV Coordinates
    struct double4
    {
        double x;
        double y;
        double z;
        double w;

        double4() {}
        explicit double4(const double& _x, const double& _y, const double& _z, const double& _w) : x(_x), y(_y), z(_z), w(_w) { }
        double4(const double4& _f4) : x(_f4.x), y(_f4.y), z(_f4.z), w(_f4.w) { }

        void operator-()
        {
            x*=-1;
            y*=-1;
            z*=-1;
            w*=-1;
        }
        double4 operator-(const double4& _rh)
        {
            double4 f4;
            f4.x = x - _rh.x;
            f4.y = y - _rh.y;
            f4.z = z - _rh.z;
            f4.w = w - _rh.w;
            return f4;
        }
        double4 operator+(const double4& _rh)
        {
            double4 f4;
            f4.x = x + _rh.x;
            f4.y = y + _rh.y;
            f4.z = z + _rh.z;
            f4.w = w + _rh.w;
            return f4;
        }

        double4& operator = (const double4& _f) { x = _f.x; y = _f.y; z = _f.z; w = _f.w; return *this; }

        inline void operator()(const double& _x, const double& _y,const double& _z, const double& _w) { x = _x; y = _y; z = _z; w = _w; } 

        inline void operator+=(const double4& _rh)
        {
            x += _rh.x;
            y += _rh.y;
            z += _rh.z;
            w += _rh.w;
        }

        inline void operator-=(const double4& _rh)
        {
            x -= _rh.x;
            y -= _rh.y;
            z -= _rh.z;
            w -= _rh.w;
        }
    };//4D Vector : 32-bit aligned
    struct double3
    {
        double3(void) : x(0.0), y(0.0), z(0.0) { }
        double x;
        double y;
        double z;

        double3(const double& f) : x(f), y(f), z(f) { }
        explicit double3(const double& _x, const double& _y, const double& _z) : x(_x), y(_y), z(_z) { }
        double3(const double4& _f4) : x(_f4.x), y(_f4.y), z(_f4.z) { }	

        void operator-()
        {
            x*=-1;
            y*=-1;
            z*=-1;
        }
        double3 operator-(const double3& _rh)
        {
            double3 f3;
            f3.x = x - _rh.x;
            f3.y = y - _rh.y;
            f3.z = z - _rh.z;
            return f3;
        }
        inline void operator=(const double3& _rh)
        {
            x = _rh.x;
            y = _rh.y;
            z = _rh.z;
        }
        inline void operator=(const double& _rh)
        {
            x = _rh;
            y = _rh;
            z = _rh;
        }
        inline void operator+=(const double3& _rh)
        {
            x += _rh.x;
            y += _rh.y;
            z += _rh.z;
        }
        inline void operator-=(const double3& _rh)
        {
            x -= _rh.x;
            y -= _rh.y;
            z -= _rh.z;
        }
        inline double3 operator*(const double& _rh)
        {
            double3 f3;
            f3.x = x * _rh;
            f3.y = y * _rh;
            f3.z = z * _rh;
            return f3;
        }
        inline void operator()(const double& _x, const double& _y,const double& _z) { x = _x; y = _y; z = _z; }

    };//

    //Matrix
    struct float4x4
    {
        union
        {
            struct
            {
                float _11, _12, _13, _14;
                float _21, _22, _23, _24;
                float _31, _32, _33, _34;
                float _41, _42, _43, _44;
            };
            float m_matrix[4][4];
        };

        inline float4x4& operator=(const float4x4& R)
        {
            for(unsigned int i = 0; i < 4; ++i)
            {
                for(unsigned int j = 0; j < 4; ++j)
                {
                    m_matrix[i][j] = R.m_matrix[i][j];
                }
            }
            return *this;
        }

        float4x4()
        {
            // Set all the Matrix's floats to 0.0f or 1.0f if it's in the diagonal line
            for(uint row = 0; row < 4; ++row)
            {
                for(uint column = 0; column < 4; ++column)
                {
                    float value = 0.0f;
                    
                    // Identity Matrix
                    if(row == column)
                        value = 1.0f;                    
                    m_matrix[row][column] = value;
                }
            }
        }
    };
    //Vertex structures
    struct Vertex
    {
        double3 Position;
        double3 Normal;
        double4 TangenU;
        double2 TexCoor;
    };
    struct Vertex_dynamic
    {
        float4 World;
        uint Owner;

        inline Vertex_dynamic& operator=(const Vertex_dynamic& M)
        {
            World = M.World;
            Owner = M.Owner;
            return *this;
        }
    };
    struct Vertex_static
    {
        float4 Position;
        float4 Normal;
    };
    //Triangulated faces data
    struct triangulatedVertex { UINT3 m_indices; UINT3 Tex_Indx;};
    struct Ray
    {
        float3 direction;
        float3 origin;
    };
    struct Plane
    {
        float3 n;
        float3 p;
        float d;

        // Intersection test with a Ray
        inline float Intersection(const Ray& r) const;
    };
    struct AABB
    {
    public:
        AABB() 
        {
            m_lowest(FLT_MAX, FLT_MAX, FLT_MAX);
            m_highest(-FLT_MAX, -FLT_MAX, -FLT_MAX);
        }
        float3 m_lowest, m_highest;

        bool IntersectionRay(const Ray& ray) const
        {
            float maxS = 0.0f;      // for line, use -FLT_MAX
            float minT = FLT_MAX;   // for line segment, use length

            // do x coordinate test (yz planes)
            // compute sorted intersection parameters
            float s, t;
            float recipX = 1.0f/ray.direction.x;
            if ( recipX >= 0.0f )
            {
                s = (m_lowest.x - ray.origin.x)*recipX;
                t = (m_highest.x - ray.origin.x)*recipX;
            }
            else
            {
                s = (m_highest.x - ray.origin.x)*recipX;
                t = (m_lowest.x - ray.origin.x)*recipX;
            }

            // adjust min and max values
            if ( s > maxS )
                maxS = s;
            if ( t < minT )
                minT = t;

            // check for intersection failure
            if ( maxS > minT )
                return false;

            // do y and z coordinate tests (xz & xy planes)


            // done, have intersection
            return true;
        }
        AABB& operator = (const AABB& _aabb)
        {
            m_lowest    = _aabb.m_lowest;
            m_highest   = _aabb.m_highest;
            return *this;
        }

        float4 GetCenterPoint(void) const
        {
            return float4(
                (m_lowest.x + m_highest.x)/2.0f,
                (m_lowest.y + m_highest.y)/2.0f,
                (m_lowest.z + m_highest.z)/2.0f,
                1.0f);
        }
    };
    struct Material
    {
        float4 Ambient;
        float4 Diffuse;
        float4 Specular;
        float4 Reflection;
        inline Material& operator=(const Material& M)
        {
            Ambient = M.Ambient;
            Diffuse = M.Diffuse;
            Specular= M.Specular;
            Reflection = M.Reflection;

            return *this;
        }
        // Deafult constructor always initializes the alpha value to 1.0f
        Material(void) : 
            Ambient(0.0f, 0.0f, 0.0f, 1.0f),
            Diffuse(0.0f, 0.0f, 0.0f, 1.0f), 
            Specular(0.0f, 0.0f, 0.0f, 1.0f) { }
    };    
    struct PerObject
    {
        Material material;
        uint DiffuseMapIndex;
        uint NormalMapIndex;
        uint SpecMapIndex;
        uint miUseTextures;

        PerObject(void) : miUseTextures(1) { }
    };
    struct FaceNormal
    {
        float3 mFaceNormal;
        float3 mCentroid;
        float3 mCentroidTip;
    };

    struct Rectangle
    {
        float mleft, mright, mtop, mbottom;
    };
    // Global typedefs
    typedef float4 vector4D;
    typedef float3 vector3D;
    typedef float4x4 Matrix;

    // GLOBAL OPERATORS
}

#endif