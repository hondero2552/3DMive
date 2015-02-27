#include "RendererDX.h"
#include "math_funcs.h"
#include "RoundButton.h"
#include "SquareButton.h"

using namespace DirectX;
using std::unique_ptr;

RendererDx::RendererDx() :
m_hWnd(0),
m_bInitialized(false),
m_bLostDevice(false),
m_bShowNormals(false),
m_bShowGrid(true),
m_bShowAxis(true),
m_bFocusOnMesh(true),
m_buseTextures(true),

//With and height of the Window
m_iWidth(0),
m_iHeight(0),
m_pCamera(nullptr),

//backbuffer quality
m_iQualityLevel(0),
m_iSamplecount(4), ///////////////////////////////////////////////////// needs to be improved
m_iPiecesToBeDrawn(0),

m_texture_resolution_index(nullptr),

m_pTextureResource(nullptr),
m_PS_POS_TEX_NORM(nullptr),
m_PS_POS_TEX_NORM_BITAN(nullptr),

// DirectX variables
m_pDxDepthStencilState(nullptr),
m_pDxBlendState_Transparent(nullptr),
m_pDxRasterState_AntiAliasing(nullptr),
m_pDxSampler(nullptr),
m_pDxTexture2DArray(nullptr),
m_PS_FacesNormal(nullptr),
m_PS_POS_NORM(nullptr),
m_PS_COLOR(nullptr),
m_VS_FacesNormal(nullptr),
m_VS_POS_TEX_NORM_BTAN(nullptr),
m_VS_POS_NORM(nullptr),
m_VS_POS_TEX_NORM(nullptr),
m_pDxDepthStencilBuff(nullptr),
m_pDxConstBuffer_perObject(nullptr),
m_pDxConstBuffer_Lights(nullptr),
m_pDxConstBuffer_perFrame(nullptr),
m_InputLayout_POS(nullptr),
m_InputLayout_POS_TEX_NORM_BTAN(nullptr),
m_InputLayout_POS_TEX(nullptr),
m_InputLayout_POS_TEX_NORM(nullptr),
m_InputLayout_POS_NORM(nullptr),
m_pDxRenderTargetView(nullptr),
m_pDxDepthStencilView(nullptr),
m_pDxSwapchain(nullptr),
m_pDxContext(nullptr),
m_pDxDevice(nullptr)
{

}

RendererDx::~RendererDx(void)
{
    // Release all the DXMesh ptrs
    for_each(m_MeshList.begin(), m_MeshList.end(), [&](DXMesh*& pMesh)
    {
        SAFE_DELETE(pMesh);
    });

    SAFE_DELETE(m_pCamera);
    for (size_t i = 0; i < MAX_SHADER_NUMBERS; ++i)
    {
        SAFE_RELEASE_COM(m_pTextureResource[i]);
        SAFE_RELEASE_COM(m_PS_POS_TEX_NORM[i]);
        SAFE_RELEASE_COM(m_PS_POS_TEX_NORM_BITAN[i]);
    }

    SAFE_DELETE_ARRAY(m_PS_POS_TEX_NORM);
    SAFE_DELETE_ARRAY(m_pTextureResource);
    SAFE_DELETE_ARRAY(m_PS_POS_TEX_NORM_BITAN);
    SAFE_DELETE_ARRAY(m_texture_resolution_index);

    SAFE_RELEASE_COM(m_pDxDepthStencilState);
    SAFE_RELEASE_COM(m_pDxBlendState_Transparent);
    SAFE_RELEASE_COM(m_pDxRasterState_AntiAliasing);
    SAFE_RELEASE_COM(m_pDxSampler);
    SAFE_RELEASE_COM(m_pDxTexture2DArray);
    SAFE_RELEASE_COM(m_PS_FacesNormal);
    SAFE_RELEASE_COM(m_PS_POS_NORM);
    SAFE_RELEASE_COM(m_PS_COLOR);
    SAFE_RELEASE_COM(m_VS_FacesNormal);
    SAFE_RELEASE_COM(m_VS_POS_TEX_NORM_BTAN);
    SAFE_RELEASE_COM(m_VS_POS_NORM);
    SAFE_RELEASE_COM(m_VS_POS_TEX_NORM);
    SAFE_RELEASE_COM(m_pDxDepthStencilBuff);
    SAFE_RELEASE_COM(m_pDxConstBuffer_perObject);
    SAFE_RELEASE_COM(m_pDxConstBuffer_Lights);
    SAFE_RELEASE_COM(m_pDxConstBuffer_perFrame);
    SAFE_RELEASE_COM(m_InputLayout_POS);
    SAFE_RELEASE_COM(m_InputLayout_POS_TEX_NORM_BTAN);
    SAFE_RELEASE_COM(m_InputLayout_POS_TEX);
    SAFE_RELEASE_COM(m_InputLayout_POS_TEX_NORM);
    SAFE_RELEASE_COM(m_InputLayout_POS_NORM);
    SAFE_RELEASE_COM(m_pDxRenderTargetView);
    SAFE_RELEASE_COM(m_pDxDepthStencilView);
    SAFE_RELEASE_COM(m_pDxSwapchain);
    SAFE_RELEASE_COM(m_pDxContext);
    SAFE_RELEASE_COM(m_pDxDevice);
}

bool RendererDx::InitializeResources(void)
{

    m_pCamera = new Camera_FP(m_iWidth, m_iHeight);                                                                                                  // ADD CODE TO CATCH BAD-ALLOC EXCEPTION

    m_pCamera->VInitCamera(
        float3(MLConvertToRadians(-25.0f), MLConvertToRadians(170.0f), 250.0),  // Position
        float4(0.0f, 0.0f, 0.0f, 1.0f),                                         // Look-At
        45.0f,                                                                  // Degrees(FoV)
        static_cast<float>(m_iWidth) / m_iHeight                                  // Ratio 
        );

    // Create the rasterizer states, i.e. antialiasing, transparent, etc
    CreateRasterAndBlendStates();

    CreateConstantBuffers();

    CreateTexturesAndSamplers();

    UpdatePerFrameConstBuffer();

    // Allocate memory
    m_pTextureResource          = new ID3D11ShaderResourceView*[MAX_SHADER_NUMBERS];
    m_PS_POS_TEX_NORM           = new ID3D11PixelShader*[MAX_SHADER_NUMBERS];
    m_PS_POS_TEX_NORM_BITAN     = new ID3D11PixelShader*[MAX_SHADER_NUMBERS];
    m_texture_resolution_index  = new uint[MAX_SHADER_NUMBERS];

    for (size_t i = 0; i < MAX_SHADER_NUMBERS; ++i)
    {
        m_pTextureResource[i]           = nullptr;
        m_PS_POS_TEX_NORM[i]            = nullptr;
        m_PS_POS_TEX_NORM_BITAN[i]      = nullptr;
        m_texture_resolution_index[i]   = 0;
    }

    // Compile all the shaders now that the device and context have been created
    CompileShadersAndInputLayouts();

    CreateWorldAxes(100.0f, 100.0f, 100.0f);
    CreateGrid(100.0f, 100.0f);
    return true;
}

