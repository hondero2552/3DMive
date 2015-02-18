
#include "MeshIOBase.h"
#include "math_funcs.h"
const char ENDOFLINE[] = "\r\n";

MeshIOBase::MeshIOBase(void) : m_AllDataIsPerVertex(true), m_next_available_index(0), m_windingCCW(false),
    m_bnormalMap(false),
    m_bLightMap(false),
    m_bDiffuseMap(false),
    m_bShadowMap(false),
    m_bHeightMap(false),
    m_bSpecularMap(false),
    m_bNormalsCalculated(false),
    m_bMeshWasFlipped(false),
    m_bFlippedUVsOnly(false),
    m_bUseOriginalNormals(true),
    m_pIView(nullptr),
    m_currentPerFaceMaterial(nullptr),
    m_material_number(0)
{

}

MeshIOBase::~MeshIOBase(void)
{
    // in case the loader is being deleted before returning a Mesh* that's why SAFE_DELETE is used
    for_each(m_pTextures.begin(), m_pTextures.end(), [&](Texture*& ptexture)
    {
        SAFE_DELETE( ptexture );
    });
    EmptyPointersList(m_PerFaceMaterialList);
}

void MeshIOBase::AddTexture(const wstring& fullpath, TEXTURE_TYPE type)
{
    if(type == TEXTURE_TYPE::UNDEFINED)
        return;

    Texture* pTexture = new Texture();                                                                  // ADD TRY CATCH BLOCK HERE
    pTexture->SetFilePath(fullpath);
    pTexture->SetType(type);
    // m_pTextures.push_front(pTexture);

    // -------------------------------------------------------
    m_currentPerFaceMaterial->AddTexture( pTexture );

    // set the appropriate map flag to true
    if(type == TEXTURE_TYPE::DIFFUSE_MAP)
        m_bDiffuseMap = true;
    else if(type == TEXTURE_TYPE::LIGHT_MAP)
        m_bLightMap = true;
    else if(type == TEXTURE_TYPE::NORMAL_MAP_TANGENT)
        m_bnormalMap = true;
    else if (type == TEXTURE_TYPE::HEIGHT_MAP)
        m_bHeightMap = true;
    else if (type == TEXTURE_TYPE::SHADOW_MAP)
        m_bShadowMap = true;
}
void MeshIOBase::AddTexture(const string& fullpath, TEXTURE_TYPE type)
{
    wstring wpath(fullpath.begin(), fullpath.end()); // convert from ascii to Unicode
    AddTexture(wpath, type);
}
void MeshIOBase::AddTexture(const char* fullpath, TEXTURE_TYPE type)
{
    string szpath(fullpath);
    AddTexture(szpath, type);
}

uint MeshIOBase::GetVertexIndex(const uint& _posIndex, const uint& _textureIndex, const uint& _normalIndex)
{
    // First, lets see if this exact same vertex exist and if it does we can return the index to it
    for(size_t i = 0; i < m_unique_vertices[_posIndex].position_index.size(); ++i)
    {
        if ((m_unique_vertices[_posIndex].position_index[i].m_tex_coord == _textureIndex) &&
            (m_unique_vertices[_posIndex].position_index[i].m_normal_coord == _normalIndex)
            )
        {
            return m_unique_vertices[_posIndex].position_index[i].m_index;
        }
    }

    // else if it doesn't exist we create it and return the index to it
    UV_NORMAL_INDEX temp;  
    temp.m_tex_coord        = _textureIndex;
    temp.m_normal_coord     = _normalIndex;
    temp.m_index            = m_next_available_index;  

    m_unique_vertices[_posIndex].position_index.push_back(temp);

    // Now add the newly created Index to the m_vertices variable
    Vertex tempVert;
    tempVert.Position = m_original_positions[_posIndex];

    // Only copy normals if they actually exist
    if(m_original_normals.size() > 0 || m_temp_normals.size() > 0)
        tempVert.Normal   = m_bUseOriginalNormals ? m_original_normals[_normalIndex] : m_temp_normals[_normalIndex].m_normal;

    // Only Copy UVs if they exist
    if(m_original_uvs.size() > 0 )
        tempVert.TexCoor  = m_original_uvs[_textureIndex];

    m_vertices.push_back(tempVert);

    // add 1 after returning the appropriate index
    return m_next_available_index++;
}

