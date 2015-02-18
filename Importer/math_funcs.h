#if defined(_WINDOWS) || defined(WINDOWS)
#pragma once
#endif

#ifndef MATHFUNCS_H
#define MATHFUNCS_H

#include "E:\\Projects\Importer\Importer\MathTypes.h"

namespace omi
{
    //Other math related functions
    inline bool isEven(const int& i) 
    { 
        return(i%2 == 0);
    }

    // 
    template<typename T>
    T clamp(const T& value, const T& min, const T& max)
    {
        T local = value;

        if(value < min)
            local = min;
        else if(value > max)
            local = max;

        return local;
    }
    template <typename T>
    T GetPercentage(const T& value, uint percent) { return( (value) * (percent/100.0f)); }
    // Convertion operations
    inline float MLConvertToRadians(float fDegrees) { return fDegrees * (PI / 180.0f); }
    inline float XMConvertToDegrees(float fRadians) { return fRadians * (180.0f / PI); }

    //-----------------------------------------------------------------------------------------------
    //4D-Vector Operations
    // FLOAT
    inline const float MLVectorDot(const float4& v1, const float4& v2) { return (v1.x*v2.x) + (v1.y*v2.y) + (v1.z*v2.z) + (v1.w*v2.w); }
    inline const float MLVectorDot(const float3& v1, const float3& v2) { return (v1.x*v2.x) + (v1.y*v2.y) + (v1.z*v2.z); }
    // DOUBLE
    inline const double MLVectorDot(const double4& v1, const double4& v2) { return (v1.x*v2.x) + (v1.y*v2.y) + (v1.z*v2.z) + (v1.w*v2.w); }
    inline const double MLVectorDot(const double3& v1, const double3& v2) { return (v1.x*v2.x) + (v1.y*v2.y) + (v1.z*v2.z); }
    // FLOAT
    inline const float MLVectorLenght(const float4& _v) { return sqrtf( (_v.x*_v.x) + (_v.y*_v.y) + (_v.z*_v.z) ); }
    inline const float MLVectorLenght(const float3& _v) { return sqrtf( (_v.x*_v.x) + (_v.y*_v.y) + (_v.z*_v.z) ); }
    // DOUBLE
    inline const double MLVectorLenght(const double4& _v) { return sqrt( (_v.x*_v.x) + (_v.y*_v.y) + (_v.z*_v.z) ); }
    inline const double MLVectorLenght(const double3& _v) { return sqrt( (_v.x*_v.x) + (_v.y*_v.y) + (_v.z*_v.z) ); }

    // FLOAT
    inline void MLVectorCross(const float3& _u, const float3& _v, float3& _out)
    {
        _out.x = _u.y*_v.z - _u.z*_v.y;
        _out.y = _u.z*_v.x - _u.x*_v.z;
        _out.z = _u.x*_v.y - _u.y*_v.x;
    }
    inline void MLVectorCross(const float4& _u, const float4& _v, float4& _out)
    {
        _out.x = _u.y*_v.z - _u.z*_v.y;
        _out.y = _u.z*_v.x - _u.x*_v.z;
        _out.z = _u.x*_v.y - _u.y*_v.x;
        _out.w = 0.0f;
    }
    inline float4 MLVectorCross(const float4& _u, const float4& _v)
    {
        float4 _out;
        _out.x = _u.y*_v.z - _u.z*_v.y;
        _out.y = _u.z*_v.x - _u.x*_v.z;
        _out.z = _u.x*_v.y - _u.y*_v.x;
        _out.w = 0.0f;
        return _out;
    }
    inline float3 MLVectorCross(const float3& _u, const float3& _v)
    {
        float3 _out;
        _out.x = _u.y*_v.z - _u.z*_v.y;
        _out.y = _u.z*_v.x - _u.x*_v.z;
        _out.z = _u.x*_v.y - _u.y*_v.x;
        return _out;
    }

