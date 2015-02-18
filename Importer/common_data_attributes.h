#if defined(_WINDOWS) || defined (WINDOWS)
#pragma once
#endif

#ifndef COMMON_ATTRIBUTES_H
#define COMMON_ATTRIBUTES_H

// textures
enum TEXTURE_TYPE{ UNDEFINED, DIFFUSE_MAP = 0xFDE8ACB9, NORMAL_MAP_TANGENT, HEIGHT_MAP, LIGHT_MAP, SHADOW_MAP, SPECULAR_MAP, AMBIENT_MAP};

// end of file markers, to be used consecutively to be accurate
enum END_OF_FILE {EOF_1 = 0x7F7FFFFE, EOF_2, EOF_3};

// end of mesh marker for files that have more than one mesh
enum END_OF_MESH {EOM_1 = 0xE05FFFFF, EOM_2};

// Left/Righ handed
enum HANDEDNESS {DIRECTX = 0X10101010, OPENGL};

enum FILE_FORMAT { OBJ = 0x2013DEAD, OBJE, HND, FBX, FORMAT_UNKNOWN };

// minimizing the space required by indices
enum INDEX_FORMAT { UNKNOWN, SHORT_TYPE = 0XDEAF0111, INT_TYPE };

// Per-vertex data stride if
enum DATA_FORMAT
{
    UNFORMATTED,
    POS3_NORM3 = 0xFADEDABD, 
    POS3_TEX2,                  // For meshes with light MAPS
    POS3_TEX2_NORM3,            // this is the most common one
    POS3_COLOR3_NORM3,
    POS3_TEX2_NORM3_BITAN4,     // BITANGENT WILL ALWAYS ENCODE THE REFLECTION FACTOR IN W-COORD
};

#endif