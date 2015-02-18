#include "OBJIO.h"
//Public: It's the constructor for the 3dMesh class/variables
OBJMeshIO::OBJMeshIO() : 
    m_vcount(0), m_vtcount(0), m_vncount(0), 
    m_triangles(0),
    m_verticesBegin(0), m_TexturesBegin(0), m_ifacesbegin(0), m_normalsBegin(0),
    m_filebuff(nullptr), m_fileLength(0)
{
    // Empty constructor
}

//Public:
//Action: Given the name of the file containing the data representing
//		  the 3d mesh, this function will read it.
//Post-Cond: The data read from the file will be stored in the m_ variables.
Mesh* OBJMeshIO::VProcessfile(const wstring& inputFile, IView* pIView)
{
    m_pIView = pIView;
    ExtractFileSNameDirectoryAndFormat(inputFile);
    m_format = FILE_FORMAT::OBJ;

    m_filebuff = OpenFile(inputFile.c_str(), m_fileLength, L"rb");

    // If the file could not be found/open
    if(!m_filebuff)
    {
        return nullptr;
    }

    // Count the vertices, Textures and indices		
    if(!CountVertsTextsNorms())
    {
        SAFE_DELETE_ARRAY(m_filebuff);
        return nullptr;
    }

    // process the materials and the library
    GetMaterialAndTextures(m_InputFileDirectory, m_InputFileName);

    if(m_TexturesBegin > 0)
        m_original_uvs.reserve(m_vtcount);

    m_polygon_indices.reserve(m_triangles*3);// Three indices per polygon face

    if(!m_bLightMap)
        m_original_normals.reserve(m_vncount); // Only save space for the normals if there is not a light map

    m_original_positions.reserve(m_vcount);

    //Read Faces' data
    if(m_ifacesbegin == 0)
    {
        m_pIView->PrintMessage(UI_MSG_TYPE::CRITICAL_ERROR, L"There was a problem reading the Indices' data, ensure the data is formatted correctly.");
        SAFE_DELETE_ARRAY(m_filebuff);
        return nullptr;
    }
    else
    {
        ReadIndices();
    }

    // Read Vertices' data
    if(m_verticesBegin == 0)
    {
        m_pIView->PrintMessage(UI_MSG_TYPE::CRITICAL_ERROR, L"There was a problem reading the vertices data, ensure the data is formatted correctly.");
        SAFE_DELETE_ARRAY(m_filebuff);
        return nullptr;
    }
    else
        ReadVertices();

    // Read normals, if they exist or zero them out if they dont to compute them later
    if(m_normalsBegin>0)
    {
        if(!ReadNormals())
        {
            m_pIView->PrintMessage(UI_MSG_TYPE::CRITICAL_ERROR, L"There was a problem reading the normals' data, ensure the data is formatted correctly.");
            SAFE_DELETE_ARRAY(m_filebuff);
            return nullptr;
        }
    }

    // Read textures, if they exist
    if(m_TexturesBegin>0)
    {
        if(!ReadTextureCoord())
        {
            m_pIView->PrintMessage(UI_MSG_TYPE::CRITICAL_ERROR, L"There was a problem reading the Texture-Coord's data, ensure the data is formatted correctly.");
            SAFE_DELETE_ARRAY(m_filebuff);
            return false;
        }
    }
    return CreateMesh();
}