void MeshIOBase::ComputeNormals()
{
    SYSTEM_INFO info;
    GetSystemInfo(&info);

    // Zero the normals
    m_temp_normals.resize(m_original_positions.size());

    const int THREAD_COUNT = info.dwNumberOfProcessors/2;
    vector<std::thread> threads;
    threads.reserve(THREAD_COUNT);

    // First create the list of faces; 3 vertex positions that will encompass a triangle
    forward_list<SGNAME*> SGNamesList;

    // For every material find the smoothing groups it contains and count how many faces each one has
    for_each(m_PerFaceMaterialList.begin(), m_PerFaceMaterialList.end(), [&] (PERFACEMATERIAL* pMat)
    {
        auto _pSGList = pMat->GetSmoothingGroupList();

        // Find the smoothing groups and start counting how many 
        for(auto SG_iter = _pSGList.begin(); SG_iter != _pSGList.end(); ++SG_iter)
        {
            // Find if the smoothing group already exist and add the amount of vertices as a face ( face = 3 vertices)
            auto found = std::find(SGNamesList.begin(), SGNamesList.end(), *(*SG_iter));        // Find out if the smoothing group exists            

            // If it exists, add the amount of vertices as a face 
            if(found != SGNamesList.end())
            {
                (*found)->msize += (*SG_iter)->mPOS_NORMAL_UV.size()/3;
            }
            else
            {
                SGNAME* SGname  = new SGNAME();
                SGname->name    = (*SG_iter)->m_name;
                SGname->msize   += (*SG_iter)->mPOS_NORMAL_UV.size()/3;
                SGNamesList.push_front( SGname );
            }
        }
    });

    // reserve enough memory for all the faces that will go in there
    for_each(SGNamesList.begin(), SGNamesList.end(), [&](SGNAME* _ptr)
    {
        _ptr->mfaces.reserve(_ptr->msize);
    });

    // copy all of the indices into faces
    for_each(m_PerFaceMaterialList.begin(), m_PerFaceMaterialList.end(), [&] (PERFACEMATERIAL* pMat)
    {
        auto _pSGList = pMat->GetSmoothingGroupList();

        // Find the smoothing groups and start counting how many 
        for(auto SGList_iter = _pSGList.begin(); SGList_iter != _pSGList.end(); ++SGList_iter)
        {
            // Find if the smoothing group already exist and add the amount of vertices as a face ( face = 3 vertices)
            auto found = std::find(SGNamesList.begin(), SGNamesList.end(), *(*SGList_iter));    // Find out if the smoothing group exists            
            assert(found != SGNamesList.end());
            vector<POS_NORMAL_UV>& lpos_normals_uv = (*SGList_iter)->mPOS_NORMAL_UV;
            for(size_t i = 0; i < lpos_normals_uv.size()/3; ++i)
            {
                face lface;
                lface.v0 = lpos_normals_uv[i * 3 + 0].vertex_index;
                lface.v1 = lpos_normals_uv[i * 3 + 1].vertex_index;
                lface.v2 = lpos_normals_uv[i * 3 + 2].vertex_index;

                // Get the vertices Position
                const double3& v0 = m_original_positions[lface.v0];
                const double3& v1 = m_original_positions[lface.v1];
                const double3& v2 = m_original_positions[lface.v2];

                // calculate the tangent vectors
                const double3 c0 = v1 - v0;
                const double3 c1 = v2 - v0;

                // Calculate the face normal
                MLVectorCross(c0, c1, lface.faceNormal);
                lface.area = 0.5 * MLVectorLenght(lface.faceNormal); // triangle's area = 1/2 the length of the face's normal

                (*found)->mfaces.push_back( lface );
            }
        }
    });

    for_each(SGNamesList.begin(), SGNamesList.end(), [&] (SGNAME* ptr)
    {
        const size_t THREAD_LOAD_AMOUNT       = ptr->mfaces.size() / THREAD_COUNT;
        const size_t REMAINDER_LOAD           = ptr->mfaces.size() % THREAD_COUNT;        

        if(THREAD_LOAD_AMOUNT == 0)     // skip this iteration if thread load < 1
            return;

        // Divide the work load evenly between the amount of threads available
        for(int ithread = 0; ithread < THREAD_COUNT; ++ithread)
        {
            // Dispatch the first thread with thread_load_amount + remainder
            if(ithread == 0)
            {
                threads.push_back(
                    std::move(
                    thread(
                    ComputeNormalsSmoothest,                    
                    ithread * THREAD_LOAD_AMOUNT, 
                    THREAD_LOAD_AMOUNT + REMAINDER_LOAD, 
                    std::ref(ptr->mfaces), 
                    std::ref(m_original_positions), 
                    std::ref(m_temp_normals))
                    )
                    );
            }
            // Dispatch all other threads with equal amount of load
            else
            {
                threads.push_back( 
                    std::move( 
                    thread( 
                    ComputeNormalsSmoothest,
                    ithread * THREAD_LOAD_AMOUNT + REMAINDER_LOAD,
                    THREAD_LOAD_AMOUNT,
                    std::ref(ptr->mfaces), 
                    std::ref(m_original_positions), 
                    std::ref(m_temp_normals)
                    )
                    )
                    );
            }
        }

        // Wait for all the threads
        for(int i = 0U; i < THREAD_COUNT; ++i)
        {
            threads[i].join();
        }

        // Delete all the old threads
        threads.clear();
    });

    // normalize all the vectors
    for(size_t i = 0; i < m_temp_normals.size(); ++i)
    {
        MLVectorNormalize(m_temp_normals[i].m_normal);
    }

    // Release the memory allocated in the SGNames list.
    EmptyPointersList(SGNamesList);
}

void MeshIOBase::ComputeTangentVectors(void)
{ 
    vector<double3> tempTangent(m_vertices.size());
    vector<double3> tempBiTangent(m_vertices.size());

    for(size_t i = 0; i < m_vertices.size(); ++i)
    {
        tempTangent[i]      = double3(0.0f, 0.0f, 0.0f);
        tempBiTangent[i]    = double3(0.0f, 0.0f, 0.0f);
    }

    for(size_t i = 0; i < m_indices.size()/3; ++i)
    {
        //Get the three tirangle vertices
        uint i0 = m_indices[i*3+0];
        uint i1 = m_indices[i*3+1];
        uint i2 = m_indices[i*3+2];

        //Get the triangle's vertices Positions
        const double3& v0 = m_vertices[i0].Position;
        const double3& v1 = m_vertices[i1].Position;
        const double3& v2 = m_vertices[i2].Position;

        //Get the triangle's vertices texture coordinates
        const double2& u0 = m_vertices[i0].TexCoor;
        const double2& u1 = m_vertices[i1].TexCoor;
        const double2& u2 = m_vertices[i2].TexCoor;

        //calculate the triangle's local basis vector
        double3 q1 = v1 - v0;
        double3 q2 = v2 - v0;

        double s1 = u1.u - u0.u;
        double s2 = u2.u - u0.u;

        double t1 = u1.v - u0.v;
        double t2 = u2.v - u0.v;

        //Compute the tangent Vector
        double r = 1.0f / (s1 * t2 - s2 * t1);	
        const double3& tangent = double3( (t2*q1.x - t1*q2.x * r), 
            (t2*q1.y - t1*q2.y * r), 
            (t2*q1.z - t1*q2.z * r));

        const double3& bitangent = double3( (-s2*q1.x + s1*q2.x)*r,
            (-s2*q1.y + s1*q2.y)*r,
            (-s2*q1.z + s1*q2.z)*r);

        //Average the appropriate tangent space vectors
        tempTangent[i0] += tangent;
        tempTangent[i1] += tangent;
        tempTangent[i2] += tangent;

        tempBiTangent[i0] += bitangent;
        tempBiTangent[i1] += bitangent;
        tempBiTangent[i2] += bitangent;

    }

    // Normalize all the bitangent and binormal vector
    for(size_t i = 0; i < m_vertices.size(); ++i)
    {
        MLVectorNormalize( tempTangent[i] );
        MLVectorNormalize( tempBiTangent[i] );
    }

    // Appply the Grand-Schmit orthogonalization
    for(size_t i = 0; i < m_vertices.size(); ++i)
    {
        Vertex& v = m_vertices[i];
        double3 t = tempTangent[i];
        double3 f = v.Normal * MLVectorDot(t, v.Normal);
        t -= f;	
        v.TangenU = double4(t.x, t.y, t.z, 0.0f);

        // Determine if UV's are mirrored
        if(MLVectorDot(MLVectorCross(v.Normal, t), tempBiTangent[i]) < 0.0f)
            v.TangenU.w = -1.0f;
        else
            v.TangenU.w = +1.0f;
    }
}