void RendererDx::VOnResize(int w, int h)
{
    m_iWidth    = w;
    m_iHeight   = h;

    // Make sure the device is Valid
    assert(m_pDxContext);
    assert(m_pDxDevice);
    assert(m_pDxSwapchain);

    // Release the old views, as they hold references to the buffers we
    // will be destroying.  Also release the old depth/stencil buffer.
    SAFE_RELEASE_COM(m_pDxRenderTargetView);
    SAFE_RELEASE_COM(m_pDxDepthStencilView);
    SAFE_RELEASE_COM(m_pDxDepthStencilBuff);

    // Resize the swap chain and recreate the render target view.
    ID3D11Texture2D* pBackBuffer2DTex = nullptr;

    HRESULT hr = m_pDxSwapchain->ResizeBuffers(2, m_iWidth, m_iHeight, DXGI_FORMAT_B8G8R8A8_UNORM, NULL);
    hr = m_pDxSwapchain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer2DTex));
    hr = m_pDxDevice->CreateRenderTargetView(pBackBuffer2DTex, NULL, &m_pDxRenderTargetView);

    // Release the temp COM object
    SAFE_RELEASE_COM(pBackBuffer2DTex);

    // Create depth/Stencil buffer and view to it
    D3D11_TEXTURE2D_DESC DepthStencilDesc;
    SecureZeroMemory(&DepthStencilDesc, sizeof(D3D11_TEXTURE2D_DESC));

    DepthStencilDesc.Width = m_iWidth;
    DepthStencilDesc.Height = m_iHeight;
    DepthStencilDesc.MipLevels = 1;
    DepthStencilDesc.ArraySize = 1;
    DepthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
    DepthStencilDesc.SampleDesc.Count = m_iSamplecount;
    DepthStencilDesc.SampleDesc.Quality = m_iQualityLevel;
    DepthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
    DepthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    hr = m_pDxDevice->CreateTexture2D(&DepthStencilDesc, NULL, &m_pDxDepthStencilBuff);

    D3D11_DEPTH_STENCIL_VIEW_DESC desc;
    desc.Format = DXGI_FORMAT_D32_FLOAT;
    desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
    desc.Flags = 0;
    desc.Texture2D.MipSlice = 0;
    hr = m_pDxDevice->CreateDepthStencilView(m_pDxDepthStencilBuff, &desc, &m_pDxDepthStencilView);

    //-----------------------------|
    // Create and set the view port|
    //-----------------------------|
    m_DxViewPort.TopLeftX = 0.0f;
    m_DxViewPort.TopLeftY = 0.0f;
    m_DxViewPort.Width = static_cast<float>(m_iWidth);
    m_DxViewPort.Height = static_cast<float>(m_iHeight);
    m_DxViewPort.MinDepth = 0.0f;
    m_DxViewPort.MaxDepth = 1.0f;

    // Set the viewport, rendertarget and depth stencil buffer
    m_pDxContext->RSSetViewports(1, &m_DxViewPort);
    m_pDxContext->OMSetRenderTargets(1, &m_pDxRenderTargetView, m_pDxDepthStencilView);

    // If the camera has been initialized, update the Projection and View matrices
    if (m_pCamera != nullptr)
    {
        m_pCamera->VOnResize(m_iHeight, m_iWidth);
        UpdatePerFrameConstBuffer();
    }
}

void RendererDx::VClearBackGround(const float* color4)
{
    m_pDxContext->ClearRenderTargetView(m_pDxRenderTargetView, color4);
    m_pDxContext->ClearDepthStencilView(m_pDxDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, NULL);
}

CompiledShader* RendererDx::GetShaderByteCode(wchar_t* file_Name)
{
    CompiledShader* compiledShader = new CompiledShader;
    size_t length = 0;
    char* bytecode = OpenFile(file_Name, length, L"rb");

    if (bytecode != nullptr)
        compiledShader->SetByteCode(bytecode, length);

    return compiledShader;
}

bool RendererDx::CreateConstantBuffers(void)
{
    // PER-FRAME    
    D3D11_BUFFER_DESC bufferDesc;
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.ByteWidth = 144;
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = NULL;
    bufferDesc.MiscFlags = NULL;
    bufferDesc.StructureByteStride = NULL;

    m_pDxDevice->CreateBuffer(&bufferDesc, NULL, &m_pDxConstBuffer_perFrame);

    // Per-Object
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.ByteWidth = 80;
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = NULL;
    bufferDesc.MiscFlags = NULL;
    bufferDesc.StructureByteStride = NULL;

    m_pDxDevice->CreateBuffer(&bufferDesc, NULL, &m_pDxConstBuffer_perObject);
    DXMesh::SetPerObjectCBuffer(m_pDxConstBuffer_perObject);
    // Color for the lines being drawn
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.ByteWidth = 16;
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = NULL;
    bufferDesc.MiscFlags = NULL;
    bufferDesc.StructureByteStride = NULL;

    m_pDxDevice->CreateBuffer(&bufferDesc, NULL, m_pDxConstBuffer_color.GetAddressOf());

    // Light
    m_DirLight.Ambient = float4(1.0f, 1.0f, 1.0f, 1.0f);
    m_DirLight.Diffuse = float4(0.7f, 0.7f, 0.7f, 1.0f);
    m_DirLight.Specular = float4(0.8f, 0.8f, 0.8f, 1.0f);

    // These coordinates are used for polar coordinates. This is how the light movement is implemented
    m_lightHead = MLConvertToRadians(20.0);
    m_lightPitch = MLConvertToRadians(45.0);
    m_lightRadius = 1.0f;

    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.ByteWidth = 64;
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = NULL;
    bufferDesc.MiscFlags = NULL;
    bufferDesc.StructureByteStride = NULL;

    m_pDxDevice->CreateBuffer(&bufferDesc, NULL, &m_pDxConstBuffer_Lights);
    MoveLight(0, 0); // Just so the buffer gets properly updated

    m_pDxContext->VSSetConstantBuffers(0, 1, &m_pDxConstBuffer_perFrame);

    m_pDxContext->PSSetConstantBuffers(0, 1, &m_pDxConstBuffer_perFrame);
    m_pDxContext->PSSetConstantBuffers(1, 1, &m_pDxConstBuffer_Lights);
    m_pDxContext->PSSetConstantBuffers(2, 1, &m_pDxConstBuffer_perObject);
    m_pDxContext->PSSetConstantBuffers(3, 1, m_pDxConstBuffer_color.GetAddressOf());

    return true;
}