//Private:
//Action: Counts the m_vertices, normals, and texture coordinates
//Post-Cond: All the right index locations are for respective data's start
//			 and also finds out if anything is missing
bool OBJMeshIO::CountVertsTextsNorms()
{
    size_t index = 1;

    // Skip all the data until the first 'v' coordinate is found
    while (m_filebuff[index] != 'v' ||  m_filebuff[index+1] != ' ' || m_filebuff[index-1] != '\n' )
        ++index;

    // Set m_startingPosition to the index where readable data begins
    m_verticesBegin = index;

    for(index; index<m_fileLength-1; ++index)
    {
        if(m_filebuff[index+1] == ' ' && m_filebuff[index] == 'v')
        {
            while(m_filebuff[index+1] == ' ' && m_filebuff[index] == 'v')
            {
                // Increment the texture counter
                m_vcount += 1;

                // Skip the rest of the line
                while(m_filebuff[index] != '\n')
                    ++index;

                index+=1;// make the file index go past the '\n' character
            }

            // This is to compensate for the for-loop incrementing
            // the file index twice and placing one character ahead
            // of the first character of the line
            index-=1;
        }

        //--------------|
        //Count Textures|
        //--------------|
        else if(m_filebuff[index+1] == 't' && m_filebuff[index] == 'v')
        {
            m_TexturesBegin = index;// Mark the coordinate for where to start reading texture coordinates

            while(m_filebuff[index+1] == 't' && m_filebuff[index] == 'v')
            {
                m_vtcount += 1; // Increment the texture counter

                // Skip the rest of the line
                while(m_filebuff[index] != '\n')
                    ++index;

                index+=1;// make the file index go past the '\n' character
            }

            // This is to compensate for the for-loop incrementing
            // the file index twice and placing one character ahead
            // of the first character of the line
            index-=1;
        }

        //--------------|
        //Count Normals	|
        //--------------|
        else if(m_filebuff[index+1] == 'n' && m_filebuff[index] == 'v' && m_filebuff[index-1] == '\n')
        {

            // Mark the coordinate for when reading Normals' data begins
            m_normalsBegin = index;

            while(m_filebuff[index+1] == 'n' && m_filebuff[index] == 'v' && m_filebuff[index-1] == '\n')
            {
                // Increment the Normal counter
                m_vncount += 1;

                //Skip the rest of the line
                while(m_filebuff[index] != '\n')
                    ++index;                

                index+=1;// make the file index go past the '\n' character
            }

            // This is to compensate for the for-loop incrementing
            // the file index twice and placing one character ahead
            // of the first character of the line
            index-=1;
        }

        //-----------|
        //Count Faces|
        //-----------|
        else if( 
            (m_filebuff[index - 1]    == '\n')  && 
            (m_filebuff[index + 0]    == 'u')   && 
            (m_filebuff[index + 1]    == 's' )  && 
            (m_filebuff[index + 2]    == 'e' )  &&
            (m_filebuff[index + 3]    == 'm' )  &&
            (m_filebuff[index + 4]    == 't' )  &&
            (m_filebuff[index + 5]    == 'l' )
            )
        {
            m_ifacesbegin   = index;    // Mark the coordinate for when reading Faces' data begins
            int spaceCount  = 0;        // To check if it's a triangulated face or Quad face

            while( (m_filebuff[index] != 'f' || m_filebuff[index-1] != '\n') )
                ++index;

            while(m_filebuff[index] == 'f' && m_filebuff[index-1] == '\n')
            {
                index+=2;// this places the index at the first number
                spaceCount = 0;// Ensuring spaceCount is re-initialized

                while( m_filebuff[index] != '\n')
                {
                    if(m_filebuff[index] == ' ' && m_filebuff[index+1] != '\r' && m_filebuff[index+1] != '\n' )// track spaces that are not at the end of the line
                        spaceCount += 1;
                    ++index;
                }		

                while((m_filebuff[index] != 'f' || m_filebuff[index-1] != '\n') && index<m_fileLength-2)
                    index += 1;

                // If the face has more than 3 spaces it cannot be properly read, only quads and triags for now

                // Triangle
                if(spaceCount == 2)
                {
                    m_triangles+=1;
                }
                // Quad 
                else if(spaceCount == 3)
                {
                    m_triangles += 2;
                }
                // Bigger than Quad - Trigger error
                else
                {
                    m_pIView->PrintMessage(UI_MSG_TYPE::CRITICAL_ERROR, L"Error, encountered a 5-vertex or higher polygon, which is not supported.");
                    return false;
                }

            }
        }
    }// Main for-loop
    return true;
}

//Private:
//Action: Reads all the Vertices position into m_vertices respective Position variable
//Post-Cond: All the values Vertex values are inputed into m_vertices.
bool OBJMeshIO::ReadVertices()
{
    //Set it to where the vertices begin
    uint file_index = m_verticesBegin; // Add 2 so it can start reading data from the first *valid character
    char szInput [20];

    double flt_inputs[3] = {0.0, 0.0, 0.0};

    double3 lVeretx;
    double3 low, high;

    for (uint vertex_count = 0; vertex_count < m_vcount; ++vertex_count)
    {
        MoveToNextSpace(m_filebuff, file_index);
        SkipAllSpaces(m_filebuff, file_index);
        // Read the 3 vertex coordinates (x, y, z)
        for(uint vertex = 0; vertex<3; ++vertex, ++file_index)
        {
            uint szIndex = 0;
            while(m_filebuff[file_index] != ' ' && m_filebuff[file_index] != '\r' && m_filebuff[file_index] != '\n')
            { 
                szInput[szIndex] = m_filebuff[file_index];
                ++szIndex;
                ++file_index;
            } 
            //Insert a null terminator to prevent buffer over-run
            szInput[szIndex] = '\0';

            // Skip the rest of the line
            if(vertex == 2)
                MoveToNextLine(m_filebuff, file_index);

            // Convert from char * to float with the ctof() inlined function
            // and assign the value to the corresponding flt_inputs[0,1,2]
            flt_inputs[vertex] = atof(szInput);
        }

        // Input the vertices and convert them to inches
        lVeretx.x = flt_inputs[0];
        lVeretx.y = flt_inputs[1];
        lVeretx.z = flt_inputs[2];
        m_original_positions.push_back(lVeretx);

        // Calculate if the new vertex position is lower/higher than the current lowest/highest
        CompareForAABB(lVeretx);
    }
    return true;
}

