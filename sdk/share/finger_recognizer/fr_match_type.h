#ifndef _FR_MATCH_TYPES_H_
#define _FR_MATCH_TYPES_H_

#define MINUTIAE_MAX_NO 64

typedef struct {
  unsigned short X;
  unsigned short Y;
  unsigned short thita;
  unsigned short misc;
} minutiaMatchType;

typedef struct {
  unsigned short   width;
  unsigned short   height;
  unsigned short   hori_res;
  unsigned short   vert_res;
  unsigned short   minutiae_no;
  unsigned short   reserved;
  minutiaMatchType minutiae[MINUTIAE_MAX_NO];
} fingerMatchType;

#endif

