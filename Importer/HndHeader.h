#pragma once
#include "MeshIOBase.h"

#define EOF_1 0xDEADBBFA
#define EOF_2 0xDEADBBFB
#define EOF_3 0xDEADBBFC


// 
enum HND_TOKENS { VERTICES = 0xDEADBBFA, INDICES, MATERIAL, TEXTURE, END_OF_FILE};

struct HND_HEADER
{
    FILE_FORMAT mMagicNumber;   // 4
    uint mMeshCount;            // 8
    uint mVertsCount;           // 12
    DATA_FORMAT mformat;        // 16
    uint mIndexCount;           // 20
    INDEX_FORMAT mIndexFormat;  // 24
    HANDEDNESS mHandedNess;     // 28
    uint ENDIANNESS;            // 32
};