//Private:
//Action: Reads all the Normals into m_vertices respective Normal variable
//Post-Cond: All the Vertex Normal values are inputed into m_vertices.
bool OBJMeshIO::ReadNormals()
{
    uint file_index = m_normalsBegin+3;
    char szInput [20];
    double flt_inputs[3] = {0.0f, 0.0f, 0.0f};
    double3 f3;

    for (uint vector_index = 0; vector_index<m_vncount; ++vector_index)
    {
        // skip any extra spaces
        while(m_filebuff[file_index] == ' ')
            file_index += 1;

        // Read the 3 Normal coordinates (x, y, z)
        for(int i = 0; i<3; ++i, ++file_index)
        {
            uint szIndex = 0;
            while(m_filebuff[file_index] != ' ' && m_filebuff[file_index] != '\n' && m_filebuff[file_index] != '\r')
            { 
                szInput[szIndex] = m_filebuff[file_index];	  
                ++szIndex;	  
                ++file_index;
            }

            // Insert a null terminator to prevent buffer over-run
            szInput[szIndex] = '\0';

            //Skip the rest of the line
            if(i == 2)
            {
                while(m_filebuff[file_index] != '\n')
                    file_index += 1;
            }
            //assign the value to the corresponding flt_inputs[0,1,2]
            flt_inputs[i] = atof(szInput);
        }

        f3.x = flt_inputs[0];
        f3.y = flt_inputs[1];
        f3.z = flt_inputs[2];	
        m_original_normals.push_back(f3); // m_tempNorm is to save normals before the final normals are computed

        // Add 3 from 'v' 'n' so index will be at first valid character
        file_index+=3;
    }
    return true;
}

//Private:
//Action: Reads all the texture-coordinates into m_vertices respective TEX-Coordinate variable
//Post-Cond: All the texture coordinates values are inputed into m_vertices.
bool OBJMeshIO::ReadTextureCoord()
{
    uint file_index = m_TexturesBegin+3;//Add 3 from 't'
    char szInput [20];
    double flt_inputs[2] = {0.0f, 0.0f};
    double2 f2;

    for (unsigned int vector_index = 0; vector_index < m_vtcount; ++vector_index)
    {
        //skip any extra spaces
        while(m_filebuff[file_index] == ' ')
            file_index += 1;

        //Read the 2 Texture coordinates (u, v) NO w-coord
        for(int i = 0; i<2; ++i, ++file_index)
        {
            uint szIndex = 0;
            while(m_filebuff[file_index] != ' ' && m_filebuff[file_index] != '\r' && m_filebuff[file_index] != '\n')
            { 
                szInput[szIndex] = m_filebuff[file_index];
                ++szIndex;
                ++file_index;
            }

            // Insert a null terminator to prevent buffer over-run
            szInput[szIndex] = '\0';

            // and assign the value to the corresponding flt_inputs[0,1]
            flt_inputs[i] = atof(szInput);
        }

        //Input the m_vertices into the "vector<Vertex> m_vertices"
        f2.u = flt_inputs[0];
        f2.v = flt_inputs[1];
        m_original_uvs.push_back(f2);

        //Skip the w coordinate and the rest of the line
        while (m_filebuff[file_index] != '\n')
            ++file_index;

        //Increment to the next valid character
        file_index += 3;
    }
    return true;
}