Mesh* MeshIOBase::CreateMesh(void)
{
    // Delete materials that are not used and invalid materials
    {
        forward_list<PERFACEMATERIAL*> deleteList;
        for_each(m_PerFaceMaterialList.begin(), m_PerFaceMaterialList.end(), [&](PERFACEMATERIAL* _ptr)
        {
            size_t polygoncount = 0;
            for_each(_ptr->GetSmoothingGroupList().begin(), _ptr->GetSmoothingGroupList().end(), [&] (SMOOTHINGGROUP* _pSG)
            {
                polygoncount += _pSG->mPOS_NORMAL_UV.size();
            });
            if(polygoncount == 0)
                deleteList.push_front(_ptr);
        });
        for_each(deleteList.begin(), deleteList.end(), [&] (PERFACEMATERIAL* _ptr)
        {
            m_PerFaceMaterialList.remove(_ptr);
        });
    }


    // Create a list of the Mesh's materials by name
    for(auto iter = m_PerFaceMaterialList.begin(); iter != m_PerFaceMaterialList.end(); ++iter)
    {
        size_t polygoncount = 0;
        for_each((*iter)->GetSmoothingGroupList().begin(), (*iter)->GetSmoothingGroupList().end(), [&] (SMOOTHINGGROUP* _pSG)
        {
            polygoncount += _pSG->mPOS_NORMAL_UV.size();
        });
    }

    // Create a list of the materials by name
    CreateIndices();
    BuildFacesNormals();
    Mesh* lMesh = RebuildMesh();

    // Release unnecessary RAM memory
    ReleaseUnnecessaryMemory();

    return lMesh;
}

void MeshIOBase::SetDefaultMaterial(void)
{
    // Default material
    m_material.Ambient(0.4f, 0.4f, 0.4f, 1.0f);
    m_material.Diffuse(1.0f, 1.0f, 1.0f, 1.0f);
    m_material.Specular(0.7f, 0.7f, 0.7f, 32.0f);
    m_material.Reflection(0.0f, 0.0f, 0.0f, 1.0f);
}

// ------------------------------------------------------------------------------------|
// This function consolidates all the per-vertex data into a single float array        |
// and passes it into the Mesh. It also sets the proper stride and vertex count values-|
// ------------------------------------------------------------------------------------|
void MeshIOBase::SetVerticesData(Mesh* pMesh) const
{
    // Vertices (UNIFIED)
    float* lpFloat          = nullptr;
    DATA_FORMAT lFormat     = UNFORMATTED;
    const uint lVertexCount = m_vertices.size();
    uint VERTEX_STRIDE      = 0;

    if (m_bDiffuseMap || m_bLightMap)
    {
        if (m_bLightMap && !m_bnormalMap)
        {
            // POS, TEXCOORD
            VERTEX_STRIDE = 5;
            lFormat = POS3_TEX2;
            lpFloat = new float[lVertexCount * VERTEX_STRIDE];
            for(uint i = 0; i < lVertexCount; ++i)
            {
                // positions
                lpFloat[i*VERTEX_STRIDE + 0] = static_cast<float>(m_vertices[i].Position.x);
                lpFloat[i*VERTEX_STRIDE + 1] = static_cast<float>(m_vertices[i].Position.y);
                lpFloat[i*VERTEX_STRIDE + 2] = static_cast<float>(m_vertices[i].Position.z);
                // TexCoords
                lpFloat[i*VERTEX_STRIDE + 3] = static_cast<float>(m_vertices[i].TexCoor.u);
                lpFloat[i*VERTEX_STRIDE + 4] = static_cast<float>(m_vertices[i].TexCoor.v);
            }
        }
        else if(m_bnormalMap)
        {
            // POS, NORMALS, TEXCOORD, TANGENT
            VERTEX_STRIDE = 12;
            lFormat = POS3_TEX2_NORM3_BITAN4;
            lpFloat = new float[lVertexCount * VERTEX_STRIDE];
            for(uint i = 0; i < lVertexCount; ++i)
            {
                // positions
                lpFloat[i*VERTEX_STRIDE + 0] = static_cast<float>(m_vertices[i].Position.x);
                lpFloat[i*VERTEX_STRIDE + 1] = static_cast<float>(m_vertices[i].Position.y);
                lpFloat[i*VERTEX_STRIDE + 2] = static_cast<float>(m_vertices[i].Position.z);
                // TexCoords
                lpFloat[i*VERTEX_STRIDE + 3] = static_cast<float>(m_vertices[i].TexCoor.u);
                lpFloat[i*VERTEX_STRIDE + 4] = static_cast<float>(m_vertices[i].TexCoor.v);
                // normals
                lpFloat[i*VERTEX_STRIDE + 5] = static_cast<float>(m_vertices[i].Normal.x);
                lpFloat[i*VERTEX_STRIDE + 6] = static_cast<float>(m_vertices[i].Normal.y);
                lpFloat[i*VERTEX_STRIDE + 7] = static_cast<float>(m_vertices[i].Normal.z);
                // Bitangent
                lpFloat[i*VERTEX_STRIDE + 8]    = static_cast<float>(m_vertices[i].TangenU.x);
                lpFloat[i*VERTEX_STRIDE + 9]    = static_cast<float>(m_vertices[i].TangenU.y);
                lpFloat[i*VERTEX_STRIDE + 10]   = static_cast<float>(m_vertices[i].TangenU.z);
                lpFloat[i*VERTEX_STRIDE + 11]   = static_cast<float>(m_vertices[i].TangenU.w);

            }
        }
        else if(m_bDiffuseMap)
        {
            // POS, TEXTURES, NORMALS
            VERTEX_STRIDE = 8;
            lFormat = POS3_TEX2_NORM3;
            lpFloat = new float[lVertexCount * VERTEX_STRIDE];
            for(uint i = 0; i < lVertexCount; ++i)
            {
                // positions
                lpFloat[i*VERTEX_STRIDE + 0] = static_cast<float>(m_vertices[i].Position.x);
                lpFloat[i*VERTEX_STRIDE + 1] = static_cast<float>(m_vertices[i].Position.y);
                lpFloat[i*VERTEX_STRIDE + 2] = static_cast<float>(m_vertices[i].Position.z);
                // TexCoords
                lpFloat[i*VERTEX_STRIDE + 3] = static_cast<float>(m_vertices[i].TexCoor.u);
                lpFloat[i*VERTEX_STRIDE + 4] = static_cast<float>(m_vertices[i].TexCoor.v);
                // normals
                lpFloat[i*VERTEX_STRIDE + 5] = static_cast<float>(m_vertices[i].Normal.x);
                lpFloat[i*VERTEX_STRIDE + 6] = static_cast<float>(m_vertices[i].Normal.y);
                lpFloat[i*VERTEX_STRIDE + 7] = static_cast<float>(m_vertices[i].Normal.z);
            }
        }        
    }
    else
    {
        // POS, NORMALS
        VERTEX_STRIDE = 6;
        lFormat = POS3_NORM3;
        lpFloat = new float[lVertexCount * VERTEX_STRIDE];
        for(uint i = 0; i < lVertexCount; ++i)
        {
            // positions
            lpFloat[i*VERTEX_STRIDE + 0] = static_cast<float>(m_vertices[i].Position.x);
            lpFloat[i*VERTEX_STRIDE + 1] = static_cast<float>(m_vertices[i].Position.y);
            lpFloat[i*VERTEX_STRIDE + 2] = static_cast<float>(m_vertices[i].Position.z);
            // normals
            lpFloat[i*VERTEX_STRIDE + 3] = static_cast<float>(m_vertices[i].Normal.x);
            lpFloat[i*VERTEX_STRIDE + 4] = static_cast<float>(m_vertices[i].Normal.y);
            lpFloat[i*VERTEX_STRIDE + 5] = static_cast<float>(m_vertices[i].Normal.z);
        }
    }

    // Now that all the work is done send the consolidated data to the Mesh
    pMesh->SetPerVertexData(lpFloat, lVertexCount, lFormat);
}

