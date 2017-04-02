#ifndef _FR_MINUTIAE_H_
#define _FR_MINUTIAE_H_

#define INT8 char
#define UINT8 unsigned char
#define INT16 short
#define UINT16 unsigned short
#define INT32 long
#define UINT32 unsigned long

#define NEIGHBOR_RADIUS      60
#define NEIGHBOR_RING_2      ((NEIGHBOR_RADIUS + 20) * (NEIGHBOR_RADIUS + 20))
#define ENOUGH_NEIGHBOR_NUM  4

// MCC_D8_S4b
#define MCC_BINARY
#define MCC_WIDTH_CELL 8
#define MCC_SECTION 4
#define INFLUENCE_WIDTH_CELL 8
#define INFLUENCE_SECTION 4
#define BINARY_THRESHOLD 110

#ifdef MCC_BINARY
#define MCC_WIDTH_BYTE (MCC_WIDTH_CELL / 8)
#define MCC_SECTION_BYTE (MCC_WIDTH_CELL * MCC_WIDTH_BYTE)
#define MCC_TOTAL_BYTE (MCC_SECTION * MCC_SECTION_BYTE)
#endif

#define TEMP_MCC_WIDTH_CELL (INFLUENCE_WIDTH_CELL + MCC_WIDTH_CELL + INFLUENCE_WIDTH_CELL)

#define PIXEL_CELL_MUL (65536 * MCC_WIDTH_CELL / NEIGHBOR_RADIUS)
#define NEIGHBOR_RANGE_PIXEL ((MCC_WIDTH_CELL / 2 + INFLUENCE_WIDTH_CELL / 2) * NEIGHBOR_RADIUS)

#define ACTIVE_MCC_NUM 32

typedef struct {
  UINT8 strength;
  UINT8 weight;
  UINT16 X;
  UINT16 Y;
  UINT16 thita;
  UINT8 MCC[MCC_TOTAL_BYTE];
} MCCType;

typedef struct {
  UINT16 activeNum;
  MCCType MCCs[ACTIVE_MCC_NUM];
} fingerMCCType;

typedef struct {
  UINT16        fingerNum;
//  FingerMCCType fingers[MCCARRAY_NUM];
  fingerMCCType fingers[];
} fingerMCCArrayType;

typedef struct {
  UINT8 active[128];
  MCCType MCCs[128];
} tempFingerMCCType;

typedef struct {
  UINT16 X;
  UINT16 Y;
  UINT8  type;
  UINT8  thita;
  UINT8  quality; // msb is type
  UINT8  misc;
} minutiaType;

typedef struct {
  UINT16 width;
  UINT16 height;
  UINT16 misc;
  UINT16 minutiae_no;
  minutiaType minutiae[128];
} fingerType;

//int sort_function(const void *d1, const void *d2);
//void initInfluence(void);
//void createMCC(fingerType *finger, int minutiaIndex);
void createMCC(fingerType *finger, int minutiaIndex, tempFingerMCCType *fingerMCC);
//void createMCCs(fingerType *finger);
void createMCCs(fingerType *finger, fingerMCCType *fingerMCC);
//double getMCCSimilarity(fingerType *finger1, int index1, fingerType *finger2, int index2, int MCCLevel);
UINT32 getMCCSimi(MCCType *MCC1, MCCType *MCC2);
//double getFingerSimilarity(fingerType *finger1, fingerType *finger2, int MCCLevel);
UINT32 getFingerSimi(fingerMCCType *finger1, fingerMCCType *finger2);


#endif