//Private:
//Action: Reads all the Indices' data into m_indices variable
//Post-Cond: All the indices for Positions, Normals, and Texture coordinates
//		     are saved for further processing by RedoVertsAndFaces()
bool OBJMeshIO::ReadIndices()
{
    size_t file_index = m_ifacesbegin; // This will make it start where the indices begin

    vector<uint> pInt_inputs(12);
    char szInput [20]; // to pass into ctoi() function

    // Read all the triangles that the Mesh contains. Keep in mind that the value 
    // of m_triagles was obtained in CountVertsTextsNorms()
    for(size_t totalfaces = 0; totalfaces < m_triangles;)
    {	
        // For internal use in this for-loop
        std::size_t space_counter = 0, intArrayIndex = 0, szIndex = 0;

        // We always check to make sure that the material or the smoothing group is not changing before we proceed to reading characters        
        if(m_filebuff[file_index] == 'u')           // Switching materials?
        {
            ReadMaterial(file_index);
            MoveToNextLine(m_filebuff, file_index);
            continue;
        }
        else if(m_filebuff[file_index] == 's')      // Switching smoothing group?
        {
            ReadSmoothingGroupDescription(file_index);
            MoveToNextLine(m_filebuff, file_index);
            continue;
        }

        // skip the 'f' character
        MoveToNextSpace(m_filebuff, file_index);

        // skip ALL space character
        while(m_filebuff[file_index] == ' ')
            ++file_index;

        // Read all valid characters until the end of the line
        while (m_filebuff[file_index] != '\r' && m_filebuff[file_index] != '\n')
        {
            if(m_filebuff[file_index] == '/') // the '/' character is used as a separator between vertex, textures, and normal coordinates
            {
                szInput[szIndex] = '\0';
                pInt_inputs[intArrayIndex++] = atoi(szInput) - 1;
                szIndex = 0; // reset the char* index value
            }
            else if(m_filebuff[file_index] == ' ') // If it's a space then we need to read the next vertex index
            {
                szInput[szIndex] = '\0';
                pInt_inputs[intArrayIndex++] = atoi(szInput) - 1;
                szIndex = 0; // reset the char* index value

                // Keep track of how many spaces we encounter in this face
                if(m_filebuff[file_index+1] != '\r' && m_filebuff[file_index+1] != '\n')
                    space_counter += 1;
                while(m_filebuff[file_index] == ' ')
                    ++file_index;

                --file_index;
            }
            else // if it's not a separator '/' and if it's not a space then treat it as a normal numerical character
            {
                if(m_filebuff[file_index] != '-' && m_filebuff[file_index] != '\r' && m_filebuff[file_index] != ' ')
                {
                    szInput[szIndex] = m_filebuff[file_index];  // Read the character
                    szIndex += 1;                               // increment char* index
                }
            }

            // always increment file buffer index here
            file_index += 1;
        }

        // Sometimes a face will end right before an '\r' or an '\n' character and we need to make sure we read that value
        if( (m_filebuff[file_index] == '\r' || m_filebuff[file_index] == '\n') && (intArrayIndex<12))
        {
            szInput[szIndex] = '\0';
            pInt_inputs[intArrayIndex] = atoi(szInput) - 1;
        }

        // -----------------------------------------------------------------------------------------------------------------|
        // Count how many faces we processed:   2 spaces means it was one triangulated face                                 |
        //                                      3 spaces means it was 1-quadface thus two triangulated faces                |
        // NOTE: we could add support for more faces but we dont want to do that                                            |
        // -----------------------------------------------------------------------------------------------------------------|
        if(space_counter == 2)
            totalfaces += 1;
        else
            totalfaces += 2;

        // skip all the characrters until we reach the next face to be read
        MoveToNextLine(m_filebuff, file_index);

        // Read the face
        ReadFace(space_counter, &pInt_inputs[0]);
    }

    return true;
}

