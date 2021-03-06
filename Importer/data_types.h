#pragma once
// HND File data format
enum DATA_FORMAT
{
  POS3_NORM3 = 0xFADED,
  POS3_TEX2,
  POS3_NORM3_TEX2,
  POS3_NORM3_COLOR3,
  POS3_NORM3_TEX2_BITANG4, // BITANGENT WILL ALWAYS ENCODE THE REFLECTION FACTOR IN W-COORD  
};