void MeshIOBase::ZeroNormals(void)
{
    for(uint i = 0; i < m_vertices.size(); ++i)
        m_vertices[i].Normal = double3(0.0, 0.0, 0.0);
}

void MeshIOBase::CompareForAABB(const float3& f)
{
    // Testing x    
    if ( f.x > m_AABB.m_highest.x )
        m_AABB.m_highest.x = f.x;
    if( f.x < m_AABB.m_lowest.x )
        m_AABB.m_lowest.x = f.x;

    // Testing y
    if( f.y < m_AABB.m_lowest.y )
        m_AABB.m_lowest.y = f.y;
    if ( f.y > m_AABB.m_highest.y )
        m_AABB.m_highest.y = f.y;

    // Testing z
    if( f.z < m_AABB.m_lowest.z )
        m_AABB.m_lowest.z = f.z;
    if ( f.z > m_AABB.m_highest.z )
        m_AABB.m_highest.z = f.z;
}

void MeshIOBase::ExtractFileSNameDirectoryAndFormat(const wstring& inputFile)
{
    m_InputFileFullPath     = inputFile;
    m_InputFileDirectory    = GetFileDirectory( inputFile );
    m_InputFileName         = GetFileName( inputFile );
    m_InputFileFormat       = GetFileFormat( inputFile );
}

void MeshIOBase::FlipIndices(bool originals)
{
    // Flip the faces' order in the Permaterial group
    for_each(m_PerFaceMaterialList.begin(), m_PerFaceMaterialList.end(), [&] (PERFACEMATERIAL* ptr)
    {
        auto SGList_iter = ptr->GetSmoothingGroupList();

        for_each(SGList_iter.begin(), SGList_iter.end(), [&] (SMOOTHINGGROUP* _ptr)
        {
            vector<POS_NORMAL_UV> lIndices = _ptr->mPOS_NORMAL_UV;         // ADD TRY\CATCH BLOCK FOR MEMORY ALLOCATION... CHANGE FUNCTION RETURN TYPE TO BOOL
            for(size_t i = 0; i < _ptr->mPOS_NORMAL_UV.size()/3; ++i)
            {
                // revert winding order
                _ptr->mPOS_NORMAL_UV[i*3 + 0] = lIndices[i*3 + 2];
                _ptr->mPOS_NORMAL_UV[i*3 + 1] = lIndices[i*3 + 1];
                _ptr->mPOS_NORMAL_UV[i*3 + 2] = lIndices[i*3 + 0];
            }
        });
    });

    // flip the indices for the faces normal feature
    vector<POS_NORMAL_UV> lIndices = m_polygon_indices;
    for(size_t i = 0; i < m_polygon_indices.size()/3; ++i)
    {
        // revert winding order
        m_polygon_indices[i*3 + 0] = lIndices[i*3 + 2];
        m_polygon_indices[i*3 + 1] = lIndices[i*3 + 1];
        m_polygon_indices[i*3 + 2] = lIndices[i*3 + 0];
    }    
}

void MeshIOBase::FlipUVs(void)
{
    // Flip only the 'v' coordinate, which  is the only one that DX and OGL differ on
    for(size_t i = 0; i < m_original_uvs.size(); ++i)
    {
        m_original_uvs[i].v = 1.0 - m_original_uvs[i].v;
    }
}

Mesh* MeshIOBase::RebuildMesh(void)
{
    Mesh* lMesh = new Mesh();

    // Set Boudnding Box
    lMesh->SetAxisAlignedBoundingBox(m_AABB);

    // Vertices
    SetVerticesData(lMesh);

    // Set per polygon-group data (indices and Materials)
    lMesh->SetPerPolygonGroupData(m_PerFaceMaterialList);

    //
    SetPerFaceNormalsData(lMesh);

    return lMesh;
}

Mesh* MeshIOBase::FlipFaces(void)
{
    m_bMeshWasFlipped = m_bMeshWasFlipped ? false : true; // Why flipping this here????????????????????????????????????????????????????????????????????????????????????????????????

    // Resize them so there wont be unnexpected memory allocation when creating the final indices
    m_unique_vertices.resize(m_original_positions.size());

    // Now flip the original order of the indices so they are calculated in the right order
    if(m_bMeshWasFlipped)
    {
        FlipIndices(true);

        // Flip the v coordinate of the UV coordinate set
        FlipUVs();

        // If there were no normals and normals ARE needed then allocate ENOUGH space for per-vertex normals
        if(!m_bLightMap)
        {
            // Flip the axis by flipping the z-coordinate
            for(size_t i = 0; i < m_original_positions.size(); ++i)
                m_original_positions[i].z *= -1;

            // Compute the normals now that the faces have been flip, which will make them face the right direction
            ComputeNormals();
        }
    }

    // Create the indices for the now flipped vertices so they are match 1-to-1 with the per vertex attributes
    CreateIndices();

    // After the indices have been created, build the faces' normal
    BuildFacesNormals();

    // Now flip the z-coordinate
    if(m_bMeshWasFlipped)
    {
        if(!m_bLightMap)
            for(size_t i = 0; i < m_original_positions.size(); ++i)
                m_original_positions[i].z *= -1;

        // Flip the v coordinate of the UV coordinate set back to how it was
        FlipUVs();

        // flip the the faces again so they go back to normal
        FlipIndices(true);
    }

    // This is where the Mesh is consolidated so it can be loaded into the pipeline
    Mesh * pMesh = RebuildMesh();

    ReleaseUnnecessaryMemory();
    return pMesh;
}