void RendererDx::VDrawScene(void)
{
    // Update constant buffers
    // We will only be drawing primitives as triangle-lists
    const float c = 0.63f;
    const float color[4] = { 0.58f, c, c, 1.0f };
    VClearBackGround(color);

    if (m_bShowAxis)
        DrawWorldAxis();

    if (m_bShowGrid)
    {
        const uint STRIDE = sizeof(float3);
        const uint OFFSET = 0;

        m_pDxContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

        // Set the vertex/pixel shaders, and input layouts to draw the grid
        m_pDxContext->VSSetShader(m_VS_FacesNormal, NULL, NULL);
        m_pDxContext->PSSetShader(m_PS_FacesNormal, NULL, NULL);
        m_pDxContext->IASetInputLayout(m_InputLayout_POS);

        float4 color(0.8f, 0.8f, 0.8f, 1.0f);
        m_pDxContext->UpdateSubresource(m_pDxConstBuffer_color.Get(), NULL, NULL, &color, NULL, NULL);// we are updating the color buffer used to draw the grid, axis, normals, and other small elements

        m_pDxContext->IASetVertexBuffers(1, 1, m_pDxVertexBuffer_Grid.GetAddressOf(), &STRIDE, &OFFSET);

        m_pDxContext->Draw(76, NULL);
    }

    // This is where we draw all the meshes in the scene that the user has imported*/
    m_pDxContext->RSSetState(m_pDxRasterState_AntiAliasing);
    for (auto i = m_MeshList.begin(); i != m_MeshList.end(); ++i)
    {
        // Set the blend state so trasparent meshes are rendered properly
        FLOAT factors[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        m_pDxContext->OMSetBlendState(m_pDxBlendState_Transparent, factors, 0xffffffff);

        const DXMesh* pMesh = *i;
        pMesh->DrawMe();

        // Reset the blendstates
        m_pDxContext->OMSetBlendState(nullptr, factors, 0xffffffff);

        // Draw the normals only if the user has this setting enabled in the context menu of the UI.
        if (m_bShowNormals)
        {
            m_pDxContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

            // Set the Shader programs and input layout for the mesh
            m_pDxContext->VSSetShader(m_VS_FacesNormal, NULL, NULL);
            m_pDxContext->PSSetShader(m_PS_FacesNormal, NULL, NULL);
            m_pDxContext->IASetInputLayout(m_InputLayout_POS);

            // update color constant buffer
            float4 color(0.0f, 0.5f, 1.0f, 1.0f);
            m_pDxContext->UpdateSubresource(m_pDxConstBuffer_color.Get(), NULL, NULL, &color, NULL, NULL);// we are updating the color buffer used to draw the grid, axis, normals, and other small elements

            // Set the index and vertex buffers
            pMesh->DrawFacesNormals();
        }
    }
}

void RendererDx::CreateRasterAndBlendStates(void)
{
    assert(m_pDxContext);
    // Antialiasing raster-state
    D3D11_RASTERIZER_DESC1 rs;
    SecureZeroMemory(&rs, sizeof(D3D11_RASTERIZER_DESC1));
    rs.FillMode = D3D11_FILL_SOLID;
    rs.CullMode = D3D11_CULL_BACK;
    rs.FrontCounterClockwise = false;
    rs.DepthClipEnable = true;
    rs.MultisampleEnable = true;

    m_pDxDevice->CreateRasterizerState1(&rs, &m_pDxRasterState_AntiAliasing);

    rs.CullMode = D3D11_CULL_NONE;
    m_pDxDevice->CreateRasterizerState1(&rs, m_pDxRasterState_NoCulling.GetAddressOf());
    m_pDxContext->RSSetState(m_pDxRasterState_AntiAliasing);

    // ----------------------------------|
    // Blend State  move this out of here|
    // ----------------------------------|
    D3D11_BLEND_DESC bd;

    bd.AlphaToCoverageEnable    = false;
    bd.IndependentBlendEnable   = false;

    bd.RenderTarget[0].BlendEnable              = true;
    bd.RenderTarget[0].SrcBlend                 = D3D11_BLEND_SRC_ALPHA;
    bd.RenderTarget[0].DestBlend                = D3D11_BLEND_INV_SRC_ALPHA;
    bd.RenderTarget[0].BlendOp                  = D3D11_BLEND_OP_ADD;
    bd.RenderTarget[0].SrcBlendAlpha            = D3D11_BLEND_ONE;
    bd.RenderTarget[0].DestBlendAlpha           = D3D11_BLEND_ZERO;
    bd.RenderTarget[0].BlendOpAlpha             = D3D11_BLEND_OP_ADD;
    bd.RenderTarget[0].RenderTargetWriteMask    = D3D11_COLOR_WRITE_ENABLE_ALL;

    m_pDxDevice->CreateBlendState(&bd, &m_pDxBlendState_Transparent);
}

void RendererDx::CreateTexturesAndSamplers()
{
    D3D11_SAMPLER_DESC sampler;
    SecureZeroMemory(&sampler, sizeof(D3D11_SAMPLER_DESC));

    sampler.Filter = D3D11_FILTER_ANISOTROPIC;
    sampler.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampler.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampler.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampler.MaxAnisotropy = 8;
    sampler.ComparisonFunc = D3D11_COMPARISON_NOT_EQUAL;

    m_pDxDevice->CreateSamplerState(&sampler, &m_pDxSampler);

    m_pDxContext->PSSetSamplers(0, 1, &m_pDxSampler);
}

void RendererDx::UpdatePerFrameConstBuffer(void)
{
    // Update the constant buffer
    struct PerFrame
    {
        Matrix ViewProjTransform, ViewTransform;
        float4 eyePos;
    };
    PerFrame cb;

    cb.ViewProjTransform = m_pCamera->VGetViewProjectionMatrix();
    cb.eyePos = m_pCamera->VGetEyePosition();
    cb.ViewTransform = m_pCamera->VGetViewMatrix();

    MLMatrixTranspose(cb.ViewProjTransform);
    MLMatrixTranspose(cb.ViewTransform);

    m_pDxContext->UpdateSubresource(m_pDxConstBuffer_perFrame, NULL, NULL, &cb, NULL, NULL);
}

bool RendererDx::AddMesh(const Mesh* pMesh)
{
    m_aabb = pMesh->GetAxisAlignedBoundingBox();
    ID3D11Buffer* lBuffer = nullptr;
    unique_ptr<DXMesh> pDxMesh(new DXMesh());

    // Set Common settings for all Meshes: material, indices/vertices count, index/vertex format and stride, and inputlayout
    pDxMesh->SetVertexStride(pMesh->GetVertexStride());
    pDxMesh->SetInputLayout(GetInputlayout(pMesh->GetVertexFormat()));

    // Create/Set the vertices and indices buffers for the Mesh
    lBuffer = CreateMeshVertexBuffer(pMesh);
    if (!lBuffer)
        return false;
    pDxMesh->SetVertexBuffer(lBuffer);

    // Set the faces normal buffer
    lBuffer = nullptr; // reset the local COM object, without releasing it    
    lBuffer = CreateFacesNormalBuffer(pMesh);
    if (!lBuffer)
        return false;

    pDxMesh->SetFacesNormalBuffer(lBuffer);
    pDxMesh->SetFacesNormals(pMesh->GetFacesNormal());
    pDxMesh->SetPolygonCount(pMesh->GetPlygonCount());

    // Load appropiate shaders and process the Materials/Indices per polygon groups
    ProcessPerGroupData(pMesh->GetPerPolygonGroupData(), *pDxMesh);

    // Only add the mesh if it's addable; i.e. it has no erros that will prevent the proper rendering
    if (pDxMesh->IsAddable())
    {

        const float AABB_LENGTH = MLVectorLenght(
            (pMesh->GetAxisAlignedBoundingBox().m_highest - pMesh->GetAxisAlignedBoundingBox().m_lowest)
            );

        // Set the minimum radius as the distance between the highest and lowest points of the Mesh's AABB
        m_pCamera->SetMinimumRadius((AABB_LENGTH / 2.0f));

        // Set the look-at point as the center of the Mesh's bounding box if necessary
        if (m_bFocusOnMesh)
        {
            SetLookAtPoint(m_aabb.GetCenterPoint());
            m_pCamera->SetRadius((AABB_LENGTH));
            ZoomOut();
        }

        // Finally add the mesh
        m_MeshList.push_front(pDxMesh.release());

        // We call this function here so we can fill up the dynamic vertex buffer for the faces normals feature
        CalCulateVisibleFacesNormals();
        return true;
    }
    else
        return false;
}

ID3D11Buffer* RendererDx::CreateMeshVertexBuffer(const Mesh* pMesh)
{
    ID3D11Buffer* lVB = nullptr;

    // Fill in a buffer description.
    D3D11_BUFFER_DESC bufferDesc;
    bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    bufferDesc.ByteWidth = pMesh->GetVertexStride() * pMesh->GetVertexCount();
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = NULL;

    // Fill in the subresource data.
    D3D11_SUBRESOURCE_DATA InitData;
    InitData.pSysMem = pMesh->GetVertexData();

    InitData.SysMemPitch = 0;
    InitData.SysMemSlicePitch = 0;

    // Create the vertex buffer
    m_pDxDevice->CreateBuffer(&bufferDesc, &InitData, &lVB);

    return lVB;
}

ID3D11Buffer* RendererDx::CreateFacesNormalBuffer(const Mesh* pMesh)
{
    ID3D11Buffer* lVB = nullptr;

    // Fill in a buffer description.
    D3D11_BUFFER_DESC bufferDesc;
    bufferDesc.Usage            = D3D11_USAGE_DYNAMIC;
    bufferDesc.ByteWidth        = (sizeof(float3)) * (pMesh->GetPlygonCount() * 2);
    bufferDesc.BindFlags        = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags   = D3D11_CPU_ACCESS_WRITE;
    bufferDesc.MiscFlags        = 0;
    bufferDesc.StructureByteStride = NULL;

    // Create the vertex buffer
    m_pDxDevice->CreateBuffer(&bufferDesc, nullptr, &lVB);

    return lVB;
}

ID3D11Buffer* RendererDx::CreateMeshIndexBuffer(const void* pIndices, INDEX_FORMAT format, uint count)
{
    ID3D11Buffer* lVB = nullptr;
    const uint INDEX_STRIDE = format == SHORT_TYPE ? sizeof(unsigned short) : sizeof(uint);

    // Fill in a buffer description.
    D3D11_BUFFER_DESC bufferDesc;
    bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    bufferDesc.ByteWidth = INDEX_STRIDE * count;
    bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = NULL;

    // Fill in the subresource data.
    D3D11_SUBRESOURCE_DATA InitData;
    InitData.pSysMem = pIndices;
    InitData.SysMemPitch = 0;
    InitData.SysMemSlicePitch = 0;

    // Create the vertex buffer
    m_pDxDevice->CreateBuffer(&bufferDesc, &InitData, &lVB);

    return lVB;
}

void RendererDx::CompileShadersAndInputLayouts(void)
{
    unique_ptr<CompiledShader> shader(nullptr);

    // -------------------------|
    // VERTEX SHADERS ----------|
    // -------------------------|

    // POS_NORM
    D3D11_INPUT_ELEMENT_DESC POS_NORM [] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, NULL },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, NULL },
    };
    shader.reset(GetShaderByteCode(L"Shaders\\VS_PN.cso"));
    m_pDxDevice->CreateVertexShader(shader->GetByteCode(), shader->length(), NULL, &m_VS_POS_NORM);
    m_pDxDevice->CreateInputLayout(POS_NORM, ARRAYSIZE(POS_NORM), shader->GetByteCode(), shader->length(), &m_InputLayout_POS_NORM);

    // POS_TEX_NORM
    D3D11_INPUT_ELEMENT_DESC POS_TEX_NORM [] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, NULL },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, NULL },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, NULL },
    };
    shader.reset(GetShaderByteCode(L"Shaders\\VS_PTN.cso"));
    m_pDxDevice->CreateVertexShader(shader->GetByteCode(), shader->length(), NULL, &m_VS_POS_TEX_NORM);
    m_pDxDevice->CreateInputLayout(POS_TEX_NORM, ARRAYSIZE(POS_TEX_NORM), shader->GetByteCode(), shader->length(), &m_InputLayout_POS_TEX_NORM);

    // POS_TEX_NORM_BITAN 
    D3D11_INPUT_ELEMENT_DESC POS_TEX_NORM_BITAN [] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, NULL },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, NULL },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, NULL },
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, NULL }
    };
    shader.reset(GetShaderByteCode(L"Shaders\\VS_PTNB.cso"));
    m_pDxDevice->CreateVertexShader(shader->GetByteCode(), shader->length(), NULL, &m_VS_POS_TEX_NORM_BTAN);
    m_pDxDevice->CreateInputLayout(POS_TEX_NORM_BITAN, ARRAYSIZE(POS_TEX_NORM_BITAN), shader->GetByteCode(), shader->length(), &m_InputLayout_POS_TEX_NORM_BTAN);

    // FACES_NORMAL
    D3D11_INPUT_ELEMENT_DESC POS [] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, NULL }
    };
    shader.reset(GetShaderByteCode(L"Shaders\\VS_FacesNormal.cso"));
    m_pDxDevice->CreateVertexShader(shader->GetByteCode(), shader->length(), NULL, &m_VS_FacesNormal);
    m_pDxDevice->CreateInputLayout(POS, ARRAYSIZE(POS), shader->GetByteCode(), shader->length(), &m_InputLayout_POS);

    // -------------------------|
    // PIXEL SHADERS -----------|
    // -------------------------|

    // FACES_NORMAL
    shader.reset(GetShaderByteCode(L"Shaders\\PS_FacesNormal.cso"));
    m_pDxDevice->CreatePixelShader(shader->GetByteCode(), shader->length(), NULL, &m_PS_FacesNormal);

    // POS_NORM
    shader.reset(GetShaderByteCode(L"Shaders\\PS_PN.cso"));
    m_pDxDevice->CreatePixelShader(shader->GetByteCode(), shader->length(), NULL, &m_PS_POS_NORM);

    // POS_TEX_NORM
    shader.reset(GetShaderByteCode(L"Shaders\\PS_PTN256.cso"));
    m_pDxDevice->CreatePixelShader(shader->GetByteCode(), shader->length(), NULL, &m_PS_POS_TEX_NORM[RES_256]);

    shader.reset(GetShaderByteCode(L"Shaders\\PS_PTN512.cso"));
    m_pDxDevice->CreatePixelShader(shader->GetByteCode(), shader->length(), NULL, &m_PS_POS_TEX_NORM[RES_512]);

    shader.reset(GetShaderByteCode(L"Shaders\\PS_PTN1024.cso"));
    m_pDxDevice->CreatePixelShader(shader->GetByteCode(), shader->length(), NULL, &m_PS_POS_TEX_NORM[RES_1024]);

    shader.reset(GetShaderByteCode(L"Shaders\\PS_PTN2048.cso"));
    m_pDxDevice->CreatePixelShader(shader->GetByteCode(), shader->length(), NULL, &m_PS_POS_TEX_NORM[RES_2048]);

    // POS_TEX_NORM_TAN
    shader.reset(GetShaderByteCode(L"Shaders\\PS_PTNB256.cso"));
    m_pDxDevice->CreatePixelShader(shader->GetByteCode(), shader->length(), NULL, &m_PS_POS_TEX_NORM_BITAN[RES_256]);

    shader.reset(GetShaderByteCode(L"Shaders\\PS_PTNB512.cso"));
    m_pDxDevice->CreatePixelShader(shader->GetByteCode(), shader->length(), NULL, &m_PS_POS_TEX_NORM_BITAN[RES_512]);

    shader.reset(GetShaderByteCode(L"Shaders\\PS_PTNB1024.cso"));
    m_pDxDevice->CreatePixelShader(shader->GetByteCode(), shader->length(), NULL, &m_PS_POS_TEX_NORM_BITAN[RES_1024]);

    shader.reset(GetShaderByteCode(L"Shaders\\PS_PTNB2048.cso"));
    m_pDxDevice->CreatePixelShader(shader->GetByteCode(), shader->length(), NULL, &m_PS_POS_TEX_NORM_BITAN[RES_2048]);
}