// Private:
// Action: This function reads and triangulates the current polygon(face), and determines the index 
//		for this particular face at each of its vertex positions
// Post-Cond: 
void OBJMeshIO::ReadFace(const uint& _spaces, const uint* _pIntArray)
{
    assert(m_currentPerFaceMaterial != nullptr);
    const size_t TRIANGLE_FACE    = 2;
    const size_t QUAD_FACE        = 3;
    POS_NORMAL_UV lvertex1, lvertex2, lvertex3, lvertex4;
    face lFace;

    // each face read is subtracted one because OBJ files indices start at 1, but arrays DO-NOT
    switch(_spaces)
    {        
    case TRIANGLE_FACE:     // Reading triangulated face
        {
            // The mesh has no uv or normal coordinates
            if(m_TexturesBegin == 0 && m_normalsBegin == 0)     
            {
                // Vertex 1
                lvertex1(_pIntArray[0], 0, 0);
                m_currentPerFaceMaterial->push_vertex(lvertex1.vertex_index, lvertex1.normal_index, lvertex1.uv_index); // insert it into the current material
                m_polygon_indices.push_back(lvertex1);

                // Vertex 2
                lvertex2(_pIntArray[1], 0, 0);
                m_currentPerFaceMaterial->push_vertex(lvertex2.vertex_index, lvertex2.normal_index, lvertex2.uv_index); // insert it into the current material
                m_polygon_indices.push_back(lvertex2);

                // Vertex 3
                lvertex3(_pIntArray[2], 0, 0);
                m_currentPerFaceMaterial->push_vertex(lvertex3.vertex_index, lvertex3.normal_index, lvertex3.uv_index); // insert it into the current material
                m_polygon_indices.push_back(lvertex3);
            }

            // The mesh has textures but has no normals coordinate
            else if(m_TexturesBegin > 0 && m_normalsBegin == 0)     
            {
                // Vertex 1
                lvertex1(_pIntArray[0], _pIntArray[1], _pIntArray[0]);
                m_currentPerFaceMaterial->push_vertex(lvertex1.vertex_index, lvertex1.normal_index, lvertex1.uv_index); // insert it into the current material
                m_polygon_indices.push_back(lvertex1);

                // Vertex 2
                lvertex2(_pIntArray[2],_pIntArray[3], _pIntArray[2]);
                m_currentPerFaceMaterial->push_vertex(lvertex2.vertex_index, lvertex2.normal_index, lvertex2.uv_index); // insert it into the current material
                m_polygon_indices.push_back(lvertex2);

                // Vertex 3
                lvertex3(_pIntArray[4], _pIntArray[5], _pIntArray[4]);
                m_currentPerFaceMaterial->push_vertex(lvertex3.vertex_index, lvertex3.normal_index, lvertex3.uv_index); // insert it into the current material
                m_polygon_indices.push_back(lvertex3);
            }

            // Textures DO NOT exist but normals do
            else if(m_TexturesBegin == 0 && m_normalsBegin > 0)     
            {
                // Vertex 1
                lvertex1(_pIntArray[0], 0, _pIntArray[1]);
                m_currentPerFaceMaterial->push_vertex(lvertex1.vertex_index, lvertex1.normal_index, lvertex1.uv_index); // insert it into the current material
                m_polygon_indices.push_back(lvertex1);

                // Vertex 2
                lvertex2(_pIntArray[2], 0,_pIntArray[3]);
                m_currentPerFaceMaterial->push_vertex(lvertex2.vertex_index, lvertex2.normal_index, lvertex2.uv_index); // insert it into the current material
                m_polygon_indices.push_back(lvertex2);

                // Vertex 3
                lvertex3(_pIntArray[4], 0, _pIntArray[5]);
                m_currentPerFaceMaterial->push_vertex(lvertex3.vertex_index, lvertex3.normal_index, lvertex3.uv_index); // insert it into the current material
                m_polygon_indices.push_back(lvertex3);
            }

            // Both normals and Textures exist
            else        
            {
                // Vertex 1
                lvertex1(_pIntArray[0], _pIntArray[1], _pIntArray[2]);
                m_currentPerFaceMaterial->push_vertex(lvertex1.vertex_index, lvertex1.normal_index, lvertex1.uv_index); // insert it into the current material
                m_polygon_indices.push_back(lvertex1);

                // Vertex 2
                lvertex2(_pIntArray[3],_pIntArray[4],_pIntArray[5]); 
                m_currentPerFaceMaterial->push_vertex(lvertex2.vertex_index, lvertex2.normal_index, lvertex2.uv_index); // insert it into the current material
                m_polygon_indices.push_back(lvertex2);

                // Vertex 3
                lvertex3(_pIntArray[6], _pIntArray[7], _pIntArray[8]);
                m_currentPerFaceMaterial->push_vertex(lvertex3.vertex_index, lvertex3.normal_index, lvertex3.uv_index); // insert it into the current material
                m_polygon_indices.push_back(lvertex3);
            }
        }
        break;
    case QUAD_FACE:         // Reading Quadface
        {
            // The mesh has no uv or normal coordinates
            if(m_TexturesBegin == 0 && m_normalsBegin == 0)     
            {
                // Triangle 1
                {
                    // Vertex 1
                    lvertex1(_pIntArray[0], 0, 0);
                    m_currentPerFaceMaterial->push_vertex(lvertex1.vertex_index, lvertex1.normal_index, lvertex1.uv_index); // insert it into the current material
                    m_polygon_indices.push_back(lvertex1);

                    // Vertex 2
                    lvertex2(_pIntArray[1], 0, 0);
                    m_currentPerFaceMaterial->push_vertex(lvertex2.vertex_index, lvertex2.normal_index, lvertex2.uv_index); // insert it into the current material
                    m_polygon_indices.push_back(lvertex2);

                    // Vertex 3
                    lvertex3(_pIntArray[2], 0, 0);
                    m_currentPerFaceMaterial->push_vertex(lvertex3.vertex_index, lvertex3.normal_index, lvertex3.uv_index); // insert it into the current material
                    m_polygon_indices.push_back(lvertex3);
                }

                // Triangle 2
                {
                    // Vertex 1
                    m_currentPerFaceMaterial->push_vertex(lvertex1.vertex_index, lvertex1.normal_index, lvertex1.uv_index); // insert it into the current material
                    m_polygon_indices.push_back(lvertex1);
                    // Vertex 3
                    m_currentPerFaceMaterial->push_vertex(lvertex3.vertex_index, lvertex3.normal_index, lvertex3.uv_index); // insert it into the current material
                    m_polygon_indices.push_back(lvertex3);
                    // Vertex 4
                    lvertex4(_pIntArray[3], 0, 0);
                    m_currentPerFaceMaterial->push_vertex(lvertex4.vertex_index, lvertex4.normal_index, lvertex4.uv_index); // insert it into the current material
                    m_polygon_indices.push_back(lvertex4);
                }
            }
            
            // The mesh has textures but has no normals coordinates
            else if(m_TexturesBegin > 0 && m_normalsBegin == 0)     
            {
                // Triangle 1
                {
                    // Vertex 1
                    lvertex1(_pIntArray[0], _pIntArray[1], _pIntArray[0]);
                    m_currentPerFaceMaterial->push_vertex(lvertex1.vertex_index, lvertex1.normal_index, lvertex1.uv_index); // insert it into the current material
                    m_polygon_indices.push_back(lvertex1);

                    // Vertex 2
                    lvertex2(_pIntArray[2],_pIntArray[3], _pIntArray[2]);
                    m_currentPerFaceMaterial->push_vertex(lvertex2.vertex_index, lvertex2.normal_index, lvertex2.uv_index); // insert it into the current material
                    m_polygon_indices.push_back(lvertex2);

                    // Vertex 3
                    lvertex3(_pIntArray[4], _pIntArray[5], _pIntArray[4]);
                    m_currentPerFaceMaterial->push_vertex(lvertex3.vertex_index, lvertex3.normal_index, lvertex3.uv_index); // insert it into the current material
                    m_polygon_indices.push_back(lvertex3);
                }

                // Triangle 2
                {
                    // Vertex 1
                    m_currentPerFaceMaterial->push_vertex(lvertex1.vertex_index, lvertex1.normal_index, lvertex1.uv_index); // insert it into the current material
                    m_polygon_indices.push_back(lvertex1);
                    // Vertex 3
                    m_currentPerFaceMaterial->push_vertex(lvertex3.vertex_index, lvertex3.normal_index, lvertex3.uv_index); // insert it into the current material
                    m_polygon_indices.push_back(lvertex3);
                    // Vertex 4
                    lvertex4(_pIntArray[6], _pIntArray[7], _pIntArray[6]);
                    m_currentPerFaceMaterial->push_vertex(lvertex4.vertex_index, lvertex4.normal_index, lvertex4.uv_index); // insert it into the current material
                    m_polygon_indices.push_back(lvertex4);
                }
            }
            else if(m_TexturesBegin == 0 && m_normalsBegin > 0)     // Textures DO NOT exist but normals do
            {
                // Triangle 1
                {
                    // Vertex 1
                    lvertex1(_pIntArray[0], 0, _pIntArray[1]);
                    m_currentPerFaceMaterial->push_vertex(lvertex1.vertex_index, lvertex1.normal_index, lvertex1.uv_index); // insert it into the current material
                    m_polygon_indices.push_back(lvertex1);

                    // Vertex 2
                    lvertex2(_pIntArray[2], 0,_pIntArray[3]);
                    m_currentPerFaceMaterial->push_vertex(lvertex2.vertex_index, lvertex2.normal_index, lvertex2.uv_index); // insert it into the current material
                    m_polygon_indices.push_back(lvertex2);

                    // Vertex 3
                    lvertex3(_pIntArray[4], 0, _pIntArray[5]);
                    m_currentPerFaceMaterial->push_vertex(lvertex3.vertex_index, lvertex3.normal_index, lvertex3.uv_index); // insert it into the current material
                    m_polygon_indices.push_back(lvertex3);
                }

                // Triangle 2
                {
                    // Vertex 1
                    m_currentPerFaceMaterial->push_vertex(lvertex1.vertex_index, lvertex1.normal_index, lvertex1.uv_index); // insert it into the current material
                    m_polygon_indices.push_back(lvertex1);
                    // Vertex 3
                    m_currentPerFaceMaterial->push_vertex(lvertex3.vertex_index, lvertex3.normal_index, lvertex3.uv_index); // insert it into the current material
                    m_polygon_indices.push_back(lvertex3);
                    // Vertex 4
                    lvertex4(_pIntArray[6], 0, _pIntArray[7]);
                    m_currentPerFaceMaterial->push_vertex(lvertex4.vertex_index, lvertex4.normal_index, lvertex4.uv_index); // insert it into the current material
                    m_polygon_indices.push_back(lvertex4);
                }
            }
            else        // Both normals and Textures exist
            {
                // Triangle 1
                {
                    // Vertex 1
                    lvertex1(_pIntArray[0], _pIntArray[1], _pIntArray[2]);
                    m_currentPerFaceMaterial->push_vertex(lvertex1.vertex_index, lvertex1.normal_index, lvertex1.uv_index); // insert it into the current material
                    m_polygon_indices.push_back(lvertex1);

                    // Vertex 2
                    lvertex2(_pIntArray[3],_pIntArray[4],_pIntArray[5]); 
                    m_currentPerFaceMaterial->push_vertex(lvertex2.vertex_index, lvertex2.normal_index, lvertex2.uv_index); // insert it into the current material
                    m_polygon_indices.push_back(lvertex2);

                    // Vertex 3
                    lvertex3(_pIntArray[6], _pIntArray[7], _pIntArray[8]);
                    m_currentPerFaceMaterial->push_vertex(lvertex3.vertex_index, lvertex3.normal_index, lvertex3.uv_index); // insert it into the current material
                    m_polygon_indices.push_back(lvertex3);
                }

                // Triangle 2
                {
                    // Vertex 1
                    m_currentPerFaceMaterial->push_vertex(lvertex1.vertex_index, lvertex1.normal_index, lvertex1.uv_index); // insert it into the current material
                    m_polygon_indices.push_back(lvertex1);
                    // Vertex 3
                    m_currentPerFaceMaterial->push_vertex(lvertex3.vertex_index, lvertex3.normal_index, lvertex3.uv_index); // insert it into the current material
                    m_polygon_indices.push_back(lvertex3);
                    // Vertex 4
                    lvertex4(_pIntArray[9], _pIntArray[10], _pIntArray[11]);
                    m_currentPerFaceMaterial->push_vertex(lvertex4.vertex_index, lvertex4.normal_index, lvertex4.uv_index); // insert it into the current material
                    m_polygon_indices.push_back(lvertex4);
                }
            }
        }
        break;        
    }
}