void MeshIOBase::CreateIndices(void)
{
    // COMMENT THIS HERE: REMEMBER 
    const bool bNeedsNormals    = (!m_bLightMap && m_original_normals.size() == 0);
    m_bUseOriginalNormals       = (m_bMeshWasFlipped || bNeedsNormals) ? false : true;

    const size_t normals_count      = m_original_normals.size();
    const size_t positions_count    = m_original_positions.size();
    const size_t uv_count           = m_original_uvs.size();
    const size_t MAX_VERTEX_COUNT   = max( max(normals_count, positions_count), uv_count );

    m_indices.reserve(m_polygon_indices.size());
    m_unique_vertices.resize(m_original_positions.size());    
    m_vertices.reserve( MAX_VERTEX_COUNT );

    // Before calculating the indices calculate the normals if needed
    if(bNeedsNormals)
        ComputeNormals();
    // For each material build
    for_each(m_PerFaceMaterialList.begin(), m_PerFaceMaterialList.end(), [&] (PERFACEMATERIAL* pMAT)
    {
        // For each smoothing goup that the material has
        for_each(pMAT->GetSmoothingGroupList().begin(), pMAT->GetSmoothingGroupList().end(), [&] (SMOOTHINGGROUP* _ptrSG)
        {
            // For each vertex that uses this material, get the vertex index and add it to the material's index array
            for_each(_ptrSG->mPOS_NORMAL_UV.begin(), _ptrSG->mPOS_NORMAL_UV.end(), [&] (const POS_NORMAL_UV& vertex)
            {
                // Get the index for this particular vertex
                const uint lIndex = GetVertexIndex(vertex.vertex_index, vertex.uv_index, vertex.normal_index);

                // Insert the index into this material's indices array
                pMAT->push_index(lIndex);

                // For the Tangent vector calculation
                m_indices.push_back(lIndex);
            });
        });

        // Now that all the indexes were inputted, get rid of unnecessary memory by
        // moving the indices into the least amount of memory
        pMAT->FormatIndexData(m_next_available_index);
    });

    if(m_bnormalMap)
        ComputeTangentVectors();
}

void MeshIOBase::ReleaseUnnecessaryMemory(void)
{
    m_indices.clear();
    vector<uint>().swap(m_indices);

    m_vertices.clear();
    vector<Vertex>().swap(m_vertices);

    m_unique_vertices.clear();
    vector<positions_indices>().swap(m_unique_vertices);

    m_polygon_faces.clear();
    vector<face>().swap(m_polygon_faces);

    m_temp_normals.clear();
    vector<Lockable_Normal>().swap(m_temp_normals);

    // For each material build
    for_each(m_PerFaceMaterialList.begin(), m_PerFaceMaterialList.end(), [&] (PERFACEMATERIAL* pMAT)
    {
        pMAT->CleanUp();
    });

    m_faces_normals.clear();
    vector<FaceNormal>().swap(m_faces_normals);

    m_next_available_index = 0;
}

void MeshIOBase::BuildFacesNormals(void)
{
    // build the faces
    if(m_polygon_faces.size() == 0)
    {
        m_polygon_faces.reserve(m_polygon_indices.size()/3);

        // Here we calculate the faces and calculate the area of each polygon face of this Mesh
        for(size_t i = 0; i < m_polygon_indices.size()/3; ++i)
        {
            face lFace;

            // Get the triangle's index
            const size_t i0 = m_polygon_indices[i*3+0].vertex_index; // Vertex 0
            const size_t i1 = m_polygon_indices[i*3+1].vertex_index; // Vertex 1
            const size_t i2 = m_polygon_indices[i*3+2].vertex_index; // Vertex 2

            lFace.v0 = i0;
            lFace.v1 = i1;
            lFace.v2 = i2;

            // Get the vertices Position
            const double3& v0 = m_original_positions[i0];
            const double3& v1 = m_original_positions[i1];
            const double3& v2 = m_original_positions[i2];

            // calculate the tangent vectors
            const double3 c0 = v1 - v0;
            const double3 c1 = v2 - v0;

            // Calculate the face normal
            MLVectorCross(c0, c1, lFace.faceNormal);
            lFace.area = 0.5 * MLVectorLenght(lFace.faceNormal); // Triangle area = 1/2 the face's normal length

            // Add the face to the list of faces
            m_polygon_faces.push_back(lFace);
        }
    }

    const double aabb_length = MLVectorLenght(m_AABB.m_highest - m_AABB.m_lowest) * .01;    // Make it 1% of the length
    const double IDEAL_LENGTH = aabb_length < 0.5 ? aabb_length*10 : aabb_length;           // If it might be too short than make it 10%

    // Build the normals at each face for the "Show-Normals" feature
    if(m_faces_normals.size() == 0)
    {
        m_faces_normals.reserve(m_polygon_faces.size() * 2);
        for(size_t index = 0; index < m_polygon_faces.size(); ++index)
        {
            // Local variable
            FaceNormal lfacenormal;

            // Get the triangle's vertices
            const double3& v0 = m_original_positions[ m_polygon_faces[index].v0 ];
            const double3& v1 = m_original_positions[ m_polygon_faces[index].v1 ];
            const double3& v2 = m_original_positions[ m_polygon_faces[index].v2 ];

            // Get the center of the triangle and save it
            const double3 centroid((v0.x + v1.x + v2.x)/3, (v0.y + v1.y + v2.y)/3, (v0.z + v1.z + v2.z)/3);
            const float3 temp(static_cast<float>(centroid.x), static_cast<float>(centroid.y), static_cast<float>(centroid.z));
            lfacenormal.mCentroid = temp;// Copy the centroid

            // Length
            const double centroid_length = MLVectorLenght(centroid)*100.0;

            // Second Vector "Normal to Show"
            double3 facenormal = m_polygon_faces[index].faceNormal;
            MLVectorNormalize( facenormal );

            lfacenormal.mFaceNormal = float3(static_cast<float>(facenormal.x), static_cast<float>(facenormal.y), static_cast<float>(facenormal.z));
            
            /*CALCULATE THE CENTROID TIP*/
            {
                facenormal = facenormal * centroid_length;
                // iterations needed
                uint iterations = 0;
                double length = centroid_length;
                while (length > IDEAL_LENGTH)
                {
                    length /= 2;
                    ++iterations;
                }
                
                // execute iterations
                for(uint i = 0; i < iterations; ++i)
                {
                    facenormal((facenormal.x + centroid.x)/2, (facenormal.y + centroid.y)/2,(facenormal.z + centroid.z)/2);
                }
            }

            const float3 temp1(static_cast<float>(facenormal.x), static_cast<float>(facenormal.y), static_cast<float>(facenormal.z));
            lfacenormal.mCentroidTip = temp1;
            m_faces_normals.push_back(lfacenormal);
        }
    }
}