ID3D11InputLayout* RendererDx::GetInputlayout(const DATA_FORMAT& F)
{
    switch (F)
    {
    case POS3_NORM3:
        return m_InputLayout_POS_NORM;

    case POS3_TEX2:
        return m_InputLayout_POS_TEX;

    case POS3_TEX2_NORM3:
        return m_InputLayout_POS_TEX_NORM;

    case POS3_TEX2_NORM3_BITAN4:
        return m_InputLayout_POS_TEX_NORM_BTAN;
    }
    return nullptr;
}

void RendererDx::ProcessPerGroupData(const forward_list<PERGROUPDATA*>& _groupdata, DXMesh& pDxMesh)
{
    vector<TexDimension>                lTexturesNameAndDimention;
    forward_list<PERGROUPDATA_MESH*>    lPerGroupDatalist;
    vector<unique_ptr<DirectX::Blob>>   pDDSTextures;
    pDDSTextures.reserve(12);

    bool bNormalMap, bLightMap, bDiffuseMap;
    bNormalMap = bLightMap = bDiffuseMap = false;

    size_t texture_resolution = 0;

    for (auto iter_current = _groupdata.begin(); iter_current != _groupdata.end(); ++iter_current)
    {
        const PERGROUPDATA* perGroupData = *iter_current;
        ID3D11Buffer* lpIndexBuffer = nullptr;

        // Mark the texture flags for the Mesh at the beginning in case they are not found
        for_each(perGroupData->mTexturesList.begin(), perGroupData->mTexturesList.end(), [&](Texture* _pTexture)
        {
            const TEXTURE_TYPE type = _pTexture->GetTextureType();
            if (type == TEXTURE_TYPE::DIFFUSE_MAP)
                bDiffuseMap = true;
            else if (type == TEXTURE_TYPE::LIGHT_MAP)
                bLightMap = true;
            else if (type == TEXTURE_TYPE::NORMAL_MAP_TANGENT)
                bNormalMap = true;
        });

        // First create a PERGROUPDATA_MESH specific for DXMesh variables and push it on the Mesh's list
        unique_ptr<PERGROUPDATA_MESH> lPolygonGroupData(new PERGROUPDATA_MESH());
        for (auto iter_text = perGroupData->mTexturesList.begin(); iter_text != perGroupData->mTexturesList.end(); ++iter_text)
        {
            const Texture* pTexture = (*iter_text);
            TexDimension texture;
            ScratchImage img_FromFile, img_dx_format, img_mipmapped, img_final;

            // This is the path where textures will be saved (local to the application)
            texture.mbfound = false;
            texture.mName = L"Models\\";
            wchar_t* filename = GetTextureName(pTexture->GetTextureFullPath());
            texture.mName += filename;
            texture.mName += L".dds";

            // Add the texture to the vector of texture names and dimensions so we can ensure that all 
            // the textures are of the same dimension at the end of THIS inner for-loop
            lTexturesNameAndDimention.push_back(texture);

            SAFE_DELETE_ARRAY(filename);

            // Get the format of the texture and try to load the file from disk if it still exists
            wstring format = GetTextureFileFormat(pTexture->GetTextureFullPath());

            HRESULT hr = S_OK;
            if (format == L"tga" || format == L"TGA")
            {
                hr = LoadFromTGAFile(pTexture->GetTextureFullPath().c_str(), nullptr, img_FromFile);
                hr = LoadFromTGAFile(pTexture->GetTextureFullPath().c_str(), nullptr, img_dx_format);
            }
            else
                hr = LoadFromWICFile(pTexture->GetTextureFullPath().c_str(), WIC_FLAGS_NONE, nullptr, img_FromFile);

            // If ANY texture file isn't found then make sure no textures are used for this group
            if (FAILED(hr))
            {
                lPolygonGroupData->m_bCanUseTextures = false;
                lPolygonGroupData->m_PerGroupData.miUseTextures = 0;
                continue;
            }

            // ensure the image is square, else flag the mesh as un-addable and break out of the loop
            if ((*img_FromFile.GetImages()).width != (*img_FromFile.GetImages()).height)
            {
                pDxMesh.isNotAddable(); // ADD DEBUG MESSAGE
                MessageBox(m_hWnd, L"This mesh is not addable because the images need to be square, i.e. 256x256.", nullptr, MB_OK | MB_ICONWARNING);
                return;
            }

            // if "texture_resolution" is zero after all the materials have been processed then this mesh has no texutres, 
            // as this point will never be reached because the for loop will never enter here
            lTexturesNameAndDimention.back().mDimension = texture_resolution = (*img_FromFile.GetImages()).width;

            // mark the texture as found
            lTexturesNameAndDimention.back().mbfound = true;

            // activate the appropriate local flag
            const TEXTURE_RESOLUTION lIndex = GetTextureIndexFromResolution(texture_resolution);
            if (lIndex == TEXTURE_RESOLUTION::UNSUPPORTED)
            {
                pDxMesh.isNotAddable(); /////////////////////////////////////////////////////////////// // DISPLAY DEBUG MESSAGE
                MessageBox(m_hWnd, L"The resolution is not supproted.", nullptr, MB_OK | MB_ICONWARNING);
                return;
            }
            else
            {
                switch (pTexture->GetTextureType())
                {
                case TEXTURE_TYPE::DIFFUSE_MAP:
                    bDiffuseMap = true;
                    lPolygonGroupData->m_PerGroupData.DiffuseMapIndex = m_texture_resolution_index[lIndex]++;
                    break;
                case TEXTURE_TYPE::NORMAL_MAP_TANGENT:
                    bNormalMap = true;
                    lPolygonGroupData->m_PerGroupData.NormalMapIndex = m_texture_resolution_index[lIndex]++;
                    break;
                case TEXTURE_TYPE::LIGHT_MAP:
                    bLightMap = true;
                    break;
                default:
                {
                    lTexturesNameAndDimention.pop_back();
                    continue;
                }
                }
            }

            // convert image to DXGI_FORMAT
            if (format != L"tga" && format != L"TGA")
                hr = Convert(*(img_FromFile.GetImages()), DXGI_FORMAT_R8G8B8A8_UNORM, TEX_FILTER_DEFAULT, 0.5f, img_dx_format);
            if (FAILED(hr))
            {
                pDxMesh.isNotAddable();
                MessageBox(m_hWnd, L"It failed converting the format.", nullptr, MB_OK | MB_ICONWARNING);
                return;
            }

            // Generate mip-maps
            hr = GenerateMipMaps(*(img_dx_format.GetImages()), TEX_FILTER_DEFAULT, 0, img_mipmapped, false);
            if (FAILED(hr))
            {
                pDxMesh.isNotAddable();
                MessageBox(m_hWnd, L"It failed generating bitmaps.", nullptr, MB_OK | MB_ICONWARNING);
                return;
            }

            // compress the image
            hr = Compress(img_mipmapped.GetImages(), img_mipmapped.GetImageCount(), img_mipmapped.GetMetadata(), DXGI_FORMAT_BC3_UNORM, TEX_COMPRESS_DEFAULT, 0.0f, img_final);
            if (FAILED(hr))
            {
                pDxMesh.isNotAddable();
                MessageBox(m_hWnd, L"It failed compressing the textures.", nullptr, MB_OK | MB_ICONWARNING);
                return;
            }

            // save to dds memory
            unique_ptr<DirectX::Blob> pDDSMemory;
            pDDSMemory.reset(new DirectX::Blob);
            hr = SaveToDDSMemory(*(img_final.GetImages()), DDS_FLAGS_NONE, *pDDSMemory);
            if (FAILED(hr))
            {
                pDxMesh.isNotAddable();
                MessageBox(m_hWnd, L"It failed saving the finalized texture to memory.", nullptr, MB_OK | MB_ICONWARNING);
                return;
            }
            pDDSTextures.push_back(std::move(pDDSMemory)); // We use move() here for performance and because Blob does not allow copy construction
        }

        // Set the index buffer        
        lpIndexBuffer = CreateMeshIndexBuffer(perGroupData->mpIndices, perGroupData->mIndicesFormat, perGroupData->mIndicesCount);
        lPolygonGroupData->m_pDXIndexbuffer = lpIndexBuffer;
        lPolygonGroupData->m_indexformat = perGroupData->mIndicesFormat;
        lPolygonGroupData->m_index_count = perGroupData->mIndicesCount;
        lPolygonGroupData->m_PerGroupData.material = perGroupData->mMaterial;
        lPolygonGroupData->m_wzMaterialName = perGroupData->mMaterialName;

        // Add the per polygon-group data to the list
        PERGROUPDATA_MESH* temp_ptr_groupdata = lPolygonGroupData.release();
        pDxMesh.AddPolygonGroup(temp_ptr_groupdata);

        // The mesh's textures art not all the same so we can't process this mesh
        if (AllTheSame(lTexturesNameAndDimention) == false)
        {
            //  ADD DEBUG MESSAGE                            
            pDxMesh.isNotAddable();
            MessageBox(m_hWnd, L"The Mesh's textures are not all the same size. This mesh cannot be added.", nullptr, MB_OK | MB_ICONWARNING);
            return;
        }
    }

    // If there were no textures than set the appropriate shaders and return
    if (lTexturesNameAndDimention.size() == 0)
    {
        // the mesh has no textures, set the appropriate shaders and return
        pDxMesh.SetVertexShader(m_VS_POS_NORM);
        pDxMesh.SetPixelShader(m_PS_POS_NORM);
        return;
    }

    // Get the index into the SRV array
    const TEXTURE_RESOLUTION SRV_INDEX = GetTextureIndexFromResolution(texture_resolution);
    if (SRV_INDEX != TEXTURE_RESOLUTION::UNSUPPORTED)
    {
        vector<TexDimension> textures;
        textures.reserve(lTexturesNameAndDimention.size());

        // Pop all the none found textures
        for_each(lTexturesNameAndDimention.begin(), lTexturesNameAndDimention.end(), [&](TexDimension& texture)
        {
            if (texture.mbfound)
                textures.push_back(texture);
        });

        AddTexturesToPipeline(pDDSTextures, SRV_INDEX);
    }   // Add textures to the pipeline...


    // Set the appropriate shaders
    if (bLightMap)
        uint debug = 0; // set shaders for the light maps --> This needs to be implemented
    else if (bDiffuseMap)
    {
        if (bNormalMap)
        {
            // set proper shaders for normal mapping
            pDxMesh.SetVertexShader(m_VS_POS_TEX_NORM_BTAN);
            pDxMesh.SetPixelShader(m_PS_POS_TEX_NORM_BITAN[SRV_INDEX]);
        }
        else
        {
            pDxMesh.SetVertexShader(m_VS_POS_TEX_NORM);
            if (SRV_INDEX == TEXTURE_RESOLUTION::UNSUPPORTED)
                pDxMesh.SetPixelShader(m_PS_POS_TEX_NORM[TEXTURE_RESOLUTION::RES_512]);
            else
                pDxMesh.SetPixelShader(m_PS_POS_TEX_NORM[SRV_INDEX]);
        }
    }
}