// Reads the .mtl file associated with the obj file
void OBJMeshIO::GetMaterialAndTextures(const wstring& fileDirectory, const wstring& filename)
{
    wstring file;
    const int FORMAT_SIZE = 5;
    file.reserve(fileDirectory.size() + filename.size() + FORMAT_SIZE);
    file += fileDirectory;
    file += filename;
    file += L'.';
    file += L"mtl";
    file.reserve(filename.length() + 1);

    size_t file_length  = 0;
    const char* mtlib   = OpenFile(file.c_str(), file_length, L"rb");
    if(!mtlib)
        return; //////////////////////////////////////////////////////////////////////////////////////////////////////// ADD DEBUG MESSAGE

    string szToken;
    szToken.reserve(100);

    bool material_found = false;

    for(size_t index = 0; index < file_length; ++index)
    {
        // skip any comments
        if(mtlib[index] == '#')
        {
            while(mtlib[index] != '\n')
                ++index;
            continue;
        }

        // Skip tabs '\t'
        while(mtlib[index] == '\t')
            ++index;

        // Get the token
        while(mtlib[index] != ' ' && mtlib[index] != '\n' && mtlib[index] != '\r')
        {
            szToken+= mtlib[index];
            ++index;
        }

        // Process token
        if(szToken[0] == 'K')
        {
            float4 color;
            color.w = 1.0f;
            material_found = true;
            // Material Data
            if(szToken == "Ka")
            {
                ReadThreeFloat(szToken, mtlib, index, color);  // Ambient
                m_currentPerFaceMaterial->SetMaterialAmbient(color);
            }
            else if(szToken == "Kd")
            {
                ReadThreeFloat(szToken, mtlib, index, color);  // Diffuse
                m_currentPerFaceMaterial->SetMaterialDiffuse(color);
            }
            else if(szToken == "Ks")
            {
                ReadThreeFloat(szToken, mtlib, index, color); // Specular
                m_currentPerFaceMaterial->SetMaterialSpecular(color);
            }
        }
        else if(szToken[0] == 'm' && (szToken[1] == 'a' && szToken[2] == 'p')) // textures: diffuse, spec, ambient, and normal maps
        {
            TEXTURE_TYPE lTextureType = UNDEFINED;
            if(szToken == "map_Kd")
            {
                lTextureType = DIFFUSE_MAP;
            }
            else if(szToken == "map_Ks")
            {
                lTextureType = SPECULAR_MAP;
            }
            else if(szToken == "map_bump")
            {
                //map_bump
                lTextureType = NORMAL_MAP_TANGENT;
            }

            wstring texture;
            GetTextureFileName(texture, mtlib, index);
            AddTexture(texture, lTextureType);

        }
        else if(szToken == "Ns")
        {
            float flt;
            ReadOneFloat(szToken, mtlib, index, flt);
            m_currentPerFaceMaterial->SetMaterialSpecularTerm(flt);
        }
        else if(szToken== "newmtl") // Material
        {
            // Material name
            ReadMaterial(mtlib, index);
        }
        else if(szToken == "d" || szToken == "Tr")
        {
            // read material's transparency
            const bool flip_it = szToken == "Tr" ? true : false;
            float flt;
            
            ReadOneFloat(szToken, mtlib, index, flt);

            if(flip_it)            
                flt = 1.0f - flt;
            
            m_currentPerFaceMaterial->SetMaterialAlphaChannel(flt);
        }
        else
        {
            while(mtlib[index] != '\n' && mtlib[index] != EOF)
                ++index;
        }
        // clear the string to properly start over
        szToken.clear();
    }
    SAFE_DELETE_ARRAY(mtlib);

    // if no material information was found apply the default material
    if(!material_found)
        SetDefaultMaterial();
}