void MeshIOBase::SetPerFaceNormalsData(Mesh* pMesh)
{
#if defined(DEBUG) || defined(_DEBUG)
    if(m_faces_normals.size() == 0)
    {
        m_pIView->PrintMessage(UI_MSG_TYPE::CRITICAL_ERROR, L"The faces' normals were trying to be set without being calculated first.");
        assert(false);
        return;
    }
#endif
    pMesh->SetFacesNormal(m_faces_normals);
}

void MeshIOBase::ExportMexhToFile(const wstring& filename, FILE_FORMAT type)
{
    m_OutputFileDirectory   = GetFileDirectory(filename);
    m_OutputFileName        = GetFileName(filename);
    m_OutputFileFormat      = GetFileFormatFromEnum(type);

    // First step
    if(m_bMeshWasFlipped)
    {
        FlipIndices(true);

        // Flip the v coordinate of the UV coordinate set
        FlipUVs();

        // If there were no normals and normals ARE needed then allocate ENOUGH space for per-vertex normals
        if(!m_bLightMap)
        {
            // Flip the axis by flipping the z-coordinate
            for(size_t i = 0; i < m_original_positions.size(); ++i)
                m_original_positions[i].z *= -1;

            // Compute the normals now that the faces have been flip, which will make them face the right direction
            ComputeNormals();
        }
    }
    switch (type)
    {
    case OBJ:
        ExportToObjFile();
        break;
    case OBJE:
        ExportToObjeFile();
        break;
    case HND:
        ExportToHndFile();
        break;
    case FBX:
        ExportToFbxFile();
        break;
    }

    // Last step
    if(m_bMeshWasFlipped)
    {
        if(!m_bLightMap)
            for(size_t i = 0; i < m_original_positions.size(); ++i)
                m_original_positions[i].z *= -1;

        // Flip the v coordinate of the UV coordinate set back to how it was
        FlipUVs();

        // flip the the faces again so they go back to normal
        FlipIndices(true);

        ReleaseUnnecessaryMemory();
    }
}