void RendererDx::AddTexturesToPipeline(const vector<unique_ptr<DirectX::Blob>>& _ddstextures, uint SRV_INDEX)
{
    size_t arraySize = _ddstextures.size();
    size_t itexturesTocopy = 0;

    // Get the data from the dds files
    vector<ID3D11Texture2D*> vResources2D;
    vResources2D.resize(arraySize);
    for (size_t i = 0; i < arraySize; ++i)
    {
        // Note: This function is in the DirectX Tool Kit Library
        CreateDDSTextureFromMemoryEx(m_pDxDevice,
            (uint8_t*) _ddstextures[i].get()->GetBufferPointer(),
            _ddstextures[i].get()->GetBufferSize(),
            NULL,
            D3D11_USAGE_IMMUTABLE,
            D3D11_BIND_SHADER_RESOURCE,
            NULL,
            NULL,
            false,
            reinterpret_cast<ID3D11Resource**>(&vResources2D[i]),
            nullptr);
    }

    if (m_pTextureResource[SRV_INDEX])
    {
        D3D11_SHADER_RESOURCE_VIEW_DESC srv;
        m_pTextureResource[SRV_INDEX]->GetDesc(&srv);

        // Add to the array count
        itexturesTocopy = srv.Texture2DArray.ArraySize;
        arraySize += itexturesTocopy;
    }

    // Get the description from at least 1 of the textures since ALL have to be the same dimension and format
    D3D11_TEXTURE2D_DESC vtexDesc;
    vResources2D[0]->GetDesc(&vtexDesc);

    D3D11_TEXTURE2D_DESC texDesc;
    texDesc.Width = vtexDesc.Width;
    texDesc.Height = vtexDesc.Height;
    texDesc.MipLevels = vtexDesc.MipLevels;
    texDesc.ArraySize = arraySize;
    texDesc.Format = vtexDesc.Format;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    texDesc.CPUAccessFlags = NULL;
    texDesc.MiscFlags = NULL;

    // Reserve the VRAM memory for the textures
    ID3D11Texture2D* textureArray = nullptr;
    m_pDxDevice->CreateTexture2D(&texDesc, NULL, &textureArray);

    // Copy the textures in the old SRV, if any
    size_t itextureIndex = 0;
    if (m_pTextureResource[SRV_INDEX])
    {
        // Pointer to the old array of textures
        ID3D11Resource* sourceSRV = nullptr;
        m_pTextureResource[SRV_INDEX]->GetResource(&sourceSRV);

        // Per texture
        for (itextureIndex; itextureIndex < itexturesTocopy; ++itextureIndex)
        {
            // Per MipMap level
            for (uint miplevel = 0; miplevel < texDesc.MipLevels; ++miplevel)
            {
                m_pDxContext->CopySubresourceRegion
                    (
                    textureArray,                                                       // Destination resource
                    D3D11CalcSubresource(miplevel, itextureIndex, texDesc.MipLevels),   // Destination miplevel index 
                    NULL,                                                               // DestX                                                                           
                    NULL,                                                               // DestY
                    NULL,                                                               // DestZ
                    sourceSRV,                                                          // Source resource from which to copy
                    miplevel,                                                           // Source MipLevel Index
                    nullptr
                    );
            }
        }
        // Calling the GetResource() increments the internal count so we need to decrement it.
        SAFE_RELEASE_COM(sourceSRV);

        // release the old memory
        SAFE_RELEASE_COM(m_pTextureResource[SRV_INDEX]);
    }

    // Copy the new textures into the SRV member array

    // Per texture
    for (itextureIndex; itextureIndex < arraySize; ++itextureIndex)
    {
        // Per MipMap level
        for (uint miplevel = 0; miplevel < texDesc.MipLevels; miplevel++)
        {
            m_pDxContext->CopySubresourceRegion
                (
                textureArray,
                D3D11CalcSubresource(miplevel, itextureIndex, texDesc.MipLevels),
                NULL,
                NULL,
                NULL,
                vResources2D[itextureIndex - itexturesTocopy],
                miplevel,
                nullptr
                );
        }
    }

    // Release the temporary Video Memory used per texture
    for (uint i = 0; i < _ddstextures.size(); ++i)
    {
        SAFE_RELEASE_COM(vResources2D[i]);
    }

    // Create the view to the Texture array by filling out the Texture2DArray union from D3D11_SHADER_RESOURCE_VIEW_DESC
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = texDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
    srvDesc.Texture2DArray.MostDetailedMip = 0;
    srvDesc.Texture2DArray.MipLevels = texDesc.MipLevels;
    srvDesc.Texture2DArray.FirstArraySlice = 0;
    srvDesc.Texture2DArray.ArraySize = arraySize;

    m_pDxDevice->CreateShaderResourceView(textureArray, &srvDesc, &m_pTextureResource[SRV_INDEX]);
    m_pDxContext->PSSetShaderResources(SRV_INDEX, 1, &m_pTextureResource[SRV_INDEX]);

    SAFE_RELEASE_COM(textureArray);
}

