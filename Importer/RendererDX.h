#pragma once
#include "DebugDX.h"// This header file includes all DirectX's required headers
#include "Light_sources.h"
#include "Cam.h"
#include "Mesh.h"
#include "DXMesh.h"
#include "IUIElement.h"
#include <DirectXTex.h>
#include <DDSTextureLoader.h>

const uint MAX_SHADER_NUMBERS = 4;

class RendererDx
{
    bool m_bShowGrid;
    bool m_bShowAxis;
    bool m_buseTextures;
private:

#pragma region DirectX Variables
    // Device, Context, swapchain
    ID3D11Device1* m_pDxDevice;
    ID3D11DeviceContext1* m_pDxContext;
    IDXGISwapChain1* m_pDxSwapchain;

    // Back buffer view, depth-stencil view
    ID3D11RenderTargetView*	m_pDxRenderTargetView;
    ID3D11DepthStencilView*	m_pDxDepthStencilView;  

    // View Port(s)
    D3D11_VIEWPORT      m_DxViewPort;

    // Input Layouts
    ID3D11InputLayout* m_InputLayout_POS_NORM;
    ID3D11InputLayout* m_InputLayout_POS_TEX_NORM;
    ID3D11InputLayout* m_InputLayout_POS_TEX;
    ID3D11InputLayout* m_InputLayout_POS_TEX_NORM_BTAN;
    ID3D11InputLayout* m_InputLayout_POS;

    // Constant buffers
    ID3D11Buffer*    m_pDxConstBuffer_perFrame;
    ID3D11Buffer*    m_pDxConstBuffer_Lights;
    ID3D11Buffer*    m_pDxConstBuffer_perObject;
    ComPtr<ID3D11Buffer>    m_pDxConstBuffer_color;
    
    ComPtr<ID3D11Buffer>    m_pDxVertexBuffer_Grid;
    ComPtr<ID3D11Buffer>    m_pDxVertexBuffer_WorldAxes;
    ComPtr<ID3D11Buffer>    m_pDxVertexBuffer_WorldAxesArrows;

    ComPtr<ID3D11Buffer>    m_pDxIndexBuffer_WorldAxes;
    ComPtr<ID3D11Buffer>    m_pDxIndexBuffer_WorldAxesArrows;

    // Depth-Stencil buffer
    ID3D11Texture2D* m_pDxDepthStencilBuff;

    // Pipeline Shader programs
    ID3D11VertexShader* m_VS_POS_TEX_NORM;
    ID3D11VertexShader* m_VS_POS_NORM;
    ID3D11VertexShader* m_VS_POS_TEX_NORM_BTAN;
    ID3D11VertexShader* m_VS_FacesNormal;

    //ComPtr<ID3D11PixelShader> m_PS_POS_TEX_NORM_BTAN;
    ID3D11PixelShader* m_PS_COLOR;
    ID3D11PixelShader* m_PS_POS_NORM;
    ID3D11PixelShader* m_PS_FacesNormal;

    //
    ID3D11PixelShader** m_PS_POS_TEX_NORM;
    ID3D11PixelShader** m_PS_POS_TEX_NORM_BITAN;

    // Textures
    ID3D11ShaderResourceView*	m_pDxTexture2DArray;

    // Texture Sampler State
    ID3D11SamplerState*  m_pDxSampler; 

    // Rasterizer states
    ID3D11RasterizerState1*	m_pDxRasterState_AntiAliasing;
    ComPtr<ID3D11RasterizerState1>	m_pDxRasterState_NoCulling;

    // Blend States
    ID3D11BlendState*    m_pDxBlendState_Transparent;  

    // Depth/Stencil states
    ID3D11DepthStencilState*	m_pDxDepthStencilState;

    ID3D11ShaderResourceView** m_pTextureResource;


#pragma endregion