// OBJ FILE Exporting functions
void MeshIOBase::ExportToObjFile(void)
{
    string name(m_OutputFileName.begin(), m_OutputFileName.end());

    string comments     = "# Wavefront OBJ file type.\r\n";
    string mtlib        = "mtllib " + name + ".mtl" + "\r\n";
    string objectname   = "o " + name + ENDOFLINE;

    WriteOBJVerticesData();
    WriteOBJNormalsData();
    WriteOBJUVsData();
    WriteOBJIndicesData();
    WriteOBJMaterials();

    string szfinal;
    szfinal.reserve(
        mtlib.size() + objectname.size() + m_UVsToFile.size() + m_IndicesToFile.size() + 
        m_VerticesToFile.size() + m_NormalsToFile.size() + comments.size());

    szfinal += comments;
    szfinal += mtlib;
    szfinal += objectname;
    szfinal += m_VerticesToFile;
    szfinal += m_UVsToFile;
    szfinal += m_NormalsToFile;    
    szfinal += m_IndicesToFile;

    FILE* File = nullptr;
    wstring wfilename = m_OutputFileDirectory + m_OutputFileName + L'.'+ m_OutputFileFormat;
    string filename(wfilename.begin(), wfilename.end());
    fopen_s(&File, filename.c_str(), "wb");

    fwrite(szfinal.c_str(), szfinal.size(), 1, File);

    fclose(File);

    // Free all the unnecessary memory
    m_VerticesToFile.clear();
    string().swap(m_VerticesToFile);

    m_NormalsToFile.clear();
    string().swap(m_NormalsToFile);

    m_UVsToFile.clear();
    string().swap(m_UVsToFile);

    m_IndicesToFile;
    string().swap(m_IndicesToFile);
}
void MeshIOBase::WriteOBJVerticesData(void)
{
    const size_t SIZE  = 256;
    char characters[SIZE];

    const uint SPACES_COUNT         = 3;
    const uint V_CHARACTER          = 1;
    const uint END_OF_LINE_MARKER   = 2;
    const uint DECIMAL_DATA         = 1 + 6;// the dot "." plus 6 digits
    const uint VERTICES_SPACE       = (V_CHARACTER + SPACES_COUNT + GetLongestPositionDouble(m_AABB) + DECIMAL_DATA + END_OF_LINE_MARKER) * (m_original_positions.size()*3);
    string toFile;
    toFile.reserve(VERTICES_SPACE);

    for(size_t i = 0; i < m_original_positions.size(); ++i)
    {      

        toFile += "v ";                                                     // Write the 'v' plus the initial space

        omi_snprintf(characters, SIZE-1, "%f", m_original_positions[i].x);  // Format the data before anything
        toFile += characters;                                               // Write the x coordinate of the vert
        toFile += ' ';                                                      // Write the space after the coordinate

        omi_snprintf(characters, SIZE-1, "%f", m_original_positions[i].y);  // Format the data before anything
        toFile += characters;                                               // Write the x coordinate of the vert
        toFile += ' ';                                                      // Write the space after the coordinate

        omi_snprintf(characters, SIZE-1, "%f", m_original_positions[i].z);
        toFile += characters;                                               // Write the x coordinate of the vert
        toFile += "\r\n";                                                   // Write the end of line characters
    }

    toFile += "# Vertices count: ";
    omi_snprintf(characters, SIZE-1, "%u",  m_original_positions.size());
    toFile += characters;
    toFile += ENDOFLINE;
    toFile += ENDOFLINE;

    m_VerticesToFile = toFile;
}
void MeshIOBase::WriteOBJNormalsData(void)
{
    const size_t SIZE  = 256;
    char characters[SIZE];
    int count = 0;
    const uint SPACES_COUNT         = 3;
    const uint VN_CHARACTER         = 2;
    const uint END_OF_LINE_MARKER   = 2;
    const uint DECIMAL_DATA         = 1 + 6;// the dot "." plus 6 digits
    const uint NORMALS_SPACE        = (VN_CHARACTER + SPACES_COUNT + 2 + DECIMAL_DATA + END_OF_LINE_MARKER) * ((m_bUseOriginalNormals ? m_original_normals.size() : m_temp_normals.size())*3 );
    string toFile;
    toFile.reserve(NORMALS_SPACE);

    for(size_t i = 0; i < (m_bUseOriginalNormals ? m_original_normals.size() : m_temp_normals.size()); ++i)
    {        
        const double3& arg1 = (m_bUseOriginalNormals ? m_original_normals[i] : m_temp_normals[i].m_normal);      

        toFile += "vn ";                                                // Write the 'v' plus the initial space

        omi_snprintf(characters, SIZE-1, "%f", arg1.x);                 // Format the data before anything
        toFile += characters;                                           // Write the x coordinate of the vert
        toFile += ' ';                                                  // Write the space after the coordinate

        omi_snprintf(characters, SIZE-1, "%f", arg1.y);                 // Format the data before anything
        toFile += characters;                                           // Write the x coordinate of the vert
        toFile += ' ';                                                  // Write the space after the coordinate

        omi_snprintf(characters, SIZE-1, "%f", arg1.z);
        toFile += characters;                                           // Write the x coordinate of the vert
        toFile += ENDOFLINE;                                            // Write the end of line characters
    }    
    toFile += "# Normals count: ";

    const uint size = m_original_normals.size();
    omi_snprintf(characters, SIZE-1, "%u", size);
    toFile += characters;

    toFile += ENDOFLINE;
    toFile += ENDOFLINE;

    m_NormalsToFile = toFile;
}
void MeshIOBase::WriteOBJUVsData(void)
{
    const size_t SIZE  = 257;
    char characters[SIZE];

    const uint SPACES_COUNT         = 2;            // We only support 2 coordinates U and V. The W coordinate is not supported as of December 2013
    const uint VT_CHARACTER         = 2;
    const uint NON_DECIMAL_PART     = 1;
    const uint END_OF_LINE_MARKER   = 2;
    const uint DECIMAL_DATA         = 1 + 6;        // the period "." plus 6 digits
    const uint TEXCOORD_SPACE       = (VT_CHARACTER + SPACES_COUNT + 2 + DECIMAL_DATA + END_OF_LINE_MARKER) * (m_original_uvs.size()*2);

    string toFile;
    toFile.reserve(TEXCOORD_SPACE);

    for(size_t i = 0; i < m_original_uvs.size(); ++i)
    {
        toFile += "vt ";                                                // Write the 'vt' plus the initial space

        // Write the U coordinate plus space at the end
        omi_snprintf(characters, SIZE, "%f", m_original_uvs[i].u);     // Format the data before anything
        toFile += characters;                                           // Write the U coordinate of the vert
        toFile += ' ';                                                  // Write the space after the coordinate

        // Write the V coordinate plus '\r\n' at the end
        omi_snprintf(characters, SIZE, "%f", m_original_uvs[i].v);                      // Format the data before anything
        toFile += characters;                                           // Write the V coordinate of the vert

        toFile += "\r\n";                                               // Write the end of line characters
    }

    toFile += "# UVs count: ";
    omi_snprintf(characters, SIZE-1, "%u", m_original_uvs.size());
    toFile += characters;
    toFile += ENDOFLINE;
    toFile += ENDOFLINE;

    m_UVsToFile = toFile;
}
void MeshIOBase::WriteOBJIndicesData(void)
{
    size_t total_indices = 0;
    for_each(m_PerFaceMaterialList.begin(), m_PerFaceMaterialList.end(), [&] (PERFACEMATERIAL* pMat)
    {
        for_each(pMat->GetSmoothingGroupList().begin(), pMat->GetSmoothingGroupList().end(), [&] (SMOOTHINGGROUP* pSG)
        {
            total_indices += pSG->mPOS_NORMAL_UV.size();
        });
    });

    const size_t SIZE  = 256;
    char characters[SIZE];

    const uint SPACES_COUNT         = 3;
    const uint F_CHARACTER          = 1;
    const uint END_OF_LINE_MARKER   = 2;
    const uint BACK_SLASHES         = 6;
    const uint LONGEST_INDEX        = GetDigitsAmount(static_cast<float>(max(max(m_original_positions.size(), m_original_normals.size()), m_original_uvs.size()) ));
    const uint DECIMAL_DATA         = 0; // Indices have no dot or decimal part
    const uint INDICES_SPACE        = ((F_CHARACTER + SPACES_COUNT + LONGEST_INDEX + BACK_SLASHES + DECIMAL_DATA + END_OF_LINE_MARKER) * total_indices);

    string toFile;
    toFile.reserve(INDICES_SPACE);

    uint trianglecount = 0;
    for_each(m_PerFaceMaterialList.begin(), m_PerFaceMaterialList.end(), [&] (PERFACEMATERIAL* pMat)
    {
        // Write use material
        toFile += "usemtl ";
        toFile += pMat->GetName();
        toFile += ENDOFLINE;

        // For each smoothing group write out 
        const forward_list<SMOOTHINGGROUP*>& _pSmoothingGroupList = pMat->GetSmoothingGroupList();
        for_each(_pSmoothingGroupList.begin(), _pSmoothingGroupList.end(), [&] (SMOOTHINGGROUP* pSG)
        {
            if(pSG->mPOS_NORMAL_UV.size() == 0)
                return;
            else
            {
                trianglecount += pSG->mPOS_NORMAL_UV.size() / 3;
            }
            toFile += "s ";
            toFile += pSG->m_name;
            toFile += ENDOFLINE;

            for(size_t i = 0; i < pSG->mPOS_NORMAL_UV.size()/3; ++i)
            {
                toFile += "f ";
                for(size_t index = 0; index < 3; ++index)
                {
                    // write vertex, texture, normal indices
                    uint arg1 = ((pSG->mPOS_NORMAL_UV)[i * 3 + index].vertex_index) + 1;
                    va_list list;
                    va_start(list, arg1);

                    vsnprintf_s(characters, SIZE-1, "%u", list - sizeof(uint));
                    toFile += characters;
                    toFile += '/';

                    arg1 = ((pSG->mPOS_NORMAL_UV)[i * 3 + index].uv_index) + 1;
                    vsnprintf_s(characters, SIZE-1, "%u", list - sizeof(uint));
                    toFile += characters;                                           
                    toFile += '/';                                                  

                    arg1 = ((pSG->mPOS_NORMAL_UV)[i * 3 + index].normal_index) + 1;
                    vsnprintf_s(characters, SIZE-1, "%u", list - sizeof(uint));
                    toFile += characters;

                    if(index != 2)
                        toFile += ' ';

                    va_end(list);
                }
                toFile += ENDOFLINE;
            }
        });
    });    

    toFile  += "# Triangle count: ";
    omi_snprintf(characters, SIZE-1, "%u", trianglecount);
    toFile += characters;

    toFile += ENDOFLINE;    
    m_IndicesToFile = toFile;
}
void MeshIOBase::WriteOBJMaterials(void)
{
    const size_t SIZE  = 256;
    char characters[SIZE];
    string toFile; 
    toFile.reserve( 200 );

    for_each(m_PerFaceMaterialList.begin(), m_PerFaceMaterialList.end(), [&] (PERFACEMATERIAL* _ptr)
    {  
        // Since the User can change the material of the Mesh once it's been loaded into GPU Memory
        // Here we get the material from the renderer that has the mesh so it can be exported just like the user wants it.
        const wstring& this_material_name = _ptr->GetNameW();
        _ptr->SetMaterial( m_pIView->VGetMaterialProperties(this_material_name) );

        const float4 ambient    = _ptr->GetMaterial().Ambient;
        const float4 diffuse    = _ptr->GetMaterial().Diffuse;
        const float4 specular   = _ptr->GetMaterial().Specular;
        toFile  += "newmtl " + _ptr->GetName() + ENDOFLINE;

        // Write ambient, diffuse, specular and specular factor to the file                

        // Ambient
        {
            toFile += "Ka ";

            omi_snprintf(characters, SIZE-1, "%f", ambient.x); // X
            toFile += characters;
            toFile += ' ';

            omi_snprintf(characters, SIZE-1, "%f", ambient.y);    // Y
            toFile += characters;
            toFile += ' ';

            omi_snprintf(characters, SIZE-1, "%f", ambient.z);    // Z
            toFile += characters;
            toFile += ENDOFLINE;
        }

        // Diffuse
        {
            toFile += "Kd ";

            omi_snprintf(characters, SIZE-1, "%f", diffuse.x);    // X
            toFile += characters;
            toFile += ' ';

            omi_snprintf(characters, SIZE-1, "%f", diffuse.y);    // Y
            toFile += characters;
            toFile += ' ';

            omi_snprintf(characters, SIZE-1, "%f", diffuse.z);    // Z
            toFile += characters;
            toFile += ENDOFLINE;
        }

        // Specular
        {
            toFile += "Ks ";

            omi_snprintf(characters, SIZE-1, "%f", specular.x);    // X
            toFile += characters;
            toFile += ' ';

            omi_snprintf(characters, SIZE-1, "%f", specular.y);    // Y
            toFile += characters;
            toFile += ' ';

            omi_snprintf(characters, SIZE-1, "%f", specular.z);    // Z
            toFile += characters;
            toFile += ENDOFLINE;            
        }

        // Spec term
        {
            toFile += "Ns ";

            omi_snprintf(characters, SIZE-1, "%f", specular.w);    // W = specular factor
            toFile += characters;
            toFile += ENDOFLINE;
        }

        // transparency
        {
            toFile += "d ";
            omi_snprintf(characters, SIZE-1, "%f", diffuse.w);
            toFile += characters;
            toFile += ENDOFLINE;
        }

        // Illumination
        toFile += "illum 2\r\n";

        // textures
        for_each(_ptr->GettexturesList().begin(), _ptr->GettexturesList().end(), [&](const Texture* _pTex)
        {
            {
                wstring wtexnameandformat = GetFileName(_pTex->GetTextureFullPath()) + L'.' + GetFileFormat(_pTex->GetTextureFullPath());
                string  texname(wtexnameandformat.begin(), wtexnameandformat.end());
                toFile += GetTextureTypeForOBJ(_pTex->GetTextureType()) + ' ' + texname + ENDOFLINE;
            }
            m_pIView->VExportTextures(_pTex->GetTextureFullPath(), m_OutputFileDirectory, GetFileFormat(_pTex->GetTextureFullPath()));
        });
    });

    wstring wfullfilename   = m_OutputFileDirectory + m_OutputFileName + L".mtl";
    string fullfilename(wfullfilename.begin(), wfullfilename.end());

    FILE* File = nullptr;
    fopen_s(&File, fullfilename.c_str(), "wb");

    fwrite(toFile.c_str(), toFile.size(), 1, File);

    fclose(File);
}

// FBX FILE Exporting functions
void MeshIOBase::ExportToFbxFile(void)
{

}

// OBJE FILE Exporting functions
void MeshIOBase::ExportToObjeFile(void)
{

}

// HND FILE Exporting functions
void MeshIOBase::ExportToHndFile(void)
{

}

forward_list<wstring> MeshIOBase::GetMaterialNames(void) const
{
    forward_list<wstring> materialnames;
    for_each(m_PerFaceMaterialList.begin(), m_PerFaceMaterialList.end(), [&] (PERFACEMATERIAL* _ptrMaterial)
    {
        materialnames.push_front(_ptrMaterial->GetNameW());
    });
    return materialnames;
}

Material& MeshIOBase::GetMaterialProperties(const wstring& material_name)
{
    auto iter_begin = m_PerFaceMaterialList.begin();
    auto iter_end   = m_PerFaceMaterialList.end();

    // find the material with that name and return its color properties
    while(iter_begin != iter_end)
    {
        if((*iter_begin)->GetNameW() == material_name)
            return (*iter_begin)->GetMaterial();
        ++iter_begin;
    }

    // if the code gets here throw an exception
    assert(iter_begin != iter_end);
    return (*iter_begin)->GetMaterial();
}