void OBJMeshIO::ReadOneFloat(string& str, const char * buffer, uint& start_index, float& dest, bool skipToNextLine)
{
    str.clear();

    // skip spaces
    while(buffer[start_index] == ' ')
        ++start_index;

    // read the data
    while(buffer[start_index] != ' ' && buffer[start_index] != '\r' && buffer[start_index] != '\n' && buffer[start_index] != EOF)
    {
        str += buffer[start_index];
        ++start_index;
    }

    // automatically skip to next line if allowed
    if(skipToNextLine)
    {
        while(buffer[start_index] != '\n' && buffer[start_index] != EOF)
            ++start_index;
    }
    dest = static_cast<float>( atof(str.c_str()) );
}

void OBJMeshIO::ReadThreeFloat(string& str, const char * buffer, uint& start_index, float4& dest)
{
    ReadOneFloat(str, buffer, start_index, dest.x, false);
    ReadOneFloat(str, buffer, start_index, dest.y, false);
    ReadOneFloat(str, buffer, start_index, dest.z);
}

void OBJMeshIO::GetTextureFileName(wstring& fullpath, const char* buffer, uint& start_index)
{
    bool full_dir = false;
    // skip all spaces
    while(buffer[start_index] == ' ' || buffer[start_index] == '\r' || buffer[start_index] == '\n')// OBJHelper::SkipAllSpaces();
        ++start_index;

    // get the data
    while(buffer[start_index] != EOF && buffer[start_index] != '\n' && buffer[start_index] != '\r')
    {
        char ch     = buffer[start_index];
        ++start_index;
        fullpath    += ch;

        if(ch == '\\')          // Track if the file read from the .mtl file has a full path
            full_dir = true;        
    }
    uint debug = 0;

    // NEED TO SAVE THE FILE'S MAIN DIRECTORY IN THE OBJMeshIO header file
    if(!full_dir)
    {
        const wstring temp(fullpath);
        fullpath = m_InputFileDirectory;
        fullpath += temp;
    }
}