    inline void MLVectorNormalize(float4& _v) 
    { 
        float l = MLVectorLenght(_v);
        if(l == 0)
            return;
        _v.x /= l;
        _v.y /= l; 
        _v.z /= l;
        _v.w /= l; 
    }
    inline void MLVectorNormalize(float3& _v) 
    { 
        float l = MLVectorLenght(_v);
        if(l == 0)
            return;
        _v.x /= l;
        _v.y /= l; 
        _v.z /= l;
    }


    // DOUBLE
    inline void MLVectorCross(const double3& _u, const double3& _v, double3& _out)
    {
        _out.x = _u.y*_v.z - _u.z*_v.y;
        _out.y = _u.z*_v.x - _u.x*_v.z;
        _out.z = _u.x*_v.y - _u.y*_v.x;
    }
    inline void MLVectorCross(const double4& _u, const double4& _v, double4& _out)
    {
        _out.x = _u.y*_v.z - _u.z*_v.y;
        _out.y = _u.z*_v.x - _u.x*_v.z;
        _out.z = _u.x*_v.y - _u.y*_v.x;
        _out.w = 0.0f;
    }
    inline double4 MLVectorCross(const double4& _u, const double4& _v)
    {
        double4 _out;
        _out.x = _u.y*_v.z - _u.z*_v.y;
        _out.y = _u.z*_v.x - _u.x*_v.z;
        _out.z = _u.x*_v.y - _u.y*_v.x;
        _out.w = 0.0f;
        return _out;
    }
    inline double3 MLVectorCross(const double3& _u, const double3& _v)
    {
        double3 _out;
        _out.x = _u.y*_v.z - _u.z*_v.y;
        _out.y = _u.z*_v.x - _u.x*_v.z;
        _out.z = _u.x*_v.y - _u.y*_v.x;
        return _out;
    }

    inline void MLVectorNormalize(double4& _v) 
    { 
        double l = MLVectorLenght(_v);
        if(l == 0)
            return;
        _v.x /= l;
        _v.y /= l; 
        _v.z /= l;
        _v.w /= l; 
    }
    inline void MLVectorNormalize(double3& _v) 
    { 
        double l = MLVectorLenght(_v);
        if(l == 0)
            return;
        _v.x /= l;
        _v.y /= l; 
        _v.z /= l;
    }


    //-----------------------------------------------------------------------------------------------
    //4x4-Matrix Operaions

    inline void MLMatrixSetToZero(float4x4& _M)
    {
        for(uint i = 0; i < 4; ++i)
        {
            for(uint j = 0; j < 4; ++j)
            {
                _M.m_matrix[i][j] = 0.0f;
            }
        }
    }

    inline void MLMatrixSetToIdentity(float4x4& _M)
    {
        MLMatrixSetToZero(_M);
        for(uint i = 0, j = 0; i < 4; ++i)
        {
            _M.m_matrix[i][j+i] = 1.0f;
        }
    }  

    inline void MLMatrixMakeCopy(const float4x4& _source, float4x4& _Destination)
    {
        for(unsigned int i = 0; i < 4; ++i)
            for(unsigned int j = 0; j < 4; ++j)
                _Destination.m_matrix[i][j] = _source.m_matrix[i][j];
    }