void RendererDx::MoveLight(int vertically, int horizontally)
{
    const float RADIANS_PER_PIXEL = MLConvertToRadians(360.0f / (m_iHeight));
    m_lightPitch += RADIANS_PER_PIXEL*(-vertically);
    m_lightHead += RADIANS_PER_PIXEL*(-horizontally);

    // Clamp the head angle
    if (m_lightPitch > CLOSE_TO_90)
        m_lightPitch = CLOSE_TO_90;
    else if (m_lightPitch < -CLOSE_TO_90)
        m_lightPitch = -CLOSE_TO_90;

    // clamp the pitch angle
    if (m_lightHead > TWO_PI)
        m_lightHead -= TWO_PI;
    else if (m_lightHead < -TWO_PI)
        m_lightHead -= TWO_PI;

    m_DirLight.Direction.x = m_lightRadius * cos(m_lightPitch) * sin(m_lightHead);  // X-coordinate
    m_DirLight.Direction.y = (-m_lightRadius) * sin(m_lightPitch);                  // Y-coordinate
    m_DirLight.Direction.z = m_lightRadius * cos(m_lightPitch) * cos(m_lightHead);  // Z-coordinate
    m_DirLight.Direction.w = 1.0f;

    m_pDxContext->UpdateSubresource(m_pDxConstBuffer_Lights, NULL, NULL, &m_DirLight, NULL, NULL);
}

bool RendererDx::InitializeDeviceAndContext(void)
{
    uint createDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    D3D_FEATURE_LEVEL featureLevels [] =
    {
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_1
    };

    // Create device and Context|
    D3D_FEATURE_LEVEL           FeatureLevel;
    ComPtr<ID3D11Device>        d3d_device;
    ComPtr<ID3D11DeviceContext> context;

    HRESULT hr = D3D11CreateDevice(0, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags,
        featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION, &d3d_device, &FeatureLevel, &context);
    if (SUCCEEDED(hr))
    {
        hr = d3d_device.Get()->QueryInterface(IID_PPV_ARGS(&m_pDxDevice));
        hr = context.Get()->QueryInterface(IID_PPV_ARGS(&m_pDxContext));

        // Determine the Feature Level is supported
        if (FeatureLevel < D3D_FEATURE_LEVEL_10_0)
        {
            MessageBox(NULL, L"Feature Level 10 or higher not supported", NULL, MB_OK);
            return false;
        }
        // Set the context for the DxMesh to use to draw itself
        DXMesh::SetContext(m_pDxContext);
        return true;
    }
    else
    {
        MessageBox(NULL, L"The  DirectX device could not be created.", NULL, MB_OK);
        return false;
    }
}

bool RendererDx::InitializeSwapChain(const HWND& hwnd, int height, int width)
{
    m_hWnd = hwnd;
    m_iHeight = height;
    m_iWidth = width;

    // Fill out a DXGI_SWAP_CHAIN_DESC
    DXGI_SWAP_CHAIN_DESC1 SwapChainDesc;
    SecureZeroMemory(&SwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC1));

    m_pDxDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_B8G8R8A8_UNORM, m_iSamplecount, &m_iQualityLevel);
    m_iQualityLevel -= 1;

    SwapChainDesc.Width = 0;    // for automatic width
    SwapChainDesc.Height = 0;    // for automatic height
    SwapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    SwapChainDesc.Stereo = false;
    SwapChainDesc.SampleDesc.Count = m_iSamplecount;
    SwapChainDesc.SampleDesc.Quality = m_iQualityLevel;
    SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    SwapChainDesc.BufferCount = 2;
    SwapChainDesc.Scaling = DXGI_SCALING_STRETCH;
    SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    SwapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
    SwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    // Actually create it now
    IDXGIDevice2* pDXGIDevice = GetDXGIDevice();
    if (pDXGIDevice == nullptr)
        return false;

    HRESULT hr = pDXGIDevice->SetMaximumFrameLatency(1); // Required for Windows Store Apps

    IDXGIAdapter * pDXGIAdapter;
    hr = pDXGIDevice->GetParent(IID_PPV_ARGS(&pDXGIAdapter));

    IDXGIFactory2 * pIDXGIFactory;
    hr = pDXGIAdapter->GetParent(IID_PPV_ARGS(&pIDXGIFactory));

    hr = pIDXGIFactory->CreateSwapChainForHwnd(m_pDxDevice, m_hWnd, &SwapChainDesc, nullptr, nullptr, &m_pDxSwapchain);
    //hr = pIDXGIFactory->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_WINDOW_CHANGES);

    m_pDxSwapchain->SetFullscreenState(false, nullptr);

    // Release temporary objects
    SAFE_RELEASE_COM(pIDXGIFactory);
    SAFE_RELEASE_COM(pDXGIAdapter);
    SAFE_RELEASE_COM(pDXGIDevice);

    // VOnResize() is called to prevent code duplication
    VOnResize(m_iWidth, m_iHeight);

    return true;
}

IDXGIDevice2* RendererDx::GetDXGIDevice(void)
{
    IDXGIDevice2* pDXGIDevice;

    HRESULT hr = m_pDxDevice->QueryInterface(IID_PPV_ARGS(&pDXGIDevice));

    if (FAILED(hr))
    {
        return nullptr;
    }
    return pDXGIDevice;
}

IDXGISwapChain1* RendererDx::GetSwapChain(void)
{
    return m_pDxSwapchain;
}

void RendererDx::PresentScene(void)
{
    HRESULT hr = S_OK;
    DXGI_PRESENT_PARAMETERS pParameters;
    SecureZeroMemory(&pParameters, sizeof(DXGI_PRESENT_PARAMETERS));

    hr = m_pDxSwapchain->Present1(0, 0, &pParameters);// By examiniing the return parameter we might be able to prevent a system crash due to DX loosing the device
}

