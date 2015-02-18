#include "DXMesh.h"

ID3D11DeviceContext1* DXMesh::m_pDeviceContext  = nullptr;
ID3D11Buffer* DXMesh::m_PerObjectConstBuffer    = nullptr;

void DXMesh::DrawMe(void) const
{
    assert(m_pDeviceContext != nullptr);
    assert(m_PerObjectConstBuffer != nullptr);
    const uint strides  = m_vertex_stride;
    const uint offset   = 0;

    m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // Fix to implement Per-Mesh primitive topology

    // Set the Shader programs and input layout for the mesh
    m_pDeviceContext->IASetInputLayout( m_inputLayout );    
    m_pDeviceContext->VSSetShader(mVertexShader, NULL, NULL);
    m_pDeviceContext->PSSetShader(mPixelShader, NULL, NULL);
    // Set vertex buffers
    m_pDeviceContext->IASetVertexBuffers(0, 1, &m_vertexbuffer, &strides, &offset);

    // Draw Mesh divided by Material group
    for(auto iter = m_PerGroupDataList.begin(); iter != m_PerGroupDataList.end(); ++iter)
    {
        const PERGROUPDATA_MESH* data = (*iter);
        const DXGI_FORMAT dx_indexformat = data->m_indexformat == SHORT_TYPE ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
        m_pDeviceContext->IASetIndexBuffer(data->m_pDXIndexbuffer, dx_indexformat, NULL);

        // update per object constant buffer
        m_pDeviceContext->UpdateSubresource(m_PerObjectConstBuffer, NULL, NULL, &(data->m_PerGroupData), NULL, NULL);

        // Draw the Mesh to the Back buffer
        m_pDeviceContext->DrawIndexed(data->m_index_count, NULL, NULL);
    }
}

void DXMesh::UseTextures(bool yesOrNo)
{
    std::for_each(m_PerGroupDataList.begin(), m_PerGroupDataList.end(), [&] (PERGROUPDATA_MESH* _groupdata)
    {
        if(yesOrNo)
        {
            if(_groupdata->m_bCanUseTextures)
                _groupdata->m_PerGroupData.miUseTextures = 1;
        }
        else
        {
            _groupdata->m_PerGroupData.miUseTextures = 0;
        }
    });
}

void DXMesh::DrawFacesNormals(void) const
{
    const uint strides  = sizeof( float ) * 3;
    const uint offset   = 0;
    m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
    
    // Set the vertex buffers
    m_pDeviceContext->IASetVertexBuffers(1, 1, &m_facesnormalBuffer, &strides, &offset);

    m_pDeviceContext->Draw(m_facesNormalsToDraw, NULL);    
}

Material& DXMesh::GetMaterial(const wstring& _material_name)
{
    auto iter_begin = m_PerGroupDataList.begin();
    auto iter_end   = m_PerGroupDataList.end();

    while(iter_begin != iter_end)
    {
        if((*iter_begin)->m_wzMaterialName == _material_name)
            return (*iter_begin)->m_PerGroupData.material;
        ++iter_begin;
    }
    assert(true);                                       // Exectuion should never get here
    return std::ref((*iter_begin)->m_PerGroupData.material);
}