#include "HNDLoader.h"


Mesh* HNDLoader::VProcessfile(const wstring& inputFile, IView* pIView)
{
    m_pIView = pIView;
    ExtractFileSNameDirectoryAndFormat(inputFile);
    m_format = FILE_FORMAT::HND;
    
    uint length = 0;
    const char* buffer = OpenFile(m_InputFileFullPath.c_str(), length, L"rb");
    if(!buffer)
        return nullptr;

    // Only if the file was successfully open
    Mesh* lMesh = new Mesh();
    lMesh->SetRawData( const_cast<char*>(buffer) );

    uint index = 0;
    HND_HEADER* header = reinterpret_cast<HND_HEADER*>(const_cast<char*>(&buffer[index]));
    index+= sizeof(HND_HEADER);

    HND_TOKENS *ltoken = reinterpret_cast<HND_TOKENS*>(const_cast<char*>(&buffer[index]));
    index += sizeof(HND_TOKENS);

    wstring pathName;
    pathName.reserve(300);
    while(*ltoken != HND_TOKENS::END_OF_FILE)
    {
        if(*ltoken == HND_TOKENS::VERTICES)
        {
            float* V = reinterpret_cast<float*>(const_cast<char*>(&buffer[index]));
            lMesh->SetPerVertexData(V, header->mVertsCount, header->mformat);

            index += (lMesh->GetVertexStride() * header->mVertsCount);
        }
        else if (*ltoken == HND_TOKENS::INDICES)
        {
            void * indices = reinterpret_cast<void*>(const_cast<char*>(&buffer[index]));
            lMesh->SetIndices(indices, header->mIndexCount, header->mIndexFormat);

            uint index_stride = header->mIndexFormat == INDEX_FORMAT::SHORT_TYPE ? sizeof(short) : sizeof(uint);
            index += (index_stride * header->mIndexCount);
        }
        else if (*ltoken == HND_TOKENS::MATERIAL)
        {
            Material * MAT = reinterpret_cast<Material*>(const_cast<char*>(&buffer[index]));
            lMesh->SetMaterial(*MAT);
            index += sizeof(Material);
        }
        else if (*ltoken == HND_TOKENS::TEXTURE)
        {
            Texture* tex = new Texture();
            
            // first get the type
            tex->SetType( *( reinterpret_cast<TEXTURE_TYPE*>( const_cast<char*>(&buffer[index]) ) ) );
            index += sizeof(uint);

            const wchar_t* lbuffer = reinterpret_cast<const wchar_t*>( const_cast<char*>(&buffer[index]) );
            uint temp_index = 0;
            while(lbuffer[temp_index] != L'<')
            {
                pathName += lbuffer[temp_index];
                index += sizeof(wchar_t);
                ++temp_index;
            }
            index += sizeof(wchar_t);
            tex->SetFilePath(pathName);
            pathName.clear();
            lMesh->AddTexture(tex);
        }
        ltoken = reinterpret_cast<HND_TOKENS*>(const_cast<char*>(&buffer[index]));
        index += sizeof(HND_TOKENS);
    }
    return lMesh;
}