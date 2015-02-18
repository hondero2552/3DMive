#include "ExporterHnd.h"

void HNDExpoter::VExportMeshToFile(const Mesh& M, const wstring& exportFile) 
{
    m_pMesh = const_cast<Mesh *>(&M);

#if defined(_WINDOWS) || defined(WINDOWS)
    _wfopen_s(&m_pOutputFile, exportFile.c_str(), L"wb");
#else
    FILE* file = fopen(fname.c_str(), "wb");
#endif

    // If the file could not be found/open
    if(!m_pOutputFile)
        return;
    HND_TOKENS ltoken;
    VWriteHeader();

    ltoken = HND_TOKENS::VERTICES;
    fwrite(&ltoken, sizeof(uint), 1, m_pOutputFile);
    VWriteVertexData();

    ltoken = HND_TOKENS::INDICES;
    fwrite(&ltoken, sizeof(uint), 1, m_pOutputFile);
    VWriteIndexData();

    ltoken = HND_TOKENS::MATERIAL;
    fwrite(&ltoken, sizeof(uint), 1, m_pOutputFile);
    VWriteMaterial();

    // texture token is written inside the function
    VWriteTextureData();

    ltoken = HND_TOKENS::END_OF_FILE;
    fwrite(&ltoken, sizeof(HND_TOKENS), 1, m_pOutputFile);

    fclose(m_pOutputFile);
}

void HNDExpoter::VWriteVertexData(void) const 
{
    const void* verts_data = m_pMesh->GetVertexData();
    fwrite(verts_data, m_pMesh->GetVertexStride(), m_pMesh->GetVertexCount(), m_pOutputFile);    
}

void HNDExpoter::VWriteIndexData(void) const 
{ 
    /*uint index_stride = m_pMesh->GetIndexFormat() == INDEX_FORMAT::SHORT_TYPE ? sizeof(short) : sizeof(uint);
    fwrite(m_pMesh->GetIndexData(), index_stride, m_pMesh->GetIndexCount(), m_pOutputFile);*/
}

void HNDExpoter::VWriteHeader(void) const 
{
    //HND_HEADER header;

    //header.mMagicNumber = FILE_FORMAT::HND;
    //header.mformat      = m_pMesh->GetVertexFormat();
    //header.mVertsCount  = m_pMesh->GetVertexCount();
    //header.mIndexFormat = m_pMesh->GetIndexFormat();
    //header.mIndexCount  = m_pMesh->GetIndexCount();
    //header.mHandedNess  = HANDEDNESS::DIRECTX;
    //header.mMeshCount   = 1;
    //header.ENDIANNESS   = 0;

    //// write the data to the file buffer
    //fwrite(&header, 1, 32, m_pOutputFile);
}

void HNDExpoter::VWriteMaterial(void) const 
{
    fwrite(
        &(m_pMesh->GetMaterial()), 
        sizeof(Material), 
        1, 
        m_pOutputFile
        );
}

void HNDExpoter::VWriteTextureData(void) const 
{
    //for(auto it = m_pMesh->GetTextures().begin(); it != m_pMesh->GetTextures().end(); ++it)
    //{
    //    const Texture* tex = *it;

    //    // first write the HND texture token
    //    HND_TOKENS ltoken = HND_TOKENS::TEXTURE;
    //    fwrite(&ltoken, sizeof(uint), 1, m_pOutputFile);

    //    // second, write the texture-type token
    //    TEXTURE_TYPE type = tex->GetTextureType();
    //    fwrite(&type, sizeof(uint), 1, m_pOutputFile);

    //    // third, write the path and null terminator
    //    const wchar_t* fullpath = tex->GetTextureFullPath().c_str();
    //    for(uint i = 0; i < tex->GetTextureFullPath().size(); ++i)
    //    {
    //        fwrite(&fullpath[i], sizeof(wchar_t), 1, m_pOutputFile);
    //    }
    //    wchar_t sz = L'<';
    //    fwrite(&sz, sizeof(wchar_t), 1, m_pOutputFile);
    //}
}