void RendererDx::CreateWorldAxes(float x, float y, float z)
{
    m_pDxVertexBuffer_WorldAxes.Reset();
    m_pDxIndexBuffer_WorldAxes.Reset();

    vector<float3> points;
    points.reserve(4);
    points.push_back(float3(0.0f, 0.0f, 0.0f));     // Origin
    points.push_back(float3(x, 0.0f, 0.0f));        // X- axis
    points.push_back(float3(0.0f, y, 0.0f));        // Y- axis
    points.push_back(float3(0.0f, 0.0f, z));        // Y- axis

    // VERTEX BUFFER |

    // Fill in a buffer description.
    D3D11_BUFFER_DESC bufferDesc;
    bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    bufferDesc.ByteWidth = sizeof(float3) * 4;
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = NULL;

    // Fill in the subresource data.
    D3D11_SUBRESOURCE_DATA InitData;
    InitData.pSysMem = &points[0];
    InitData.SysMemPitch = 0;
    InitData.SysMemSlicePitch = 0;

    // Create the vertex buffer
    m_pDxDevice->CreateBuffer(&bufferDesc, &InitData, m_pDxVertexBuffer_WorldAxes.GetAddressOf());

    // INDEX BUFFER |

    vector<short> indices;
    indices.reserve(6);
    indices.push_back(0); indices.push_back(1); // x
    indices.push_back(0); indices.push_back(2); // y
    indices.push_back(0); indices.push_back(3); // z

    const uint index_stride = sizeof(unsigned short);

    // Fill in a buffer description for the index buffer
    bufferDesc.ByteWidth = index_stride * indices.size();
    bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

    // Fill in the subresource data.
    InitData.pSysMem = &indices[0];

    // Create the vertex buffer
    m_pDxDevice->CreateBuffer(&bufferDesc, &InitData, m_pDxIndexBuffer_WorldAxes.GetAddressOf());

    // ARROWS
    {
        // --------------|
        // Vertex Buffer |
        // --------------|
        const float value = y * .015f; // 1%
        const float times = 3.5f;

        const float YoffSet = y;
        const float XoffSet = x;
        const float ZoffSet = z;

        vector<float3> arrowVertices;
        arrowVertices.reserve(15);


        // X arrow
        arrowVertices.push_back(float3(XoffSet, value, -value));
        arrowVertices.push_back(float3(XoffSet, value, value));
        arrowVertices.push_back(float3(XoffSet, -value, value));
        arrowVertices.push_back(float3(XoffSet, -value, -value));

        arrowVertices.push_back(float3((times*value) + XoffSet, 0.0f, 0.0f));

        // Y arrow
        arrowVertices.push_back(float3(-value, YoffSet, value));
        arrowVertices.push_back(float3(value, YoffSet, value));
        arrowVertices.push_back(float3(value, YoffSet, -value));
        arrowVertices.push_back(float3(-value, YoffSet, -value));

        arrowVertices.push_back(float3(0.0f, (times*value) + YoffSet, 0.0f));

        // Z arrow
        arrowVertices.push_back(float3(value, value, ZoffSet));
        arrowVertices.push_back(float3(-value, value, ZoffSet));
        arrowVertices.push_back(float3(-value, -value, ZoffSet));
        arrowVertices.push_back(float3(value, -value, ZoffSet));

        arrowVertices.push_back(float3(0.0f, 0.0f, (times*value) + ZoffSet));

        bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bufferDesc.ByteWidth = sizeof(float3) * arrowVertices.size();
        InitData.pSysMem = &arrowVertices[0];
        m_pDxDevice->CreateBuffer(&bufferDesc, &InitData, m_pDxVertexBuffer_WorldAxesArrows.GetAddressOf());

        // -------------|
        // INDEX BUFFER |
        // -------------|
        vector<unsigned short> arrowsIndices;
        arrowsIndices.reserve(18);

        // Bottom triangles
        arrowsIndices.push_back(0); arrowsIndices.push_back(1); arrowsIndices.push_back(2);
        arrowsIndices.push_back(0); arrowsIndices.push_back(2); arrowsIndices.push_back(3);

        // Side triangles to the tip
        arrowsIndices.push_back(0); arrowsIndices.push_back(1); arrowsIndices.push_back(4);
        arrowsIndices.push_back(1); arrowsIndices.push_back(2); arrowsIndices.push_back(4);
        arrowsIndices.push_back(2); arrowsIndices.push_back(3); arrowsIndices.push_back(4);
        arrowsIndices.push_back(3); arrowsIndices.push_back(0); arrowsIndices.push_back(4);

        bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        bufferDesc.ByteWidth = sizeof(unsigned short) * arrowsIndices.size();
        InitData.pSysMem = &arrowsIndices[0];
        m_pDxDevice->CreateBuffer(&bufferDesc, &InitData, m_pDxIndexBuffer_WorldAxesArrows.GetAddressOf());
    }
}

void RendererDx::CreateGrid(float x, float z)
{
    const float Xc = x / 10;
    const float Zc = z / 10;
    const float yOffset = -0.4f; // So the grid would be JUST below the (0,0,0) origin and below the xy-plane

    vector<float3> points;
    points.reserve(76);

    for (uint i = 1; i < 20; ++i)
    {
        const float xpoint = x - (i* Xc);
        const float zpoint = z - (i* Zc);

        // Z points
        points.push_back(float3(x - Xc, yOffset, zpoint));
        points.push_back(float3(-x + Xc, yOffset, zpoint));

        // X points
        points.push_back(float3(xpoint, yOffset, z - Zc));
        points.push_back(float3(xpoint, yOffset, -z + Zc));
    }

    // Fill in a buffer description.
    D3D11_BUFFER_DESC bufferDesc;
    bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    bufferDesc.ByteWidth = sizeof(float3) * points.size();
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = NULL;

    // Fill in the subresource data.
    D3D11_SUBRESOURCE_DATA InitData;
    InitData.pSysMem = &points[0];
    InitData.SysMemPitch = 0;
    InitData.SysMemSlicePitch = 0;

    // Create the vertex buffer
    m_pDxDevice->CreateBuffer(&bufferDesc, &InitData, m_pDxVertexBuffer_Grid.GetAddressOf());
}

void RendererDx::DrawWorldAxis(void)
{
    const uint strides = sizeof(float3);
    const uint offset = 0;

    m_pDxContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

    // Set the Shader programs and input layout for the mesh
    m_pDxContext->VSSetShader(m_VS_FacesNormal, NULL, NULL);
    m_pDxContext->PSSetShader(m_PS_FacesNormal, NULL, NULL);
    m_pDxContext->IASetInputLayout(m_InputLayout_POS);

    // change color constant buffer
    m_pDxContext->IASetVertexBuffers(1, 1, m_pDxVertexBuffer_WorldAxes.GetAddressOf(), &strides, &offset);
    m_pDxContext->IASetIndexBuffer(m_pDxIndexBuffer_WorldAxes.Get(), DXGI_FORMAT_R16_UINT, 0);

    float4 color(1.0f, 0.0f, 0.0f, 1.0f);

    // X axis        
    m_pDxContext->UpdateSubresource(m_pDxConstBuffer_color.Get(), NULL, NULL, &color, NULL, NULL);
    m_pDxContext->DrawIndexed(2, 0, 0);

    // Y axis
    color = float4(0.0f, 1.0f, 0.0f, 1.0f);
    m_pDxContext->UpdateSubresource(m_pDxConstBuffer_color.Get(), NULL, NULL, &color, NULL, NULL);
    m_pDxContext->DrawIndexed(2, 2, 0);

    // Z axis
    color = float4(0.0f, 0.0f, 1.0f, 1.0f);
    m_pDxContext->UpdateSubresource(m_pDxConstBuffer_color.Get(), NULL, NULL, &color, NULL, NULL);
    m_pDxContext->DrawIndexed(2, 4, 0);

    {
        // Set the Rasterstate to NO_CULLING so the arrows can be drawn properly
        m_pDxContext->RSSetState(m_pDxRasterState_NoCulling.Get());
        m_pDxContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        // Set the vertex and index buffer
        m_pDxContext->IASetVertexBuffers(1, 1, m_pDxVertexBuffer_WorldAxesArrows.GetAddressOf(), &strides, &offset);
        m_pDxContext->IASetIndexBuffer(m_pDxIndexBuffer_WorldAxesArrows.Get(), DXGI_FORMAT_R16_UINT, 0);

        // X Arrow
        color = float4(1.0f, 0.0f, 0.0f, 1.0f);
        m_pDxContext->UpdateSubresource(m_pDxConstBuffer_color.Get(), NULL, NULL, &color, NULL, NULL);
        m_pDxContext->DrawIndexed(18, 0, 0);

        // Y Arrow
        color = float4(0.0f, 1.0f, 0.0f, 1.0f);
        m_pDxContext->UpdateSubresource(m_pDxConstBuffer_color.Get(), NULL, NULL, &color, NULL, NULL);
        m_pDxContext->DrawIndexed(18, 0, 5);

        // Z Arrow
        color = float4(0.0f, 0.0f, 1.0f, 1.0f);
        m_pDxContext->UpdateSubresource(m_pDxConstBuffer_color.Get(), NULL, NULL, &color, NULL, NULL);
        m_pDxContext->DrawIndexed(18, 0, 10);
    }
}

