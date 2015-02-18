#pragma once
#include <forward_list>
#include "DebugDX.h"
#include "common_data_attributes.h"
#include "MathTypes.h"
#include "3DMesh Helper.h"

using namespace omi;

struct PERGROUPDATA_MESH
{   
    ID3D11Buffer*   m_pDXIndexbuffer;
    PerObject       m_PerGroupData;
    wstring         m_wzMaterialName;
    INDEX_FORMAT    m_indexformat;
    uint            m_index_count;
    bool            m_bCanUseTextures;

    PERGROUPDATA_MESH(void) : m_pDXIndexbuffer(nullptr), m_index_count(0), m_bCanUseTextures(true){ }
    ~PERGROUPDATA_MESH(void)
    { 
        SAFE_RELEASE_COM( m_pDXIndexbuffer ); 
    }
};

class DXMesh
{
private:
    static ID3D11DeviceContext1*    m_pDeviceContext;
    static ID3D11Buffer*            m_PerObjectConstBuffer;

    ID3D11Buffer* m_vertexbuffer;
    ID3D11Buffer* m_facesnormalBuffer;    
    ID3D11VertexShader*     mVertexShader;
    ID3D11PixelShader*      mPixelShader;
    ID3D11InputLayout* m_inputLayout;

    uint m_iPolygonCount;
    uint m_facesNormalsToDraw;
    uint m_vertex_count;
    uint m_vertex_stride;    

    bool m_bvisible;
    bool m_bAddable;

    std::forward_list<PERGROUPDATA_MESH*> m_PerGroupDataList;
    vector<FaceNormal> m_FacesNormals;

public:
    DXMesh(void) : m_bAddable(true), m_bvisible(true), m_iPolygonCount(0), m_vertexbuffer(nullptr), m_facesnormalBuffer(nullptr)
    {

    }
    ~DXMesh(void) 
    { 
        EmptyPointersList(m_PerGroupDataList);
        SAFE_RELEASE_COM( m_vertexbuffer );
        SAFE_RELEASE_COM( m_facesnormalBuffer );
        mVertexShader   = nullptr;
        mPixelShader    = nullptr;
        m_inputLayout   = nullptr;
    }

    // GETTERS

    inline ID3D11Buffer* const* GetVertexBuffer(void) const { return &m_vertexbuffer; }
    inline ID3D11Buffer* GetFacesNormalBuffer(void) { return m_facesnormalBuffer; }

    inline void SetPolygonCount(uint count) { m_iPolygonCount = count; }
    inline uint GetPolygonCount(void) const { return m_iPolygonCount; }    

    inline  ID3D11InputLayout* GetInputLayout(void) const { return m_inputLayout; }    

    inline const bool& visible(void) { return m_bvisible; }

    inline const uint& GetVertexStride(void) const { return m_vertex_stride; }

    inline bool IsAddable(void) const { return m_bAddable; }

    // SETTERS    
    inline void SetVertexShader(ID3D11VertexShader* Shader) { mVertexShader = Shader; }
    inline void SetPixelShader(ID3D11PixelShader* Shader) { mPixelShader = Shader; }

    inline void SetVertexBuffer(ID3D11Buffer* vbuffer) { m_vertexbuffer = vbuffer; }
    inline void SetFacesNormalBuffer(ID3D11Buffer* vbuffer) { m_facesnormalBuffer = vbuffer; }
    inline void SetVertexStride(const uint& stride) { m_vertex_stride = stride; }      

    inline void SetInputLayout(ID3D11InputLayout* IL) { m_inputLayout = IL; }

    inline void isNotAddable(void) { m_bAddable = false; }

    inline void SetPerPolygonGroupData(std::forward_list<PERGROUPDATA_MESH*>& pergroupdatalist) { m_PerGroupDataList = pergroupdatalist; }
    inline void AddPolygonGroup(PERGROUPDATA_MESH* _perGroupData) { m_PerGroupDataList.push_front( _perGroupData ); }

    void UseTextures(bool yesOrNo);

    // STATIC FUNCTIONS
    static void SetContext(ID3D11DeviceContext1* pDeviceContext) { m_pDeviceContext = pDeviceContext; }
    static void SetPerObjectCBuffer(ID3D11Buffer* cbPerObject) { m_PerObjectConstBuffer = cbPerObject; }

    // DRAW FUNCTIONS
    void DrawMe(void) const;
    void DrawFacesNormals(void) const;
    void SetFacesNormals(const vector<FaceNormal> _facesnormal) { m_FacesNormals = _facesnormal; }
    const vector<FaceNormal>& GetFacesNormals(void) const { return m_FacesNormals; }
    void SetFacesNormalToDrawCount(uint _count) { m_facesNormalsToDraw = _count; }

    Material& GetMaterial(const wstring& _material_name);
};