    uint m_iQualityLevel;
    uint m_iSamplecount;
    bool m_bInitialized;
    bool m_bLostDevice;
    bool m_bShowNormals;
    HWND m_hWnd;// To use when device needs to be (re)created
    int m_iWidth, m_iHeight;
    uint m_iPiecesToBeDrawn;
    Camera_FP* m_pCamera;
    forward_list<DXMesh*> m_MeshList;
    AABB m_aabb;
    bool m_bFocusOnMesh;
    uint* m_texture_resolution_index;

    // Light Movement variables
    float m_lightPitch;
    float m_lightHead;
    float m_lightRadius;
    DirectionalLight m_DirLight;

private:
    // Private functions
    bool CreateConstantBuffers(void);

    ID3D11Buffer* CreateMeshVertexBuffer(const Mesh* Mesh);
    ID3D11Buffer* CreateMeshIndexBuffer(const void* pIndices, INDEX_FORMAT format, uint count);
    ID3D11Buffer* CreateFacesNormalBuffer(const Mesh* Mesh);
    
    void CompileShadersAndInputLayouts(void);

    bool VCreateVertexBufferAABB(const AABB& _aabb);
    // Input-Layouts functions
    void CreateInputlayout(const void* _compiledShader, uint _size);
    void UpdatePerFrameConstBuffer(void);

    CompiledShader* GetShaderByteCode(wchar_t* file_Name);
    void CreateRasterAndBlendStates(void);
    void CreateTexturesAndSamplers(void);
    void DrawAABBs(void);

    ID3D11InputLayout* GetInputlayout(const DATA_FORMAT& F);

    void ProcessPerGroupData(const forward_list<PERGROUPDATA*>&, DXMesh& pDxMesh);

    void AddTexturesToPipeline(const vector<unique_ptr<DirectX::Blob>>&, uint);
    
    void CreateWorldAxes(float x, float y, float z);
    void CreateGrid(float x, float z);

    void DrawWorldAxis(void);
public:
    RendererDx(void);
    ~RendererDx(void);

    bool InitializeDeviceAndContext(void);
    bool InitializeSwapChain(const HWND& hwnd, int height, int width);

    /*******************************************/
    // These functions are the basis for the user interface implementation 2.0
    
    void AddUIElement(IUIElement* _ptrUIElement);
    
    /*******************************************/
    
    
    // Inherited Functions
    void VClearBackGround(const float* _color4);
    void VOnLostDevice(void) {}
    void VOnRestore(void) {}
    void VOnResize(int w, int h);

    bool AddMesh(const Mesh* pMesh);
    inline const AABB& GetAABB(void) { return m_aabb; }
    // DRAW FUNCTIONS
    void VDrawScene(void);
    void PresentScene(void);

    inline void SetCameraMovementSpeed(int value) { m_pCamera->SetMovementSpeed(value); }
    inline void SetCameraZoomSpeed(int value) { m_pCamera->SetZoomSpeed(value); }
    inline void SetLookAtPoint(const float4& _target) { m_pCamera->SetLookAtPoint(_target); }
    inline void ZoomIn(void) { m_pCamera->ZoomIn(); MoveCamera(0, 0); UpdatePerFrameConstBuffer(); }
    inline void ZoomOut(void) { m_pCamera->ZoomOut(); MoveCamera(0, 0); UpdatePerFrameConstBuffer(); }
    
    void MoveCamera(const int vertically, const int horizontally);
    void MoveLight(int vertically, int horizontally);

    IDXGIDevice2* GetDXGIDevice(void);
    IDXGISwapChain1* GetSwapChain(void);
    bool InitializeResources(void);

    void LightFollowCamera(void);
    void StopFollowingCamera(void);

    void WireFrameMode(void);
    void BackFaceCulling(void);
    void TexturesOnOff(void);
    void DeleteMesh(void);
    uint GetTriangleCount(void);

    void ShowNormals(void) { m_bShowNormals =  m_bShowNormals ? false : true; }
    void ShowWorldGrid(void) { m_bShowGrid = m_bShowGrid ? false : true; }
    void ShowWorldAxes(void) { m_bShowAxis = m_bShowAxis ? false : true; }
    Material& GetMaterial(const wstring& _material_name);

    void FocusOnMesh(void);
    void CalCulateVisibleFacesNormals(void);
};