void RendererDx::WireFrameMode(void)
{
    // Set the Rasterizer state back to its default settings
    m_pDxContext->RSSetState(nullptr);

    // Get the Current Rasterizer state structure and release the pointer to it
    D3D11_RASTERIZER_DESC1 rs;
    m_pDxRasterState_AntiAliasing->GetDesc1(&rs);
    SAFE_RELEASE_COM(m_pDxRasterState_AntiAliasing);

    // change the fill mode as appropriate
    rs.FillMode = (rs.FillMode == D3D11_FILL_SOLID) ? D3D11_FILL_WIREFRAME : D3D11_FILL_SOLID;

    // recreate the Raster state with the new fill-mode
    HRESULT hr = m_pDxDevice->CreateRasterizerState1(&rs, &m_pDxRasterState_AntiAliasing);

    assert(SUCCEEDED(hr));

    // No need to set the state to the pipeline as the DrawScene function will do this evert frame
}

void RendererDx::BackFaceCulling(void)
{
    // Set the Rasterizer state back to its default settings
    m_pDxContext->RSSetState(nullptr);

    // Get the Current Rasterizer state structure and release the pointer to it
    D3D11_RASTERIZER_DESC1 rs;
    m_pDxRasterState_AntiAliasing->GetDesc1(&rs);
    SAFE_RELEASE_COM(m_pDxRasterState_AntiAliasing);

    // change the fill mode as appropriate
    rs.CullMode = (rs.CullMode == D3D11_CULL_BACK) ? D3D11_CULL_NONE : D3D11_CULL_BACK;

    // recreate the Raster state with the new fill-mode
    HRESULT hr = m_pDxDevice->CreateRasterizerState1(&rs, &m_pDxRasterState_AntiAliasing);

    assert(SUCCEEDED(hr));

    // No need to set the state to the pipeline as the DrawScene function will do this evert frame
}

void RendererDx::TexturesOnOff(void)
{
    // invert the value
    m_buseTextures = m_buseTextures ? false : true;

    if (!m_MeshList.empty())
    {
        auto mesh = *(m_MeshList.begin());
        mesh->UseTextures(m_buseTextures);
    }
}

void RendererDx::DeleteMesh(void)
{
    // Delete all the Meshes
    uint debug = 0;
    for_each(m_MeshList.begin(), m_MeshList.end(), [&](DXMesh*& ptr)
    {
        SAFE_DELETE(ptr);
    });
    m_MeshList.clear();

    // Reset all the textures indices and delete all textures
    for (size_t i = 0; i < MAX_SHADER_NUMBERS; ++i)
    {
        m_texture_resolution_index[i] = 0;
        SAFE_RELEASE_COM(m_pTextureResource[i]);
    }
    if (m_bFocusOnMesh)
    {
        FocusOnMesh();
        m_bFocusOnMesh = true;
    }
    m_pCamera->SetMinimumRadius(1.0f);
}

uint RendererDx::GetTriangleCount(void)
{
    uint lPolygonCount = 0;
    for_each(m_MeshList.begin(), m_MeshList.end(), [&](DXMesh* _pMesh)
    {
        lPolygonCount += _pMesh->GetPolygonCount();
    });
    return lPolygonCount;
}

void RendererDx::LightFollowCamera(void)
{
    float2 headandpitch = m_pCamera->GetHeadandPitch();
    m_lightHead = headandpitch.u;
    m_lightPitch = headandpitch.v;
    MoveLight(0, 0);

    m_DirLight.Direction.x *= -1;   // X-coordinate
    m_DirLight.Direction.y *= -1;   // Y-coordinate
    m_DirLight.Direction.z *= -1;   // Z-coordinate
    m_DirLight.Direction.w = 1.0f;

    m_pDxContext->UpdateSubresource(m_pDxConstBuffer_Lights, NULL, NULL, &m_DirLight, NULL, NULL);
}

void RendererDx::StopFollowingCamera(void)
{
    // because the camera != light direction we need to rotate the coordinates of the camera
    float2 headandpitch = m_pCamera->GetHeadandPitch();
    m_lightHead = headandpitch.u + MLConvertToRadians(180);
    m_lightPitch = -headandpitch.v;
    MoveLight(0, 0);
}

Material& RendererDx::GetMaterial(const wstring& _material_name)
{
    assert(m_MeshList.empty() == false);
    return m_MeshList.front()->GetMaterial(_material_name);
}

void RendererDx::FocusOnMesh(void)
{
    m_bFocusOnMesh = m_bFocusOnMesh ? false : true; // flip the value of this flag

    if (m_bFocusOnMesh)
    {
        if (m_MeshList.empty() == false)
        {
            // calculate the distance from the highest to the lowest point of the AABB
            const float AABB_LENGTH = MLVectorLenght((m_aabb.m_highest - m_aabb.m_lowest));

            // Set the minimum radius as the distance between the highest and lowest points of the Mesh's AABB
            m_pCamera->SetMinimumRadius((AABB_LENGTH / 2.0f));

            // Set the look-at point as the center of the Mesh's bounding box if necessary
            SetLookAtPoint(m_aabb.GetCenterPoint());
            m_pCamera->SetRadius((AABB_LENGTH));
            ZoomOut();
        }
    }
    else if (m_bFocusOnMesh == false)
    {
        SetLookAtPoint(float4());
        ZoomIn();
        ZoomOut();
    }
}

void RendererDx::MoveCamera(const int vertically, const int horizontally)
{
    m_pCamera->MoveCamera(vertically, horizontally);
    UpdatePerFrameConstBuffer();

    if (m_MeshList.empty() == false)
    {
        CalCulateVisibleFacesNormals();
    }
}

void RendererDx::CalCulateVisibleFacesNormals(void)
{
    assert(m_MeshList.empty() == false);
    DXMesh* pdxMesh = m_MeshList.front();

    // Get the pointer to the vertex buffer
    ID3D11Buffer* pDynamicVertexBuffer = pdxMesh->GetFacesNormalBuffer();

    // Get the faces' normal
    const vector<FaceNormal>& facesNormals = pdxMesh->GetFacesNormals();

    // Get "eye" position
    const float3 position = float3(m_pCamera->VGetEyePosition());

    uint iFacesNormalsToDraw = 0;
    D3D11_MAPPED_SUBRESOURCE lFacesNormals;
    SecureZeroMemory(&lFacesNormals, sizeof(D3D11_MAPPED_SUBRESOURCE));
    HRESULT hr = m_pDxContext->Map(pDynamicVertexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &lFacesNormals);
    float3* pdata = reinterpret_cast<float3*>(lFacesNormals.pData);
    for (size_t i = 0; i < facesNormals.size(); ++i)
    {
        if (MLVectorDot(facesNormals[i].mFaceNormal, position) > 0.0f)
        {
            pdata[iFacesNormalsToDraw++] = facesNormals[i].mCentroid;
            pdata[iFacesNormalsToDraw++] = facesNormals[i].mCentroidTip;
        }
    }
    m_pDxContext->Unmap(pDynamicVertexBuffer, NULL);
    pdxMesh->SetFacesNormalToDrawCount(iFacesNormalsToDraw);
}

void RendererDx::AddUIElement(IUIElement* _ptrUIElement)
{
    const BUTTON_TYPE lType = _ptrUIElement->GetButtonType();
    if (lType == BUTTON_TYPE::ROUND_BUTTON)
    {
        // get a pointer to the correct data type
        RoundButton* lptrRoundButton = static_cast<RoundButton*>(_ptrUIElement);
        // Create the vertices; POS_W, UV-coordinates.
        vector<float> lVertexPositions;
        vector<unsigned short> lIndices;
        //createcircle(lptrRoundButton->GetCenterPoint(), lptrRoundButton->GetRadius(), ) /*NEED OUTLINE WIDTH HERE*/
        
        //

    }
    else if (lType == BUTTON_TYPE::SQUARE_BUTTON)
    {

    }
    else if (lType == BUTTON_TYPE::SCROLL_BAR)
    {

    }
}