void OBJMeshIO::ReadMaterial(const char* buffer, std::size_t& start_index)
{
    std::string str;

    // skip the usemtl
    MoveToNextSpace(buffer, start_index);
    ++start_index; // skip the space

    // Reserve enough space for all the characters to avoid multiple memory allocation calls
    const size_t char_count = CountValidCharacters(buffer, start_index);
    str.reserve(char_count);

    // While it is not the end of the line keep reading characters
    while(buffer[start_index] != '\n')
    {
        // Read only valid characters
        if( (buffer[start_index] != ' ')    &&
            (buffer[start_index] != '\r')   &&
            (buffer[start_index] != '\n') 
            )
        {
            str += buffer[start_index];
        }

        // increase the index
        ++start_index;
    }

    // Check if the new material needs to be added, or if it alerady exists then set it as the current material
    auto found = find(m_PerFaceMaterialList.begin(), m_PerFaceMaterialList.end(), str);

    // If it wasn't found then add it
    if(found != m_PerFaceMaterialList.end())
    {
        m_currentPerFaceMaterial = (*found);
    }
    else
    {
        // Create a new per-face material
        PERFACEMATERIAL* pMAT = new PERFACEMATERIAL(str);
        //pMAT->mMaterialNumber = m_material_number++;

        // Add it to the list
        m_PerFaceMaterialList.push_front(pMAT);

        // Set it as the current material
        m_currentPerFaceMaterial = m_PerFaceMaterialList.front();
    }

}

void OBJMeshIO::ReadSmoothingGroupDescription(const char* buffer, std::size_t& start_index)
{
    string SmoothingGroupName;

    // skip the usemtl
    MoveToNextSpace(buffer, start_index);
    ++start_index; // skip the space

    // Reserve enough space for all the characters to avoid multiple memory allocation calls
    const size_t char_count = CountValidCharacters(buffer, start_index);
    SmoothingGroupName.reserve(char_count);

    // While it is not the end of the line keep reading characters
    while(buffer[start_index] != '\n')
    {
        // Read only valid characters
        if( (buffer[start_index] != ' ') &&
            (buffer[start_index] != '\r') &&
            (buffer[start_index] != '\n') )
        {
            SmoothingGroupName += buffer[start_index];
        }

        // increase the index
        ++start_index;
    }

    // Add the smoothing group into the material
    m_currentPerFaceMaterial->AddSmoothingGroup( SmoothingGroupName );
}