    inline void MLMatrixTranspose(float4x4& _M)
    {
        float4x4 temp;
        MLMatrixMakeCopy(_M, temp);
        for(uint i = 0; i < 4; ++i)
        {
            for(uint j = 0; j<4; ++j)
            {
                _M.m_matrix[i][j] = temp.m_matrix[j][i];
            }
        }
    }
    //
    static void MLMatrixLookAtLH(const float4& _pos, const float4& _target, const float4& _up, float4x4& _out)
    {
        float4 u, v, w, q;
        q = _pos;

        //         __               __   
        //         |  Ux   Vx   Wx   0 |
        //         |                   |
        //         |  Uy   Vy   Wy   0 |
        //ViewProj=|                   |   
        //         |  Uz   Vz   Wz   0 |
        //         |                   |
        //         |-Q*U -Q*U -Q*U   1 |
        //         ~~               ~~

        //Getting W
        w = const_cast<float4&>(_target) - q;
        MLVectorNormalize(w);

        //Getting U
        MLVectorCross(_up, w, u);
        MLVectorNormalize(u);

        //Getting V
        MLVectorCross(w, u, v);

        //Setting up ViewProj
        _out.m_matrix[0][0] = u.x;
        _out.m_matrix[0][1] = v.x;
        _out.m_matrix[0][2] = w.x;
        _out.m_matrix[0][3] = 0.0f;

        _out.m_matrix[1][0] = u.y;
        _out.m_matrix[1][1] = v.y;
        _out.m_matrix[1][2] = w.y;
        _out.m_matrix[1][3] = 0.0f;

        _out.m_matrix[2][0] = u.z;
        _out.m_matrix[2][1] = v.z;
        _out.m_matrix[2][2] = w.z;
        _out.m_matrix[2][3] = 0.0f;

        -q;//Negate Q

        _out.m_matrix[3][0] = MLVectorDot(q, u);
        _out.m_matrix[3][1] = MLVectorDot(q, v);
        _out.m_matrix[3][2] = MLVectorDot(q, w);
        _out.m_matrix[3][3] = 1.0f;
    }

    inline void MLMatrixPerspectiveFoVLH(const float& _FoV_Radians, const float& _ratio, const float& _NearP, const float& _FarP, Matrix& _out)
    {
        MLMatrixSetToZero(_out);
        _out.m_matrix[0][0] = 1.0f/(_ratio*tanf(_FoV_Radians/2));//Projecting x coords
        _out.m_matrix[1][1] = 1.0f/tanf(_FoV_Radians/2);//Projecting y coords
        _out.m_matrix[2][2] = _FarP/(_FarP - _NearP);//Normalizing values to the near plane
        _out.m_matrix[2][3] = 1.0f;
        _out.m_matrix[3][2] = -1*(_NearP*_FarP)/(_FarP - _NearP);//Normalizing values to the far plane
        _out.m_matrix[3][3] = 0.0f;
    }

    inline void MLMatrixCreateTranslation(const float& _x,const float& _y, const float& _z, Matrix& _out)
    {
        // First set it to the identity matrix
        MLMatrixSetToIdentity(_out);
        _out.m_matrix[3][0] = _x;
        _out.m_matrix[3][1] = _y;
        _out.m_matrix[3][2] = _z;
    }
    inline void MLMatrixCreateTranslation(const float& _x,const float& _y, const float& _z, float4& _out)
    {
        Matrix L;

        MLMatrixCreateTranslation(_x, _y, _z, L);

        _out.x = L.m_matrix[3][0];
        _out.y = L.m_matrix[3][1];
        _out.z = L.m_matrix[3][2];
        _out.w = 1.0f;
    }

    // Overloaded Operators
    inline Matrix operator*(const Matrix& L, const Matrix R)
    {
        Matrix M;
        float4 Lrow;
        float4 Rcol;
        for(uint i = 0; i < 4; ++i)
        {
            // Assign the values of the L's i row to Lrow
            // float4(      _x,           _y,           _z,           _w)
            Lrow(L.m_matrix[i][0], L.m_matrix[i][1], L.m_matrix[i][2], L.m_matrix[i][3]);

            for(uint j = 0; j < 4; ++j)
            {
                // Get the column vector of R matrix
                Rcol(R.m_matrix[0][j], R.m_matrix[1][j], R.m_matrix[2][j], R.m_matrix[3][j]);

                // Calculate the dot product betwee the row vector and column vector
                M.m_matrix[i][j] = MLVectorDot(Lrow, Rcol);
            }
        }

        return M;
    }

