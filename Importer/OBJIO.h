#if defined(_WINDOWS) || defined(WINDOWS)
#pragma once
#endif

#ifndef OBJ_IMPORTER_H
#define OBJ_IMPORTER_H
//*****************************************************************************************************|
//The OBJMeshIO class is the main importer interface of how Directx 11 will get the Geometry we want		   |
//The constructor is where the main processing begins, and it's where all subsequent methods are called|
//*****************************************************************************************************|

#include <memory>
#include <vector>
#include <string>
#include "math_funcs.h"
#include "Debug_ALL.h"
#include "MeshIOBase.h"

//Namespace's uses|
using namespace omi;
using namespace std;
using std::string;

class OBJMeshIO : public MeshIOBase
{
    // Variables
private:
    uint m_vcount, m_vtcount, m_vncount;
    uint m_triangles;
    uint m_verticesBegin, m_TexturesBegin, m_ifacesbegin, m_normalsBegin;
    uint m_fileLength;

    char* m_filebuff;

public:
    OBJMeshIO(void);
    ~OBJMeshIO(void) { SAFE_DELETE_ARRAY(m_filebuff); }
    Mesh* VProcessfile(const wstring& inputFile, IView* pIView);
    
private:
    void ReadQuadFace(vector<triangulatedVertex>& _triangFaces, const int* _quadfaces, UINT& vector_index);
    void ReadTriangFace(vector<triangulatedVertex>& _triangFaces, const int* _quadfaces, UINT& _vectorIndex);
    
    //-------------------------------------------------------------------------------------|
    //Private:																			 |
    //Action: Reads all the Vertices position into m_vertices respective Position variable |
    //Post-Cond: All the Vertex Position values are inputed into m_vertices.				 |
    //-------------------------------------------------------------------------------------|
    bool ReadVertices();

    //-------------------------------------------------------------------------|
    //Private:																 |
    //Action: Reads all the Normals into m_vertices respective Normal variable |
    //Post-Cond: All the Vertex Normal values are inputed into m_vertices.	 |
    //-------------------------------------------------------------------------|
    bool ReadNormals();

    //---------------------------------------------------------------------------------------------|
    //Private:																					 |
    //Action: Reads all the texture-coordinates into m_vertices respective TEX-Coordinate variable |
    //Post-Cond: All the texture coordinates values are inputed into m_vertices.					 |
    //---------------------------------------------------------------------------------------------|
    bool ReadTextureCoord(void);

    //--------------------------------------------------------------------------|
    //Private:																    |
    //Action: Reads all the Indices' data into m_indices variable               |
    //Post-Cond: All the indices for Positions, Normals, and Texture coordinates|
    //		   are saved for further processing by RedoVertsAndFaces()		    |
    //--------------------------------------------------------------------------|
    bool ReadIndices(void);

    //--------------------------------------------------------------------------|
    //Private:																    |
    //Action: Counts the m_vertices, normals, and texture coordinates           |
    //Post-Cond: All the right index locations are for respective data's start  |
    //			 and also finds out if anything is missing					    |
    //--------------------------------------------------------------------------|
    bool CountVertsTextsNorms(void);

    //--------------------------------------------------------------------------------------------------|
    //Private:																						    |
    //Action: This function reads and triangulates the current polygon(face), and determines the index  |
    //		for this particular face at each of its vertex positions								    |
    //Post-Cond:																					    |
    //--------------------------------------------------------------------------------------------------|
    void ReadFace(const uint& _spaces, const uint* _pint);

    // Reads the .mtl file associated with the obj file
    void GetMaterialAndTextures(const wstring& fileDirectory, const wstring& filename);

    //
    void ReadOneFloat(string& str, const char * buffer, uint& start_index, float& dest, bool skiptonextline = true);
    void ReadThreeFloat(string& str, const char * buffer, uint& start_index, float4& dest);

    void GetTextureFileName(wstring& fullpath, const char* buffer, uint& start_index);

    void ReadMaterial(const char* buffer, size_t& start_index);
    void ReadSmoothingGroupDescription(const char* buffer, std::size_t& start_index);
    
    void ReadMaterial(std::size_t& start_index) { ReadMaterial(m_filebuff, start_index); }
    void ReadSmoothingGroupDescription(std::size_t& start_index) { ReadSmoothingGroupDescription(m_filebuff, start_index); }
};

#endif