    inline float3 operator*(const float3& f3, const float& _rh)
    {
        float3 L3;
        L3.x = f3.x*_rh;
        L3.y = f3.y*_rh;
        L3.z = f3.z*_rh;
        return L3;
    }
    inline float3 operator-(const float3& _Lh, const float3& _rh)
    {
        float3 f3;
        f3.x = _Lh.x - _rh.x;
        f3.y = _Lh.y - _rh.y;
        f3.z = _Lh.z - _rh.z;
        return f3;
    }
    inline float3 operator+(const float3& _lh, const float3& _rh)
    {
        float3 R;
        R.x = _lh.x + _rh.x;
        R.y = _lh.y + _rh.y;
        R.z = _lh.z + _rh.z;

        return R;
    }
    static float4 operator*(const float4& V, const Matrix& M)
    {
        float4 result, row;
        //X
        row(M.m_matrix[0][0], M.m_matrix[1][0], M.m_matrix[2][0], M.m_matrix[3][0]);
        result.x = MLVectorDot(V, row);
        //Y
        row(M.m_matrix[0][1], M.m_matrix[1][1], M.m_matrix[2][1], M.m_matrix[3][1]);
        result.y = MLVectorDot(V, row);
        //Z
        row(M.m_matrix[0][2], M.m_matrix[1][2], M.m_matrix[2][2], M.m_matrix[3][2]);
        result.z = MLVectorDot(V, row);
        //W
        row(M.m_matrix[0][3], M.m_matrix[1][3], M.m_matrix[2][3], M.m_matrix[3][3]);
        result.w = MLVectorDot(V, row);

        return result;
    }

    inline double3 operator*(const double3& f3, const double& _rh)
    {
        double3 L3;
        L3.x = f3.x*_rh;
        L3.y = f3.y*_rh;
        L3.z = f3.z*_rh;
        return L3;
    }
    inline double3 operator-(const double3& _Lh, const double3& _rh)
    {
        double3 f3;
        f3.x = _Lh.x - _rh.x;
        f3.y = _Lh.y - _rh.y;
        f3.z = _Lh.z - _rh.z;
        return f3;
    }
    inline double3 operator+(const double3& _lh, const double3& _rh)
    {
        double3 R;
        R.x = _lh.x + _rh.x;
        R.y = _lh.y + _rh.y;
        R.z = _lh.z + _rh.z;

        return R;
    }

    static void MLMatrixInverseView(const Matrix& V, Matrix& M)
    {
        //Inverse
        float4 u(V._11, V._12, V._13, 0.0f);
        float4 v(V._21, V._22, V._23, 0.0f);
        float4 w(V._31, V._32, V._33, 0.0f);
        float4 q(V._41, V._42, V._43, 1.0f);

        M.m_matrix[0][0] = u.x;
        M.m_matrix[0][1] = v.x;
        M.m_matrix[0][2] = w.x;
        M.m_matrix[0][3] = 0.0f;

        M.m_matrix[1][0] = u.y;
        M.m_matrix[1][1] = v.y;
        M.m_matrix[1][2] = w.y;
        M.m_matrix[1][3] = 0.0f;

        M.m_matrix[2][0] = u.z;
        M.m_matrix[2][1] = v.z;
        M.m_matrix[2][2] = w.z;
        M.m_matrix[2][3] = 0.0f;

        -q;//Negate Q

        M.m_matrix[3][0] = MLVectorDot(q, u);
        M.m_matrix[3][1] = MLVectorDot(q, v);
        M.m_matrix[3][2] = MLVectorDot(q, w);
        M.m_matrix[3][3] = 1.0f;  
    }

    // ------------------------------------------------------------------------------------------------------|
    // Functions from classes *******************************************************************************|
    // ------------------------------------------------------------------------------------------------------|
    inline float Plane::Intersection(const Ray& ray) const 
    {
        // From Mathematics for 3D Game Programming and Computer Graphics - Third Edition	
        float t = (-MLVectorDot(n, ray.origin) + d) / MLVectorDot(n, ray.direction);

        return t;
    }
}

#endif