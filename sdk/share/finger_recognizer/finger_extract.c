#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <malloc.h>

#include "finger_extract.h"
//#include "fr_type.h"

//#define DEBUG

//#define DIR16
#define DIR32

#define ABS(x) (((x) < 0) ? -(x) : (x))
#define MAX2(a,b) ((a>b) ? (a) : (b))
#define MIN2(a,b) ((a>b) ? (b) : (a))
#define MIN3(a,b,c) ((MIN2(a,b)>c) ? c : MIN2(a,b))
#define MAX3(a,b,c) ((a>MAX2(b,c)) ? a : MAX2(b,c))
//#define VAR_THRESHOLD		6
#define VAR_THRESHOLD		8
#define SPIKE_THRESHOLD			12
#define BREAK_THRESHOLD			14*14
#define LADDER_THRESHOLD			12

#define SWAP(a,b) do {\
unsigned char tmp=a; \
			a=b; \
			b=tmp;} while(0)\

#define SORT3(a,b,c) do {\
	if(a>b) SWAP(a,b); \
	if(a>c) SWAP(a,c); \
	if(b>c) SWAP(b,c); } while(0)\

static unsigned char dir_pattern0[8][64] = {
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
},
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
},
{ 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff
},
{ 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00
},
{ 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00
},
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00
},
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
},
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff,
  0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00,
  0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
},
};

#if defined(DIR16)
static unsigned char dir_pattern1[5][64] = {
{ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
},
{ 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
},
{ 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff,
},
{ 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00,
},
{ 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
}
};
#endif
#if defined(DIR32)
static unsigned char dir_pattern1[9][64] = {
  { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,  // 0
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  },
  { 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,  // 1
    0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  },
  { 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 2
    0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  },
  { 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 3
    0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  },
  { 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 4
    0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff,
  },
  { 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 5
    0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00,
  },
  { 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 6
    0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00,
  },
  { 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 7
    0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  },
  { 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 8
    0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  }
};
#endif

/*
static int dir_mask[8][4][2] = {
	{{-1,0},{0,0},{1,0},{2,0}},
	{{-1,0},{0,0},{1,1},{2,1}},
	{{-1,-1},{0,0},{1,1},{2,2}},
	{{0,-1},{0,0},{1,1},{1,2}},
	{{0,-1},{0,0},{0,1},{0,2}},
	{{0,-1},{0,0},{-1,1},{-1,2}},
	{{1,-1},{0,0},{-1,1},{-2,2}},
	{{-1,0},{0,0},{1,-1},{2,-1}},
};
*/

static int dir_mask[8][8][2] = {
	{ {-3,0}, {-2,0}, {-1,0},{0,0}, {1,0}, {2,0}, {3,0}, {4,0}},
	{{-3,-1},{-2,-1}, {-1,0},{0,0}, {1,1}, {2,1}, {3,2}, {4,2}},
	{{-3,-3},{-2,-2},{-1,-1},{0,0}, {1,1}, {2,2}, {3,3}, {4,4}},
	{{-1,-3},{-1,-2}, {0,-1},{0,0}, {1,1}, {1,2}, {2,3}, {2,4}},
	{ {0,-3}, {0,-2}, {0,-1},{0,0}, {0,1}, {0,2}, {0,3}, {0,4}},
	{ {1,-3}, {1,-2}, {0,-1},{0,0},{-1,1},{-1,2},{-2,3},{-2,4}},
	{ {3,-3}, {2,-2}, {1,-1},{0,0},{-1,1},{-2,2},{-3,3},{-4,4}},
	{ {-3,1}, {-2,1}, {-1,0},{0,0},{1,-1},{2,-1},{3,-2},{4,-2}},
};

static int dir_lookup[5][5] = {
	{3,3,4,6,6},
	{3,3,4,5,5},
	{2,2,7,6,6},
	{1,1,0,7,6},
	{1,1,0,0,7},
};

#if defined(DIR16)
// has bug, updated 20160825
/*
static char dir_middle[16][16] = {
	{ 0, 1, 1, 2, 2, 3, 3, 4,12,13,13,14,14,15,15, 0}, //(0,X)
	{ 1, 1, 2, 2, 3, 3, 4, 4, 5,13,14,14,15,15, 0, 0}, //(1,X)
	{ 1, 2, 2, 3, 3, 4, 4, 5, 5, 6,14,15,15, 0, 0, 1}, //(2,X)
	{ 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,15, 0, 0, 1, 1}, //(3,X)
	{ 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 0, 1, 1, 2}, //(4,X)
	{ 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 1, 2, 2}, //(5,X)
	{ 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9,10, 2, 3}, //(6,X)
	{ 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9,10,10,11, 3}, //(7,X)
	{12, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9,10,10,11,11,12}, //(8,X)
	{13, 5, 6, 6, 7, 7, 8, 8, 9, 9,10,10,11,11,12,12}, //(9,X)
	{13,14, 6, 7, 7, 8, 8, 9, 9,10,10,11,11,12,12,13}, //(10,X)
	{14,14,15, 7, 8, 8, 9, 9,10,10,11,11,12,12,13,13}, //(11,X)
	{14,14,15,15, 8, 9, 9,10,10,11,11,12,12,13,13,14}, //(12,X)
	{15,15, 0, 0, 1, 9,10,10,11,11,12,12,13,13,14,14}, //(13,X)
	{15, 0, 0, 1, 1, 2,10,11,11,12,12,13,13,14,14,15}, //(14,X)
	{ 0, 0, 1, 1, 2, 2, 3,11,12,12,13,13,14,14,15,15}, //(15,X)
};
*/
static char dir_middle[16][16] = {
  {  0,  1,  1,  2,  2,  3,  3,  4, 12, 13, 13, 14, 14, 15, 15,  0},
  {  1,  1,  2,  2,  3,  3,  4,  4,  5, 13, 14, 14, 15, 15,  0,  0},
  {  1,  2,  2,  3,  3,  4,  4,  5,  5,  6, 14, 15, 15,  0,  0,  1},
  {  2,  2,  3,  3,  4,  4,  5,  5,  6,  6,  7, 15,  0,  0,  1,  1},
  {  2,  3,  3,  4,  4,  5,  5,  6,  6,  7,  7,  8,  0,  1,  1,  2},
  {  3,  3,  4,  4,  5,  5,  6,  6,  7,  7,  8,  8,  9,  1,  2,  2},
  {  3,  4,  4,  5,  5,  6,  6,  7,  7,  8,  8,  9,  9, 10,  2,  3},
  {  4,  4,  5,  5,  6,  6,  7,  7,  8,  8,  9,  9, 10, 10, 11,  3},
  { 12,  5,  5,  6,  6,  7,  7,  8,  8,  9,  9, 10, 10, 11, 11, 12},
  { 13, 13,  6,  6,  7,  7,  8,  8,  9,  9, 10, 10, 11, 11, 12, 12},
  { 13, 14, 14,  7,  7,  8,  8,  9,  9, 10, 10, 11, 11, 12, 12, 13},
  { 14, 14, 15, 15,  8,  8,  9,  9, 10, 10, 11, 11, 12, 12, 13, 13},
  { 14, 15, 15,  0,  0,  9,  9, 10, 10, 11, 11, 12, 12, 13, 13, 14},
  { 15, 15,  0,  0,  1,  1, 10, 10, 11, 11, 12, 12, 13, 13, 14, 14},
  { 15,  0,  0,  1,  1,  2,  2, 11, 11, 12, 12, 13, 13, 14, 14, 15},
  {  0,  0,  1,  1,  2,  2,  3,  3, 12, 12, 13, 13, 14, 14, 15, 15}
};


static char dir_diff[16][16] = {
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 7, 6, 5, 4, 3, 2, 1},
	{ 1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 7, 6, 5, 4, 3, 2},
	{ 2, 1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 7, 6, 5, 4, 3},
	{ 3, 2, 1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 7, 6, 5, 4},
	{ 4, 3, 2, 1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 7, 6, 5},
	{ 5, 4, 3, 2, 1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 7, 6},
	{ 6, 5, 4, 3, 2, 1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 7},
	{ 7, 6, 5, 4, 3, 2, 1, 0, 1, 2, 3, 4, 5, 6, 7, 8},
	{ 8, 7, 6, 5, 4, 3, 2, 1, 0, 1, 2, 3, 4, 5, 6, 7},
	{ 7, 8, 7, 6, 5, 4, 3, 2, 1, 0, 1, 2, 3, 4, 5, 6},
	{ 6, 7, 8, 7, 6, 5, 4, 3, 2, 1, 0, 1, 2, 3, 4, 5},
	{ 5, 6, 7, 8, 7, 6, 5, 4, 3, 2, 1, 0, 1, 2, 3, 4},
	{ 4, 5, 6, 7, 8, 7, 6, 5, 4, 3, 2, 1, 0, 1, 2, 3},
	{ 3, 4, 5, 6, 7, 8, 7, 6, 5, 4, 3, 2, 1, 0, 1, 2},
	{ 2, 3, 4, 5, 6, 7, 8, 7, 6, 5, 4, 3, 2, 1, 0, 1},
	{ 1, 2, 3, 4, 5, 6, 7, 8, 7, 6, 5, 4, 3, 2, 1, 0}
};
#endif

#if defined(DIR32)
static char dir_middle[32][32] = {
  { 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 24, 25, 25, 26, 26, 27, 27, 28, 28, 29, 29, 30, 30, 31, 31, 0},
  { 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 25, 26, 26, 27, 27, 28, 28, 29, 29, 30, 30, 31, 31, 0, 0},
  { 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 26, 27, 27, 28, 28, 29, 29, 30, 30, 31, 31, 0, 0, 1},
  { 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 27, 28, 28, 29, 29, 30, 30, 31, 31, 0, 0, 1, 1},
  { 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 28, 29, 29, 30, 30, 31, 31, 0, 0, 1, 1, 2},
  { 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 29, 30, 30, 31, 31, 0, 0, 1, 1, 2, 2},
  { 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13, 14, 30, 31, 31, 0, 0, 1, 1, 2, 2, 3},
  { 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15, 31, 0, 0, 1, 1, 2, 2, 3, 3},
  { 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15, 16, 0, 1, 1, 2, 2, 3, 3, 4},
  { 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15, 16, 16, 17, 1, 2, 2, 3, 3, 4, 4},
  { 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15, 16, 16, 17, 17, 18, 2, 3, 3, 4, 4, 5},
  { 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15, 16, 16, 17, 17, 18, 18, 19, 3, 4, 4, 5, 5},
  { 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15, 16, 16, 17, 17, 18, 18, 19, 19, 20, 4, 5, 5, 6},
  { 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15, 16, 16, 17, 17, 18, 18, 19, 19, 20, 20, 21, 5, 6, 6},
  { 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15, 16, 16, 17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 6, 7},
  { 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15, 16, 16, 17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 7},
  { 24, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15, 16, 16, 17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 23, 24},
  { 25, 25, 10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15, 16, 16, 17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 23, 24, 24},
  { 25, 26, 26, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15, 16, 16, 17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 23, 24, 24, 25},
  { 26, 26, 27, 27, 12, 12, 13, 13, 14, 14, 15, 15, 16, 16, 17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 23, 24, 24, 25, 25},
  { 26, 27, 27, 28, 28, 13, 13, 14, 14, 15, 15, 16, 16, 17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 23, 24, 24, 25, 25, 26},
  { 27, 27, 28, 28, 29, 29, 14, 14, 15, 15, 16, 16, 17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 23, 24, 24, 25, 25, 26, 26},
  { 27, 28, 28, 29, 29, 30, 30, 15, 15, 16, 16, 17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 23, 24, 24, 25, 25, 26, 26, 27},
  { 28, 28, 29, 29, 30, 30, 31, 31, 16, 16, 17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 23, 24, 24, 25, 25, 26, 26, 27, 27},
  { 28, 29, 29, 30, 30, 31, 31, 0, 0, 17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 23, 24, 24, 25, 25, 26, 26, 27, 27, 28},
  { 29, 29, 30, 30, 31, 31, 0, 0, 1, 1, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 23, 24, 24, 25, 25, 26, 26, 27, 27, 28, 28},
  { 29, 30, 30, 31, 31, 0, 0, 1, 1, 2, 2, 19, 19, 20, 20, 21, 21, 22, 22, 23, 23, 24, 24, 25, 25, 26, 26, 27, 27, 28, 28, 29},
  { 30, 30, 31, 31, 0, 0, 1, 1, 2, 2, 3, 3, 20, 20, 21, 21, 22, 22, 23, 23, 24, 24, 25, 25, 26, 26, 27, 27, 28, 28, 29, 29},
  { 30, 31, 31, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 21, 21, 22, 22, 23, 23, 24, 24, 25, 25, 26, 26, 27, 27, 28, 28, 29, 29, 30},
  { 31, 31, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 22, 22, 23, 23, 24, 24, 25, 25, 26, 26, 27, 27, 28, 28, 29, 29, 30, 30},
  { 31, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 23, 23, 24, 24, 25, 25, 26, 26, 27, 27, 28, 28, 29, 29, 30, 30, 31},
  { 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 24, 24, 25, 25, 26, 26, 27, 27, 28, 28, 29, 29, 30, 30, 31, 31}
};

static char dir_diff[32][32] = {
  { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1},
  { 1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2},
  { 2, 1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3},
  { 3, 2, 1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4},
  { 4, 3, 2, 1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5},
  { 5, 4, 3, 2, 1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6},
  { 6, 5, 4, 3, 2, 1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7},
  { 7, 6, 5, 4, 3, 2, 1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 15, 14, 13, 12, 11, 10, 9, 8},
  { 8, 7, 6, 5, 4, 3, 2, 1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 15, 14, 13, 12, 11, 10, 9},
  { 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 15, 14, 13, 12, 11, 10},
  { 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 15, 14, 13, 12, 11},
  { 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 15, 14, 13, 12},
  { 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 15, 14, 13},
  { 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 15, 14},
  { 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 15},
  { 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16},
  { 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
  { 15, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14},
  { 14, 15, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13},
  { 13, 14, 15, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12},
  { 12, 13, 14, 15, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11},
  { 11, 12, 13, 14, 15, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10},
  { 10, 11, 12, 13, 14, 15, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9},
  { 9, 10, 11, 12, 13, 14, 15, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 1, 2, 3, 4, 5, 6, 7, 8},
  { 8, 9, 10, 11, 12, 13, 14, 15, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 1, 2, 3, 4, 5, 6, 7},
  { 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 1, 2, 3, 4, 5, 6},
  { 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 1, 2, 3, 4, 5},
  { 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 1, 2, 3, 4},
  { 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 1, 2, 3},
  { 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 1, 2},
  { 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 1},
  { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0}
};
#endif

minutiae_list mlist;
minutiae minutiaes[MINUTIAE_MAX] = {-1};

/*unsigned char *image0, *image1, *image2;
unsigned char *block0, *block1, *block2, *block3, *block4;
int *integralValue;*/


static int blurWeight[3][3] = {
  {1, 2, 1},
  {2, 4, 2},
  {1, 2, 1}
};

int imageBlur(unsigned char *dst, unsigned char *src, int width, int height) {
  int h, v, hh, vv, hhh, vvv, value;

  for (v = 0; v < height; v++)
  	for (h = 0; h < width; h++) {
	  value = 0;
	  for (vv = 0; vv < 3; vv++) {
	  	vvv = v + vv - 1;
		if (vvv < 0) vv = 0;
		else if (vvv >= height) vv = height - 1;
	  	for (hh = 0; hh < 3; hh++) {
		  hhh = h + hh - 1;
		  if (hhh < 0) hh = 0;
		  else if (hhh >= width) hh = width - 1;
		  value += *(src + vvv * width + hhh) * blurWeight[vv][hh];
	  	}
	  }
	  *(dst + v * width + h) = (value + 8) >> 4;
  	}
}

int intBlur(int *dst, int *src, int width, int height) {
  int h, v, hh, vv, hhh, vvv, value;

  for (v = 0; v < height; v++)
    for (h = 0; h < width; h++) {
      value = 0;
      for (vv = 0; vv < 3; vv++) {
        vvv = v + vv - 1;
        if (vvv < 0) vv = 0;
        else if (vvv >= height) vv = height - 1;
        for (hh = 0; hh < 3; hh++) {
          hhh = h + hh - 1;
          if (hhh < 0) hh = 0;
          else if (hhh >= width) hh = width - 1;
          value += *(src + vvv * width + hhh) * blurWeight[vv][hh];
        }
      }
      *(dst + v * width + h) = (value + 8) >> 4;
    }

  return 0;
}

int imageBilinearScale(unsigned char *dst, unsigned char *src, int dst_width, int dst_height, int src_width, int src_height) {
  int h, v, top_left, top_right, bottom_left, bottom_right;
  double h_step, v_step, h_center, v_center, left, top, right, bottom, value;

  h_step = src_width * 1.0 / dst_width;
  v_step = src_height * 1.0 / dst_height;
  for (v = 0; v < dst_height; v++) {
    v_center = v_step / 2 + v_step * v;
    top = floor(v_center + 0.5) - 0.5;
    bottom = top + 1.0;
    if (top < 0.5) top = 0.5;
    if (bottom > src_height - 0.5) bottom = src_height - 0.5;
    for (h = 0; h < dst_width; h++) {
      h_center = h_step / 2 + h_step * h;
      left = floor(h_center + 0.5) - 0.5;
      right = left + 1.0;
      if (left < 0.5) left = 0.5;
      if (right > src_width - 0.5) right = src_width - 0.5;
      // do bilinear
      top_left     = *(src + (int)top    * src_width + (int)left );
      top_right    = *(src + (int)top    * src_width + (int)right);
      bottom_left  = *(src + (int)bottom * src_width + (int)left );
      bottom_right = *(src + (int)bottom * src_width + (int)right);
      value = (top_left    * (right - h_center) + top_right    * (h_center - left)) * (bottom - v_center)
            + (bottom_left * (right - h_center) + bottom_right * (h_center - left)) * (v_center - top);
      *(dst + v * dst_width + h) = (int)(value + 0.5);
    }
  }
  return 0;
}

#if 0
void createIntegralValue(int width, int height, unsigned char *image, int *integral) {
  int h, v;

  integral[0] = image[0];
  for (h = 1; h < width; h++)
    integral[h] = integral[h - 1] + image[h];
  for (v = 1; v < height; v++) {
    integral[v * width] = integral[(v - 1) * width] + image[v * width];
    for (h = 1; h < width; h++)
      integral[v * width + h] = image[v * width + h] + integral[v * width + h - 1]
                                + integral[(v - 1) * width + h] - integral[(v - 1) * width + h - 1];
  }
}
#else
void createIntegralValue(int width, int height, unsigned char *image, int *integral) {
  int h, v, *pc, *ptr, tl, bl;
  unsigned char *pi;

  integral[0] = image[0];
  for (h = 1; h < width; h++)
    integral[h] = integral[h - 1] + image[h];
  for (v = 1; v < height; v++) {
    pi = image + v * width;
    pc = integral + v * width;
    ptr = pc - width;
    bl = *pc = *pi + (tl = *ptr);
    for (h = 1; h < width; h++) {
      pi++;
      pc++;
      ptr++;
      *pc = *pi + bl + *ptr - tl;
      tl = *ptr;
      bl = *pc;
    }
  }
}
#endif


#if 0
int getIntegralValue(int width, int height, int *integralValue, int region_left, int region_top, int region_width, int region_height, int *count) {
  int left_outbound, top_outbound, tl, tr, bl, br, *integralValueS32, area;

  left_outbound = top_outbound = 0;
  region_left--;
  if (region_left < 0) { region_left = 0; left_outbound = 1; }
  region_top--;
  if (region_top < 0) { region_top = 0; top_outbound = 1; }
  if (region_left + region_width >= width) region_width = width - region_left - 1;
  if (region_top + region_height > height) region_height = height - region_top - 1;
  *count = region_width * region_height;

  if (left_outbound) {
    if (top_outbound) { // left && top
      tl = tr = bl = 0;
    } else { // left && !top
      tl = bl = 0;
      tr = integralValue[region_top * width + region_left + region_width];
    }
  } else {
    if (top_outbound) { // !left && top
      tl = tr = 0;
    } else { // !left && !top
      tl = integralValue[region_top * width + region_left];
      tr = integralValue[region_top * width + region_left + region_width];
    }
    bl = integralValue[(region_top + region_height) * width + region_left];
  }
  br = integralValue[(region_top + region_height) * width + region_left + region_width];
  return br - bl - tr + tl;
}
#else
int getIntegralValue(int width, int height, int *integralValue, int region_left, int region_top, int region_width, int region_height) {
  int tl, tr, bl, br;

  // assert 0 <= region_left   < width
  // assert 0 <= region_top    < height
  // assert 0 <= region_right  < width
  // assert 0 <= region_bottom < height

  tl = integralValue[region_top * width + region_left];
  tr = integralValue[region_top * width + region_left + region_width];
  bl = integralValue[(region_top + region_height) * width + region_left];
  br = integralValue[(region_top + region_height) * width + region_left + region_width];

  return br - bl - tr + tl;
}
#endif

// [SmallY20160601] local shading correction
#define RADIUS_SHIFT 2
#define LOCAL_RADIUS (1 << RADIUS_SHIFT)
#define LOCAL_COUNT ((LOCAL_RADIUS * 2) * (LOCAL_RADIUS * 2))
#define COUNT_SHIFT ((RADIUS_SHIFT + 1) * 2)
#if 0
int localShadingCorrection(unsigned char *image, int width, int height, int *integralValue) {
  int h, v, data, count, local;

  createIntegralValue(width, height, image, integralValue);
  for (v = LOCAL_RADIUS; v < height - LOCAL_RADIUS; v++)
    for (h = LOCAL_RADIUS; h < width - LOCAL_RADIUS; h++) {
      local = getIntegralValue(width, height, integralValue, h - LOCAL_RADIUS, v - LOCAL_RADIUS, LOCAL_RADIUS * 2, LOCAL_RADIUS * 2);
      local = (local + LOCAL_COUNT / 2) >> COUNT_SHIFT;
      data = image[v * width + h];
      data = data - (local - 128);
      image[v * width + h] = data;
    }
}
#else
int localShadingCorrection(unsigned char *image, int width, int height, int *integralValue) {
  int h, v, *ptl, *ptr, *pbl, *pbr, local, data;

  createIntegralValue(width, height, image, integralValue);
  for (v = LOCAL_RADIUS; v < height - LOCAL_RADIUS; v++) {
    ptl = integralValue + (v - LOCAL_RADIUS) * width + LOCAL_RADIUS - LOCAL_RADIUS;
    ptr = ptl + LOCAL_RADIUS * 2;
    pbl = ptl + LOCAL_RADIUS * 2 * width;
    pbr = pbl + LOCAL_RADIUS * 2;
    for (h = LOCAL_RADIUS; h < width - LOCAL_RADIUS; h++) {
      local = *pbr - *pbl - *ptr + *ptl;
      local = (local + LOCAL_COUNT / 2) >> COUNT_SHIFT;
      data = image[v * width + h];
      data = data - (local - 128);
      image[v * width + h] = data;
      ptl++;
      ptr++;
      pbl++;
      pbr++;
    }
  }
}
#endif

#define STEPSHIFT 10
#define ONESTEP (1 << STEPSHIFT)
#define HALFSTEP (ONESTEP >> 1)
void imageBilinearScalePasteY(unsigned char *dst, int dst_width, int dst_height, int dst_region_left, int dst_region_top, int dst_region_right, int dst_region_bottom,
    unsigned char *src, int src_width, int src_height, int src_region_left, int src_region_top, int src_region_right, int src_region_bottom) {
  int h, v, dst_region_width, dst_region_height, src_region_width, src_region_height, i1;
  int top_left, top_right, bottom_left, bottom_right;
  int h_step, v_step, h_center, v_center, left, top, right, bottom, value;

  dst_region_width = dst_region_right - dst_region_left;
  dst_region_height = dst_region_bottom - dst_region_top;
  src_region_width = src_region_right - src_region_left;
  src_region_height = src_region_bottom - src_region_top;
  h_step = (src_region_width * ONESTEP + dst_region_width / 2) / dst_region_width;
  v_step = (src_region_height * ONESTEP + dst_region_height / 2) / dst_region_height;
  for (v = dst_region_top; v < dst_region_bottom; v++) {
    v_center = v_step / 2 + v_step * v + (src_region_top << STEPSHIFT);
    top = (((v_center + HALFSTEP) >> STEPSHIFT) << STEPSHIFT) - HALFSTEP;
    bottom = top + ONESTEP;
    if (top < HALFSTEP) top = HALFSTEP;
    if (bottom > (src_height << STEPSHIFT) - HALFSTEP) bottom = (src_height << STEPSHIFT) - HALFSTEP;
    for (h = dst_region_left; h < dst_region_right; h++) {
      h_center = h_step / 2 + h_step * h + (src_region_left << STEPSHIFT);
      left = (((h_center + HALFSTEP) >> STEPSHIFT) << STEPSHIFT) - HALFSTEP;
      right = left + ONESTEP;
      if (left < HALFSTEP) left = HALFSTEP;
      if (right > (src_width << STEPSHIFT) - HALFSTEP) right = (src_width << STEPSHIFT) - HALFSTEP;
      // do bilinear
      top_left     = *(src + (top    >> STEPSHIFT) * src_width + (left  >> STEPSHIFT));
      top_right    = *(src + (top    >> STEPSHIFT) * src_width + (right >> STEPSHIFT));
      bottom_left  = *(src + (bottom >> STEPSHIFT) * src_width + (left  >> STEPSHIFT));
      bottom_right = *(src + (bottom >> STEPSHIFT) * src_width + (right >> STEPSHIFT));
      value = (top_left    * (right - h_center) + top_right    * (h_center - left)) * (bottom - v_center)
            + (bottom_left * (right - h_center) + bottom_right * (h_center - left)) * (v_center - top);
      *(dst + v * dst_width + h) = (value + (1 << (STEPSHIFT * 2 - 1))) >> (STEPSHIFT * 2);
    }
  }
}

// [SmallY20160527] global normalize
// only need to apply for GT-5110E1 320x240 sensor after shading correction
void globalNormalize(unsigned char *image, int width, int height) {
#if 0
#define HIST_TH (0.1 / 256)
  int i1, h, v, data, hist[256], min, max;
  int checkValue = HIST_TH * width * height;

  for (i1 = 0; i1 < 256; i1++) hist[i1] = 0;
  for (v = 0; v < height; v++)
    for (h = 0; h < width; h++)
      hist[image[v * width + h]]++;
  for (i1 = 0; i1 < 256; i1++)
    if (hist[i1] > HIST_TH * width * height) {
      min = i1;
      break;
    }
  for (i1 = 255; i1 >= 0; i1--)
    if (hist[i1] > HIST_TH * width * height) {
      max = i1;
      break;
    }
  for (v = 0; v < height; v++)
    for (h = 0; h < width; h++) {
      data = image[v * width + h];
      if (data >= max) data = 255;
      else if (data <= min) data = 0;
      else data = (int)(255.0 * (data - min) / (max - min) + 0.5);
      image[v * width + h] = data;
    }
#else
#define HIST_TH (int)(4.0 * 65536 / 256)
    int i1, h, v, data, hist[256] = { 0 }, min, max, normFactor, checkBound = HIST_TH * width * height;

    //for (i1 = 0; i1 < 256; i1++) hist[i1] = 0;
    for (v = 0; v < height; v++)
      for (h = 0; h < width; h++)
        hist[image[v * width + h]]++;
    for (i1 = 0; i1 < 256; i1++)
      if ((hist[i1] << 16) > checkBound) {
        min = i1;
        break;
      }
    for (i1 = 255; i1 >= 0; i1--)
      if ((hist[i1] << 16) > checkBound) {
        max = i1;
        break;
      }
    normFactor = (255 * 65536 + (max - min) / 2) / (max - min);
    for (v = 0; v < height; v++)
      for (h = 0; h < width; h++) {
        data = image[v * width + h];
        if (data >= max) data = 255;
        else if (data <= min) data = 0;
        else data = ((data - min) * normFactor + 32768) >> 16;
        image[v * width + h] = data;
      }
#endif
}

unsigned char median(unsigned char *line0, unsigned char *line1, unsigned char *line2) {
  unsigned char r0, r1, r2, r3, r4, r5, r6, r7, r8;
  unsigned char tmp3, tmp6;

  r0 = line0[0];  r1 = line0[1];  r2 = line0[2];
  r3 = line1[0];  r4 = line1[1];  r5 = line1[2];
  r6 = line2[0];  r7 = line2[1];  r8 = line2[2];

#if 0
  r0 = 0;  r1 = 1;  r2 = 2;
  r3 = 3;  r4 = 4;  r5 = 5;
  r6 = 6;  r7 = 7;  r8 = 8;
#endif

  SORT3(r0, r1, r2);
  SORT3(r3, r4, r5);
  SORT3(r6, r7, r8);
  tmp3 = MAX3(r0, r3, r6);
  tmp6 = MIN3(r2, r5, r8);
#if 0
  SORT3(r1, r5, r7);
  SORT3(tmp3, r5, tmp6);
  return r5;
#else
  SORT3(r1,r4,r7);
  SORT3(tmp3,r4,tmp6);
  return r4;
#endif
}

int find_mean(int width, int height, unsigned char *din, unsigned char *mean) {
  int i, j, ii, jj, m, n;
  int val;

  for(i = 0; i < height - WIN8; i += WIN8) {
    for(j = 0; j < width - WIN8; j += WIN8) {
      m = (i / WIN8) * (width / WIN8) + j / WIN8;
      val = 0;
      for(ii = 0; ii < WIN8; ii++) {
        for(jj = 0; jj < WIN8; jj++) {
          n = (i + ii) * width + j + jj;
          val += din[n];
        }
      }
      mean[m] = (unsigned char)(val >> 6);
    }
  }
  return 0;
}

int find_variance(int width, int height, unsigned char *din, unsigned char *mean, unsigned char *variance) {
	int i,j,ii,jj, m, n, w, h;
	int var;

	w = width/WIN8;
	h = height/WIN8;

	for(i=0;i<height-WIN8;i+=WIN8) {
		for(j=0;j<width-WIN8;j+=WIN8) {
			m = (i/WIN8)*(width/WIN8)+j/WIN8;
			var = 0;
			for(ii=0;ii<WIN8;ii++) {
				for(jj=0;jj<WIN8;jj++) {
					n = (i+ii)*width+j+jj;
					var += ABS(din[n]-mean[m]);
				}
			}
			variance[m] = var>>6;
		}
	}
	return 0;
}

int find_border(int width, int height, unsigned char *din, 
				  unsigned char *mean, unsigned char *variance, unsigned char *border)
{
	int i,j,ii,jj, m, n, w, h;
	int var;
	unsigned char *tmp;

	w = width/WIN8;
	h = height/WIN8;

	// expand border
	for (j=0;j<w;j++) {
		border[j] = 1;
		border[(h-1)*w+j] = 1;
	}

	for(i=1;i<h-1;i++) {
		border[i*w] = 1;
		border[i*w + (w-1)] = 1;
		for(j=1;j<w-1;j++) {
			unsigned char p1 = (variance[i*w+j] < VAR_THRESHOLD) ? 1 : 0;
			unsigned char p2 = (variance[(i-1)*w+j] < VAR_THRESHOLD) ? 1 : 0;
			unsigned char p3 = (variance[(i-1)*w+j+1] < VAR_THRESHOLD) ? 1 : 0;
			unsigned char p4 = (variance[i*w+j+1] < VAR_THRESHOLD) ? 1 : 0;
			unsigned char p5 = (variance[(i+1)*w+j+1] < VAR_THRESHOLD) ? 1 : 0;
			unsigned char p6 = (variance[(i+1)*w+j] < VAR_THRESHOLD) ? 1 : 0;
			unsigned char p7 = (variance[(i+1)*w+j-1] < VAR_THRESHOLD) ? 1 : 0;
			unsigned char p8 = (variance[i*w+j-1] < VAR_THRESHOLD) ? 1 : 0;
			unsigned char p9 = (variance[(i-1)*w+j-1] < VAR_THRESHOLD) ? 1 : 0;
			int A = p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9;			
			if(p1) {
				if (A<3) p1 = 0;					
			} else {
				if (A>2) p1 = 1;				
			}
			border[i*w+j] = p1;
		}
	}
	return 0;
}

int clear_border(int width, int height, unsigned char *din, unsigned char *border, unsigned char *dout) {
  int i, j, m, n, ii, jj;

  for(i = 0; i < height - WIN8; i += WIN8) {
    for(j = 0; j < width - WIN8; j += WIN8) {
      m = (i / WIN8) * (width / WIN8) + j / WIN8;
      if (border[m]) {
        for(ii = 0; ii < WIN8; ii++) {
          for(jj=0; jj < WIN8; jj++) {
            n = (i + ii) * width + (j + jj);
            dout[n] = 0x0;
          }
        }
      }
    }
  }
  return 0;
}

int dilation_border(int width, int height, unsigned char *din, unsigned char *dout) {
  int i, j;

  for (j = 0; j < width; j++) {
    dout[j] = 1;
    dout[(height - 1) * width + j] = 1;
  }

  for(i = 1; i < height - 1; i++) {
    dout[i * width] = 1;
    dout[i * width + (width - 1)] = 1;
    for(j = 1; j < width - 1; j++) {
      unsigned char p1 = din[i * width + j];
      if(!p1) {
        unsigned char p2 = din[(i - 1) * width + j];
        unsigned char p3 = din[(i - 1) * width + j + 1];
        unsigned char p4 = din[i       * width + j + 1];
        unsigned char p5 = din[(i + 1) * width + j + 1];
        unsigned char p6 = din[(i + 1) * width + j];
        unsigned char p7 = din[(i + 1) * width + j - 1];
        unsigned char p8 = din[i       * width + j - 1];
        unsigned char p9 = din[(i - 1) * width + j - 1];
        int A = p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9;
        if (A > 0) dout[i * width + j] = 1;
        else dout[i * width + j] = 0;
      } else
        dout[i * width + j] = 1;
    }
  }
  return 0;
}

int expand_border(int width, int height, unsigned char *din, unsigned char *dout)
{
	int i,j,ii,jj, n, m;

	for(i=0;i<height-WIN8;i+=WIN8) {
		for(j=0;j<width-WIN8;j+=WIN8) {
			m = (i/WIN8)*(width/WIN8)+j/WIN8;
			for(ii=0;ii<WIN8;ii++) {
				for(jj=0;jj<WIN8;jj++) {
					n = (i+ii)*width + (j+jj);
					dout[n] = (din[m]) ? 0xff : 0;
				}
			}
		}
	}
	return 0;
}

#if 0
int median_filter(int width, int height, unsigned char *din, unsigned char *dout) {
  int i, j;

  memcpy(&dout[0], &din[0], width);
  memcpy(&dout[(height - 1) * width], &din[(height - 1) * width], width);

  for(i = 1; i < height - 1; i++) {
    dout[i * width] = din[i * width];
    for (j = 1; j < width - 1; j++) {
      dout[i * width+j] = median(&din[(i - 1) * width + j - 1], &din[i * width + j - 1], &din[(i + 1) * width + j - 1]);
    }
    dout[i * width + j] = din[i * width + j];
  }
  return 0;
}
#else
int median_filter(int width, int height, unsigned char *din, unsigned char *dout, int region_left, int region_top, int region_right, int region_bottom) {
  int i, j;
  unsigned char *pin, *pout, *pt, *pc, *pb;

  if (region_left < 1) region_left = 1;
  if (region_right > width - 1) region_right = width - 1;
  if (region_top < 1) region_top = 1;
  if (region_bottom > height - 1) region_bottom = height - 1;

  for(i = 0; i < height; i++) {
  	pin = din + i * width;
	pt = pin - width - 1;
	pc = pin - 1;
	pb = pin + width - 1;
	pout = dout + i * width;
    for (j = 0; j < width; j++) {
	  if (i < region_top || i >= region_bottom || j < region_left || j >= region_right)
	  	*pout = *pin;
	  else
        *pout = median(pt, pc, pb);
	  pin++;
	  pt++;
	  pc++;
	  pb++;
	  pout++;
    }
  }
  return 0;
}
#endif

int mean_filter(int width, int height, signed char *din, signed char *dout)
{
	int i,j;

	memcpy(&dout[0], &din[0], width);
	memcpy(&dout[(height-1)*width], &din[(height-1)*width], width);

	for(i=1;i<height-1;i++) {
		dout[i*width] = din[i*width];
		for(j=1;j<width-1;j++) {
			int p1 = din[i*width+j];
			int p2 = din[(i-1)*width+j];
			int p3 = din[(i-1)*width+j+1];
			int p4 = din[i*width+j+1];
			int p5 = din[(i+1)*width+j+1];
			int p6 = din[(i+1)*width+j];
			int p7 = din[(i+1)*width+j-1];
			int p8 = din[i*width+j-1];
			int p9 = din[(i-1)*width+j-1];
//			int A = (p1 + ((p2+p4+p6+p8)>>1) + ((p3+p5+p7+p9)>>2))>>2;
			int A = ((p1 << 2) + ((p2 + p4 + p6 + p8) << 1) + p3 + p5 + p7 + p9 + 8) >> 4;
			dout[i*width+j] = (signed char)A;
		}
		dout[i*width+j] = din[i*width+j];
	}
	return 0;
}

int gradient_filter(int width, int height, unsigned char *din, short *gx, short *gy) {
  int i, j;

  for(j = 0; j < width; j++) {
    gx[j] = gy[j] = 0;
    gx[(height - 1) * width + j] = gy[(height - 1) * width + j] = 0;
  }

  for(i = 1; i < height - 1; i++) {
    gx[i * width] = gy[i * width] = 0;
    for(j = 1; j < width - 1; j++) {
      gx[i * width + j] = (din[(i + 1) * width + j - 1] + (din[(i + 1) * width + j] << 1) + din[(i + 1) * width + j + 1]) -
                (din[(i - 1) * width + j - 1] + (din[(i - 1) * width + j] << 1) + din[(i - 1) * width + j + 1]);
      gy[i * width + j] = (din[(i - 1) * width + j + 1] + (din[i * width + j + 1] << 1) + din[(i + 1) * width + j + 1]) -
                (din[(i - 1) * width + j - 1] + (din[i * width + j - 1] << 1) + din[(i + 1) * width + j - 1]);
    }
    gx[i * width + j] = gy[i * width + j] = 0;
  }
  return 0;
}

#if 0
int find_dir(int width, int height, unsigned char *din, unsigned char *dir) {
  int i, j, ii, jj, n, m;
  short *gx, *gy;
  int *vx, *vy;
  signed char *cosq, *sinq; // cos() and sin()
  signed char *cosm, *sinm; // median of cos() and sin()
  int dirw, dirh;
  float ratio;
  int ret;

  gx = (short *)malloc(width * height * sizeof(short));
  gy = (short *)malloc(width * height * sizeof(short));
  vx = (int *)malloc((width / WIN8) * (height / WIN8) * sizeof(int));
  vy = (int *)malloc((width / WIN8) * (height / WIN8) * sizeof(int));
  memset(vx, 0, (width / WIN8) * (height / WIN8) * sizeof(int));
  memset(vy, 0, (width / WIN8) * (height / WIN8) * sizeof(int));

  ret = gradient_filter(width, height, din, gx, gy);

  for(i = 0; i < height - WIN8; i += WIN8) {
    for(j = 0; j < width - WIN8; j += WIN8) {
      m = (i / WIN8) * (width / WIN8) + j / WIN8;
      for(ii = 0; ii < WIN8; ii++) {
        for(jj = 0; jj < WIN8; jj++) {
          n = (i + ii) * width + j + jj;
          vx[m] += (gx[n] * gy[n]) << 1;
          vy[m] += gx[n] * gx[n] - gy[n] * gy[n];
        }
      }
    }
  }

  dirw = width / WIN8;
  dirh = height / WIN8;
  cosq = (signed char *)gx;
  sinq = (signed char *)gy;

  for(i = 0; i < dirh; i++) {
    for(j = 0; j < dirw; j++) {
      int vx1, vx2, vy1;
      n = i * dirw + j;
      vx1 = ABS(106 * vx[n]);	// tan(22.5) = 106/256
      vx2 = ABS(618 * vx[n]);	// tan(67.5) = 618/256
      vy1 = ABS(vy[n] << 8);
      if (vx[n]==0) vy1 = 700 * vy[n]; // >618 is ok

      if (vy[n] >= 0) {
        if (vx[n] >= 0) {
          if (vy1 < vx1) {
            dir[n] = 6;
            cosq[n] = 2;  //1 cos0
            sinq[n] = 0;  //0 sin0
          } else if (vy1 >= vx1 && vy1 < vx2) {
            dir[n] = 7;
            cosq[n] = 1;	// 0.7 cos45
            sinq[n] = 1;	// 0.7 sin45
          } else {//if (vy1>=vx2)
            dir[n] = 0;
            cosq[n] = 0;	// 0 cos90
            sinq[n] = 2;	// 1 sin90
          }
        } else {
          if (vy1 >= vx2) {
            dir[n] = 0;
            cosq[n] = 0;	// 0 cos90
            sinq[n] = 2;	// 1 sin90
          } else if (vy1 >= vx1 && vy1 < vx2) {
            dir[n] = 1;
            cosq[n] = -1;	// -0.7 cos135
            sinq[n] = 1;	// 0.7  sin135
          } else { //if (vy<vx1)
            dir[n] = 2;
            cosq[n] = -2;	// -1 cos180
            sinq[n] = 0;	// 0 sin180
          }
        }
      } else {
        if (vx[n] < 0) {
          if (vy1 < vx1) {
            dir[n] = 2;
            cosq[n] = -2;	// -1 cos180
            sinq[n] = 0;	// 0 sin180
          } else if (vy1 >= vx1 && vy1 < vx2) {
            dir[n] = 3;
            cosq[n] = -1;	// -0.7 cos225
            sinq[n] = -1;	// -0.7 sin225
          } else { //if (vy1>=vx2)
            dir[n] = 4;
            cosq[n] = 0;    // 0 cos270
            sinq[n] = -2;   // -1 sin270
          }
        } else {
          if (vy1 >= vx2) {
            dir[n] = 4;
            cosq[n] = 0;	// 0 cos270
            sinq[n] = -2;	// -1 sin270
          } else if (vy1>=vx1 && vy1<vx2) {
            dir[n] = 5;
            cosq[n] = 1;	// 0.7 cos315
            sinq[n] = -1;	// -0.7 sin315
          } else {//if (vy<vx1)
            dir[n] = 6;
            cosq[n] = 2;	// 1 cos0
            sinq[n] = 0;	// 0 sin0
          }
        }
      }
    }
  }

  cosm = (signed char *)vx;
  sinm = (signed char *)vy;
  ret = mean_filter(dirw, dirh, cosq, cosm);
  ret = mean_filter(dirw, dirh, sinq, sinm);
  //memcpy(cosm, cosq, dirw*dirh);
  //memcpy(sinm, sinq, dirw*dirh);

  for(i = 2; i < dirh - 2; i++) {
    for(j = 2; j < dirw - 2; j++) {
      n = i * dirw + j;
      jj = cosm[n] + 2;
      ii = sinm[n] + 2;
      dir[n] = dir_lookup[ii][jj];
    }
  }

  free(gx);
  free(gy);
  free(vx);
  free(vy);

  return 0;
}
#else
int find_dir(int width, int height, unsigned char *din, unsigned char *dir) {
  int i, j, ii, jj, n, m;
  short *gx, *gy;
  int *vx, *vy, *vxo, *vyo;
  unsigned char *diro;
  signed char *cosq, *sinq; // cos() and sin()
  signed char *cosm, *sinm; // median of cos() and sin()
  int dirw, dirh;
  float ratio;
  int ret;

  gx = (short *)malloc(width * height * sizeof(short));
  gy = (short *)malloc(width * height * sizeof(short));
  vx = (int *)malloc((width / WIN8) * (height / WIN8) * sizeof(int));
  vy = (int *)malloc((width / WIN8) * (height / WIN8) * sizeof(int));
  vxo = (int *)malloc((width / WIN8) * (height / WIN8) * sizeof(int));
  vyo = (int *)malloc((width / WIN8) * (height / WIN8) * sizeof(int));
  diro = (unsigned char *)malloc((width / WIN8) * (height / WIN8) * sizeof(unsigned char));
  memset(vx, 0, (width / WIN8) * (height / WIN8) * sizeof(int));
  memset(vy, 0, (width / WIN8) * (height / WIN8) * sizeof(int));
  memset(vxo, 0, (width / WIN8) * (height / WIN8) * sizeof(int));
  memset(vyo, 0, (width / WIN8) * (height / WIN8) * sizeof(int));
  memset(diro, 0, (width / WIN8) * (height / WIN8) * sizeof(unsigned char));

  ret = gradient_filter(width, height, din, gx, gy);

  for(i = 0; i < height - WIN8; i += WIN8) {
    for(j = 0; j < width - WIN8; j += WIN8) {
      m = (i / WIN8) * (width / WIN8) + j / WIN8;
      if (j == 192 && i == 80)
        j += 0;
      for(ii = 0; ii < WIN8; ii++) {
        for(jj = 0; jj < WIN8; jj++) {
          n = (i + ii) * width + j + jj;
          vxo[m] += (gx[n] * gy[n]) << 1;
          vyo[m] += gx[n] * gx[n] - gy[n] * gy[n];
//          vx[m] += (gx[n] * gy[n]) << 1;
//          vy[m] += gx[n] * gx[n] - gy[n] * gy[n];
        }
      }
    }
  }

  dirw = width / WIN8;
  dirh = height / WIN8;
//  cosq = (signed char *)gx;
//  sinq = (signed char *)gy;

  for(i = 0; i < dirh; i++) {
    for(j = 0; j < dirw; j++) {
      int vx1, vx2, vy1;

      if (j == 8 / 8 && i == 8 / 8)
        j += 0;

      n = i * dirw + j;


//      vx1 = ABS(106 * (vx[n] >> 8));	// tan(22.5) = 106/256
//      vx2 = ABS(618 * (vx[n] >> 8));	// tan(67.5) = 618/256
      vx1 = ABS(128 * (vx[n] >> 8));	// tan(26.565) = 128/256
      vx2 = ABS(512 * (vx[n] >> 8));	// tan(63.434) = 512/256
//      vx1 = ABS(102 * (vxo[n] >> 8));	// tan(21.8) = (0.4 * 256)/256
//      vx2 = ABS(640 * (vxo[n] >> 8));	// tan(68.2) = (2.5 * 256)/256
//      vy1 = ABS(vy[n] << 8);
      vy1 = ABS(vyo[n]);
      if (vxo[n] == 0) vy1 = 700 * vyo[n]; // >618 is ok

      if (vyo[n] >= 0) {
        if (vxo[n] >= 0) {
          if (vy1 < vx1) {
            diro[n] = 6;
//            cosq[n] = 2;  //1 cos0
//            sinq[n] = 0;  //0 sin0
          } else if (vy1 >= vx1 && vy1 < vx2) {
            diro[n] = 7;
//            cosq[n] = 1;	// 0.7 cos45
//            sinq[n] = 1;	// 0.7 sin45
          } else {//if (vy1>=vx2)
            diro[n] = 0;
//            cosq[n] = 0;	// 0 cos90
//            sinq[n] = 2;	// 1 sin90
          }
        } else {
          if (vy1 >= vx2) {
            diro[n] = 0;
//            cosq[n] = 0;	// 0 cos90
//            sinq[n] = 2;	// 1 sin90
          } else if (vy1 >= vx1 && vy1 < vx2) {
            diro[n] = 1;
//            cosq[n] = -1;	// -0.7 cos135
//            sinq[n] = 1;	// 0.7  sin135
          } else { //if (vy<vx1)
            diro[n] = 2;
//            cosq[n] = -2;	// -1 cos180
//            sinq[n] = 0;	// 0 sin180
          }
        }
      } else {
        if (vxo[n] < 0) {
          if (vy1 < vx1) {
            diro[n] = 2;
//            cosq[n] = -2;	// -1 cos180
//            sinq[n] = 0;	// 0 sin180
          } else if (vy1 >= vx1 && vy1 < vx2) {
            diro[n] = 3;
//            cosq[n] = -1;	// -0.7 cos225
//            sinq[n] = -1;	// -0.7 sin225
          } else { //if (vy1>=vx2)
            diro[n] = 4;
//            cosq[n] = 0;    // 0 cos270
//            sinq[n] = -2;   // -1 sin270
          }
        } else {
          if (vy1 >= vx2) {
            diro[n] = 4;
//            cosq[n] = 0;	// 0 cos270
//            sinq[n] = -2;	// -1 sin270
          } else if (vy1>=vx1 && vy1<vx2) {
            diro[n] = 5;
//            cosq[n] = 1;	// 0.7 cos315
//            sinq[n] = -1;	// -0.7 sin315
          } else {//if (vy<vx1)
            diro[n] = 6;
//            cosq[n] = 2;	// 1 cos0
//            sinq[n] = 0;	// 0 sin0
          }
        }
      }
    }
  }

  intBlur(vx, vxo, width / WIN8, height / WIN8);
  intBlur(vy, vyo, width / WIN8, height / WIN8);
//  memcpy(vx, vxo, width / WIN8 * height / WIN8 * sizeof(int));
//  memcpy(vy, vyo, width / WIN8 * height / WIN8 * sizeof(int));

  for(i = 0; i < dirh; i++) {
    for(j = 0; j < dirw; j++) {
      int vx1, vx2, vy1;

      n = i * dirw + j;
//      vx1 = ABS(106 * (vx[n] >> 8));	// tan(22.5) = 106/256
//      vx2 = ABS(618 * (vx[n] >> 8));	// tan(67.5) = 618/256
      vx1 = ABS(128 * (vx[n] >> 8));	// tan(26.565) = 128/256
      vx2 = ABS(512 * (vx[n] >> 8));	// tan(63.434) = 512/256
//      vx1 = ABS(102 * (vx[n] >> 8));	// tan(21.8) = (0.4 * 256)/256
//      vx2 = ABS(640 * (vx[n] >> 8));	// tan(68.2) = (2.5 * 256)/256
//      vy1 = ABS(vy[n] << 8);
      vy1 = ABS(vy[n]);
      if (vx[n]==0) vy1 = 700 * vy[n]; // >618 is ok

      if (vy[n] >= 0) {
        if (vx[n] >= 0) {
          if (vy1 < vx1) {
            dir[n] = 6;
          } else if (vy1 >= vx1 && vy1 < vx2) {
            dir[n] = 7;
          } else {//if (vy1>=vx2)
            dir[n] = 0;
          }
        } else {
          if (vy1 >= vx2) {
            dir[n] = 0;
          } else if (vy1 >= vx1 && vy1 < vx2) {
            dir[n] = 1;
          } else { //if (vy<vx1)
            dir[n] = 2;
          }
        }
      } else {
        if (vx[n] < 0) {
          if (vy1 < vx1) {
            dir[n] = 2;
          } else if (vy1 >= vx1 && vy1 < vx2) {
            dir[n] = 3;
          } else { //if (vy1>=vx2)
            dir[n] = 4;
          }
        } else {
          if (vy1 >= vx2) {
            dir[n] = 4;
          } else if (vy1>=vx1 && vy1<vx2) {
            dir[n] = 5;
          } else {//if (vy<vx1)
            dir[n] = 6;
          }
        }
      }
    }
  }


/*
  cosm = (signed char *)vx;
  sinm = (signed char *)vy;
  ret = mean_filter(dirw, dirh, cosq, cosm);
  ret = mean_filter(dirw, dirh, sinq, sinm);
  //memcpy(cosm, cosq, dirw*dirh);
  //memcpy(sinm, sinq, dirw*dirh);

  for(i = 2; i < dirh - 2; i++) {
    for(j = 2; j < dirw - 2; j++) {
      n = i * dirw + j;
      jj = cosm[n] + 2;
      ii = sinm[n] + 2;
      dir[n] = dir_lookup[ii][jj];
    }
  }
*/
  free(gx);
  free(gy);
  free(vx);
  free(vy);
  free(vxo);
  free(vyo);

  return 0;
}

#endif

int expand_dir(int width, int height, unsigned char *dir, unsigned char *data)
{
	int i,j,ii,jj, n, m;

	for(i=0;i<height-WIN8;i+=WIN8) {
		for(j=0;j<width-WIN8;j+=WIN8) {
			m = dir[(i/WIN8)*(width/WIN8)+j/WIN8]&0x7;
			for(ii=0;ii<WIN8;ii++) {
				for(jj=0;jj<WIN8;jj++) {
					n = (i+ii)*width + (j+jj);
					data[n] = dir_pattern0[m][ii*WIN8+jj];
				}
			}
		}
	}
	return 0;
}

int dir_filter(int width, int height, unsigned char *din, unsigned char *dir, unsigned char *mean, unsigned char *variance, unsigned char *dout) {
  int i, j, ii, jj, idx, m, n, n0, n1, n2, n3, n4, n5, n6, n7;
  int sum, valm, valv;

  for(i = 8; i < height - WIN8 * 2; i += WIN8) {
    for(j = 8; j < width - WIN8 * 2; j += WIN8) {
      m = (i / WIN8) * (width / WIN8) + j / WIN8;
      valm = mean[m];
      valv = variance[m] >> 1;
      idx = dir[m] & 0x7;
      for(ii = 0; ii < WIN8; ii++) {
        for(jj = 0; jj < WIN8; jj++) {
          n = (i + ii) * width + (j + jj);
/*
					n0 = (i+ii+dir_mask[idx][0][1])*width + (j+jj+dir_mask[idx][0][0]);
					n1 = (i+ii+dir_mask[idx][1][1])*width + (j+jj+dir_mask[idx][1][0]);
					n2 = (i+ii+dir_mask[idx][2][1])*width + (j+jj+dir_mask[idx][2][0]);
					n3 = (i+ii+dir_mask[idx][3][1])*width + (j+jj+dir_mask[idx][3][0]);
					sum = ((unsigned int)din[n0] + (unsigned int)din[n1] +
						   (unsigned int)din[n2] + (unsigned int)din[n3])>>2;

*/
          n0 = (i + ii + dir_mask[idx][0][1]) * width + (j + jj + dir_mask[idx][0][0]);
          n1 = (i + ii + dir_mask[idx][1][1]) * width + (j + jj + dir_mask[idx][1][0]);
          n2 = (i + ii + dir_mask[idx][2][1]) * width + (j + jj + dir_mask[idx][2][0]);
          n3 = (i + ii + dir_mask[idx][3][1]) * width + (j + jj + dir_mask[idx][3][0]);
          n4 = (i + ii + dir_mask[idx][4][1]) * width + (j + jj + dir_mask[idx][4][0]);
          n5 = (i + ii + dir_mask[idx][5][1]) * width + (j + jj + dir_mask[idx][5][0]);
          n6 = (i + ii + dir_mask[idx][6][1]) * width + (j + jj + dir_mask[idx][6][0]);
          n7 = (i + ii + dir_mask[idx][7][1]) * width + (j + jj + dir_mask[idx][7][0]);
          sum = ((unsigned int)din[n0] + (unsigned int)din[n1] +
                 (unsigned int)din[n2] + (unsigned int)din[n3] +
                 (unsigned int)din[n4] + (unsigned int)din[n5] +
                 (unsigned int)din[n6] + (unsigned int)din[n7] + 4) >> 3;

          if (sum > valm) sum += valv;
          if (sum < valm) sum -= valv;
          if (sum > 255) sum = 255;
          if (sum < 0) sum = 0;
          dout[n] = (unsigned char)sum;
        }
      }
    }
  }

  return 0;
}

int binary_filter(int width, int height, unsigned char *din, unsigned char *mean, unsigned char *dout) {
  int i, j, ii, jj, n, m, val;

  for(i = 0; i < height - WIN8; i += WIN8) {
    for(j = 0; j < width - WIN8; j += WIN8) {
//      m = (i / WIN8) * (width / WIN8) + j / WIN8;
//      val = (mean[m] + mean[m + 1] + mean[m + (width / WIN8)] + mean[m + (width / WIN8) + 1]) >> 2;
      val = mean[(i / WIN8) * (width / WIN8) + j / WIN8];
      for(ii = 0; ii < WIN8; ii++) {
        for(jj = 0; jj < WIN8; jj++) {
          n = (i + ii) * width + (j + jj);
          dout[n] = (din[n] >= val) ? 0 : 1;
        }
      }
    }
  }

  return 0;
}

#if 0
int smooth_filter(int width, int height, unsigned char *din, unsigned char *dout) {
  int i, j;

  for(i = 1; i < height - 1; i++) {
    for(j = 1; j < width - 1; j++) {
      unsigned char p1 = din[i       * width + j    ];
      unsigned char p2 = din[(i - 1) * width + j    ];
      unsigned char p3 = din[(i - 1) * width + j + 1];
      unsigned char p4 = din[i       * width + j + 1];
      unsigned char p5 = din[(i + 1) * width + j + 1];
      unsigned char p6 = din[(i + 1) * width + j    ];
      unsigned char p7 = din[(i + 1) * width + j - 1];
      unsigned char p8 = din[i       * width + j - 1];
      unsigned char p9 = din[(i - 1) * width + j - 1];
      int A = p1 + p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9;
      if (A > 4) dout[i * width + j] = 1;
      else dout[i * width + j] = 0;
    }
  }
  return 0;
}
#else
int smooth_filter(int width, int height, unsigned char *din, unsigned char *dout, int region_left, int region_top, int region_right, int region_bottom) {
  int i, j, A;
  unsigned char p1, p2, p3, p4, p5, p6, p7, p8, p9;

  for(i = 0; i < height; i++) {
    for(j = 0; j < width; j++) {
      if (i < region_top || i >= region_bottom || j < region_left || j >= region_right) {
        dout[i * width + j] = 0;
        continue;
      }
      p1 = din[i       * width + j    ];
      p2 = din[(i - 1) * width + j    ];
      p3 = din[(i - 1) * width + j + 1];
      p4 = din[i       * width + j + 1];
      p5 = din[(i + 1) * width + j + 1];
      p6 = din[(i + 1) * width + j    ];
      p7 = din[(i + 1) * width + j - 1];
      p8 = din[i       * width + j - 1];
      p9 = din[(i - 1) * width + j - 1];
      A = p1 + p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9;
      if (A > 4) dout[i * width + j] = 1;
      else dout[i * width + j] = 0;
    }
  }
  return 0;
}
#endif

#if 0
int thinning_teration(int width, int height, unsigned char *din, unsigned char *dout, int iter) {
  int flag = 0;
  int i, j, A, B, m1, m2;

  for(i = 1; i < height - 1; i++) {
    for(j = 1; j < width - 1; j++) {
      unsigned char p1 = din[i * width + j];
      if(p1) {
        unsigned char p2 = din[(i - 1) * width + j];
        unsigned char p3 = din[(i - 1) * width + j + 1];
        unsigned char p4 = din[i       * width + j + 1];
        unsigned char p5 = din[(i + 1) * width + j + 1];
        unsigned char p6 = din[(i + 1) * width + j    ];
        unsigned char p7 = din[(i + 1) * width + j - 1];
        unsigned char p8 = din[i       * width + j - 1];
        unsigned char p9 = din[(i - 1) * width + j - 1];

        A  = (p2 == 0 && p3 == 1) + (p3 == 0 && p4 == 1) +
                (p4 == 0 && p5 == 1) + (p5 == 0 && p6 == 1) +
                (p6 == 0 && p7 == 1) + (p7 == 0 && p8 == 1) +
                (p8 == 0 && p9 == 1) + (p9 == 0 && p2 == 1);
        B  = p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9;
        m1 = iter == 0 ? (p2 * p4 * p6) : (p2 * p4 * p8);
        m2 = iter == 0 ? (p4 * p6 * p8) : (p2 * p6 * p8);

		// [SmallY20160718] change rule
//		if (A == 1 && (B >= 2 && B <= 6) && m1 == 0 && m2 == 0) {
		if (A == 1 && (B >= 3 && B <= 6) && m1 == 0 && m2 == 0) {
          dout[i * width + j] = 0;
          flag = 1;
        } else
          dout[i * width + j] = 1;
      } else
        dout[i * width + j] = 0;
    }
  }

  return flag;
}
#else
static unsigned char mapThinning[256] = {
  6, 6, 6, 6, 6, 6, 6, 7, 6, 6, 6, 6, 6, 6, 7, 7,
  6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 6, 7, 7,
  6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 4, 4, 6, 6, 4, 4,
  6, 6, 6, 6, 6, 6, 6, 6, 7, 6, 4, 4, 7, 6, 5, 5,
  6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
  6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
  6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 4, 4, 6, 6, 4, 4,
  7, 6, 6, 6, 6, 6, 6, 6, 7, 6, 4, 4, 7, 6, 5, 4,
  6, 6, 6, 7, 6, 6, 6, 7, 6, 6, 2, 2, 6, 6, 2, 3,
  6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 2, 2, 6, 6, 2, 3,
  6, 6, 2, 2, 6, 6, 2, 2, 4, 4, 0, 0, 4, 4, 0, 0,
  6, 6, 2, 2, 6, 6, 2, 2, 4, 4, 0, 0, 4, 4, 0, 0,
  6, 7, 6, 7, 6, 6, 6, 7, 6, 6, 2, 2, 6, 6, 2, 3,
  6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 2, 2, 6, 6, 2, 2,
  7, 7, 2, 3, 6, 6, 2, 3, 4, 4, 0, 0, 4, 4, 0, 0,
  7, 7, 2, 3, 6, 6, 2, 2, 5, 5, 0, 0, 5, 4, 0, 0
};

// [SmallY20160812] speed up test 1
int thinning_teration(int width, int height, unsigned char *din, unsigned char *dout, int iter, int region_left, int region_top, int region_right, int region_bottom) {
  int flag = 0;
  int i, j, jj, A, B, m1, m2;
  unsigned char *pt, *pc, *pb, pattern, state;
  unsigned int *pcii, *pcoi;

  for(i = region_top; i < region_bottom; i++) {
    pcii = (unsigned int *)(din + i * width + region_left);
    pcoi = (unsigned int *)(dout + i * width + region_left);
    pt = din + (i - 1) * width + region_left;
    pc = din +  i      * width + region_left;
    pb = din + (i + 1) * width + region_left;
    for(j = region_left; j < region_right; j += 4) {
      if (*pcii) {
        for (jj = j; jj < j + 4; jj++) {
          if (*pc) {
            pattern = (*(pt - 1)     ) | (*(pt    ) << 7) | (*(pt + 1) << 6) |
                      (*(pc - 1) << 1)                    | (*(pc + 1) << 5) |
                      (*(pb - 1) << 2) | (*(pb    ) << 3) | (*(pb + 1) << 4);
            state = mapThinning[pattern];

            if ((state & 1) && ((iter == 0 && ((state & 2) == 2)) || (iter == 1 && ((state & 4) == 4)))) {
              dout[i * width + jj] = 0;
              flag = 1;
            } else
              dout[i * width + jj] = 1;
          } else
            dout[i * width + jj] = 0;
          pt++;
          pc++;
          pb++;
        }
      } else {
        *pcoi = 0;
        pt += 4;
        pc += 4;
        pb += 4;
      } // end of if (*pci) {
      pcii++;
      pcoi++;
    }
  }

  return flag;
}
#endif

#if 0
int post_thinning(int width, int height, unsigned char *data) {
  int i, j;

  for(i = 1; i < height - 1; i++) {
    for(j = 1; j < width - 1; j++) {
      unsigned char p1 = data[i * width + j];
      if (p1) {
        unsigned char p2 = data[(i - 1) * width + j    ];
        unsigned char p3 = data[(i - 1) * width + j + 1];
        unsigned char p4 = data[i       * width + j + 1];
        unsigned char p5 = data[(i + 1) * width + j + 1];
        unsigned char p6 = data[(i + 1) * width + j    ];
        unsigned char p7 = data[(i + 1) * width + j - 1];
        unsigned char p8 = data[i       * width + j - 1];
        unsigned char p9 = data[(i - 1) * width + j - 1];
        /* remove -*- and -*- case
                  **-     ***
                  ---	  ---     */
        if ((p2 == 1 && p4 == 1 && p7 == 0) |
            (p4 == 1 && p6 == 1 && p9 == 0) |
            (p6 == 1 && p8 == 1 && p3 == 0) |
            (p8 == 1 && p2 == 1 && p5 == 0))
          data[i * width + j] = 0;
      }
    }
  }

  return 0;
}
#else
int post_thinning(int width, int height, unsigned char *data, int region_left, int region_top, int region_right, int region_bottom) {
  int i, j;

  for(i = region_top; i < region_bottom; i++) {
    for(j = region_left; j < region_right; j++) {
      unsigned char p1 = data[i * width + j];
      if (p1) {
        unsigned char p2 = data[(i - 1) * width + j    ];
        unsigned char p3 = data[(i - 1) * width + j + 1];
        unsigned char p4 = data[i       * width + j + 1];
        unsigned char p5 = data[(i + 1) * width + j + 1];
        unsigned char p6 = data[(i + 1) * width + j    ];
        unsigned char p7 = data[(i + 1) * width + j - 1];
        unsigned char p8 = data[i       * width + j - 1];
        unsigned char p9 = data[(i - 1) * width + j - 1];
        /* remove -*- and -*- case
                  **-     ***
                  ---	  ---     */
        if ((p2 == 1 && p4 == 1 && p7 == 0) |
            (p4 == 1 && p6 == 1 && p9 == 0) |
            (p6 == 1 && p8 == 1 && p3 == 0) |
            (p8 == 1 && p2 == 1 && p5 == 0))
          data[i * width + j] = 0;
      }
    }
  }

  return 0;
}
#endif

#if 0
int thinning_filter(int width, int height, unsigned char *data, unsigned char *tmp) {
  int ret, count = 0;

  do {
    ret = 0;
    ret += thinning_teration(width, height, data, tmp, 0);
    ret += thinning_teration(width, height, tmp, data, 1);
    count++;
  } while (ret > 0);

  printf("thinning iteration count = %d\n", count);

  post_thinning(width, height, data);

  return 0;
}
#else
// [SmallY20160901] set active region to speed up
int thinning_filter(int width, int height, unsigned char *data, unsigned char *tmp, int region_left, int region_top, int region_right, int region_bottom) {
  int ret, count = 0;

  if (region_top < 1) region_top = 1;
  if (region_bottom > height - 1) region_bottom = height - 1;
  region_left &= 0x0FFFC;
  if (region_left < 4) region_left = 4;
  region_right &= 0x0FFFC;
  if (region_right == width) region_right -= 4;

  do {
    ret = 0;
    ret += thinning_teration(width, height, data, tmp, 0, region_left, region_top, region_right, region_bottom);
    ret += thinning_teration(width, height, tmp, data, 1, region_left, region_top, region_right, region_bottom);
    count++;
  } while (ret > 0);

//  printf("thinning iteration count = %d\n", count);

  post_thinning(width, height, data, region_left, region_top, region_right, region_bottom);

  return 0;
}
#endif

int find_minutiae(int width, int height, unsigned char *din, unsigned char *border, minutiae *minutiaes) {
  int cnt = 0;
  int i, j, A;

  for(i = 16; i < height - 16; i++) {
    for(j = 16; j < width - 16; j++) {
      if(din[i * width + j]) {
        unsigned char p2 = din[(i - 1) * width + j    ];
        unsigned char p3 = din[(i - 1) * width + j + 1];
        unsigned char p4 = din[i       * width + j + 1];
        unsigned char p5 = din[(i + 1) * width + j + 1];
        unsigned char p6 = din[(i + 1) * width + j    ];
        unsigned char p7 = din[(i + 1) * width + j - 1];
        unsigned char p8 = din[i       * width + j - 1];
        unsigned char p9 = din[(i - 1) * width + j - 1];
        A = (p2 ^ p3) + (p3 ^ p4) + (p4 ^ p5) + (p5 ^ p6) + (p6 ^ p7) + (p7 ^ p8) + (p8 ^ p9) + (p9 ^ p2);
        if(A == 2 || A == 6 || A == 8) {
          if(!border[(i / WIN8) * (width / WIN8) + j / WIN8]) {
            if(A == 2) {
              minutiaes[cnt].type = 1;
              minutiaes[cnt].x = j;
              minutiaes[cnt].y = i;
              cnt++;
            } else if (A == 6) {
              minutiaes[cnt].type = 2;
              minutiaes[cnt].x = j;
              minutiaes[cnt].y = i;
              cnt++;
            }
          }
        }
      }
    }
  }
  return cnt;
}

int expand_minutiae(int width, int height, int size, minutiae *minutiaes,
					unsigned char *end, unsigned char *fork, int en_dir)
{
	int i,j,k;

	memset(end, 0, width*height);
	memset(fork, 0, width*height);
	for(i=0;i<size;i++) {
		if (minutiaes[i].type==1) {
			if(en_dir) {
				for(k=0;k<8;k++) {
					for(j=0;j<8;j++) {
						if(minutiaes[i].dir<=4 && minutiaes[i].dir>=0 && dir_pattern1[minutiaes[i].dir][k*8+j])
							end[(minutiaes[i].y+k)*width+minutiaes[i].x+j] = dir_pattern1[minutiaes[i].dir][k*8+j];
						else if (minutiaes[i].dir<=8 && minutiaes[i].dir>4 && dir_pattern1[8-minutiaes[i].dir][k*8+j])
							end[(minutiaes[i].y+k)*width+minutiaes[i].x-j] = dir_pattern1[8-minutiaes[i].dir][k*8+j];
						else if (minutiaes[i].dir<=12 && minutiaes[i].dir>8 && dir_pattern1[minutiaes[i].dir-8][k*8+j])
							end[(minutiaes[i].y-k)*width+minutiaes[i].x-j] = dir_pattern1[minutiaes[i].dir-8][k*8+j];
						else if (minutiaes[i].dir>12 && dir_pattern1[16-minutiaes[i].dir][k*8+j])
							end[(minutiaes[i].y-k)*width+minutiaes[i].x+j] = dir_pattern1[16-minutiaes[i].dir][k*8+j];

					}
				}
			}
			end[minutiaes[i].y*width+minutiaes[i].x] = 0xff;
			end[(minutiaes[i].y-1)*width+minutiaes[i].x] = 0xff;
			end[(minutiaes[i].y-1)*width+minutiaes[i].x+1] = 0xff;
			end[minutiaes[i].y*width+minutiaes[i].x+1] = 0xff;
			end[(minutiaes[i].y+1)*width+minutiaes[i].x+1] = 0xff;
			end[(minutiaes[i].y+1)*width+minutiaes[i].x] = 0xff;
			end[(minutiaes[i].y+1)*width+minutiaes[i].x-1] = 0xff;
			end[minutiaes[i].y*width+minutiaes[i].x-1] = 0xff;
			end[(minutiaes[i].y-1)*width+minutiaes[i].x-1] = 0xff;
		} else if (minutiaes[i].type==2) {
			if(en_dir) {
				for(k=0;k<8;k++) {
					for(j=0;j<8;j++) {
						if(minutiaes[i].dir<=4 && minutiaes[i].dir>=0 && dir_pattern1[minutiaes[i].dir][k*8+j])
							fork[(minutiaes[i].y+k)*width+minutiaes[i].x+j] = dir_pattern1[minutiaes[i].dir][k*8+j];
						else if (minutiaes[i].dir<=8 && minutiaes[i].dir>4 && dir_pattern1[8-minutiaes[i].dir][k*8+j])
							fork[(minutiaes[i].y+k)*width+minutiaes[i].x-j] = dir_pattern1[8-minutiaes[i].dir][k*8+j];
						else if (minutiaes[i].dir<=12 && minutiaes[i].dir>8 && dir_pattern1[minutiaes[i].dir-8][k*8+j])
							fork[(minutiaes[i].y-k)*width+minutiaes[i].x-j] = dir_pattern1[minutiaes[i].dir-8][k*8+j];
						else if (minutiaes[i].dir>12 && dir_pattern1[16-minutiaes[i].dir][k*8+j])
							fork[(minutiaes[i].y-k)*width+minutiaes[i].x+j] = dir_pattern1[16-minutiaes[i].dir][k*8+j];

					}
				}
			}
			fork[minutiaes[i].y*width+minutiaes[i].x] = 0xff;
			fork[(minutiaes[i].y-1)*width+minutiaes[i].x] = 0xff;
			fork[(minutiaes[i].y-1)*width+minutiaes[i].x+1] = 0xff;
			fork[minutiaes[i].y*width+minutiaes[i].x+1] = 0xff;
			fork[(minutiaes[i].y+1)*width+minutiaes[i].x+1] = 0xff;
			fork[(minutiaes[i].y+1)*width+minutiaes[i].x] = 0xff;
			fork[(minutiaes[i].y+1)*width+minutiaes[i].x-1] = 0xff;
			fork[minutiaes[i].y*width+minutiaes[i].x-1] = 0xff;
			fork[(minutiaes[i].y-1)*width+minutiaes[i].x-1] = 0xff;
		}
	}

	return 0;
}

int check_minutiae(int size, minutiae *minutiaes, minutiae check)
{
	int i;
	for (i=0;i<size;i++) {
		if (minutiaes[i].x == check.x &&
			minutiaes[i].y == check.y)
			return i;
	}
	return -1;
}

int search_next(int width, unsigned char *thinning, int x, int y, int prev, minutiae_list *mlist, int cnt, int iter);

int search_end(int width, unsigned char *thinning, minutiae_list *mlist)
{
	int cnt, ret = 0; 
	int x = mlist->item[0].x;
	int y = mlist->item[0].y;
	unsigned char p2 = thinning[(y-1)*width+x];
	unsigned char p3 = thinning[(y-1)*width+x+1];
	unsigned char p4 = thinning[y*width+x+1];
	unsigned char p5 = thinning[(y+1)*width+x+1];
	unsigned char p6 = thinning[(y+1)*width+x];
	unsigned char p7 = thinning[(y+1)*width+x-1];
	unsigned char p8 = thinning[y*width+x-1];
	unsigned char p9 = thinning[(y-1)*width+x-1];
	
	cnt = SPIKE_THRESHOLD;

	if (p2>0)
		ret = search_next(width, thinning, x, y-1, 2, mlist, cnt, 0);
	else if (p3>0)
		ret = search_next(width, thinning, x+1,y-1, 3, mlist, cnt, 0);
	else if (p4>0)
		ret = search_next(width, thinning, x+1, y, 4, mlist, cnt,  0);
	else if (p5>0)
		ret = search_next(width, thinning, x+1, y+1, 5, mlist, cnt, 0);
	else if (p6>0)
		ret = search_next(width, thinning, x, y+1, 6, mlist, cnt, 0);
	else if (p7>0)
		ret = search_next(width, thinning, x-1, y+1, 7, mlist, cnt, 0);
	else if (p8>0)
		ret = search_next(width, thinning, x-1, y, 8, mlist, cnt, 0);
	else if (p9>0)
		ret = search_next(width, thinning, x-1, y-1, 9, mlist, cnt, 0);
	
	mlist->item[0].vx[0] = mlist->item[1].x - x;
	mlist->item[0].vy[0] = mlist->item[1].y - y;

	return ret;
}

int search_fork(int width, unsigned char *thinning, minutiae_list *mlist, int iter)
{
	int cnt, ret, last;
	int idx = mlist->size-1;
	int x = mlist->item[idx].x;
	int y = mlist->item[idx].y;
	int v = 0;

	unsigned char p2 = thinning[(y-1)*width+x];
	unsigned char p3 = thinning[(y-1)*width+x+1];
	unsigned char p4 = thinning[y*width+x+1];
	unsigned char p5 = thinning[(y+1)*width+x+1];
	unsigned char p6 = thinning[(y+1)*width+x];
	unsigned char p7 = thinning[(y+1)*width+x-1];
	unsigned char p8 = thinning[y*width+x-1];
	unsigned char p9 = thinning[(y-1)*width+x-1];

	cnt = LADDER_THRESHOLD;

	if (p2>0) {
		ret = search_next(width, thinning, x, y-1, 2, mlist, cnt, iter);
		last = mlist->idx;
		mlist->item[idx].vx[v] = mlist->item[last].x - mlist->item[idx].x;
		mlist->item[idx].vy[v++] = mlist->item[last].y - mlist->item[idx].y;
	}
	if (p4>0) {
		ret = search_next(width, thinning, x+1, y, 4, mlist, cnt, iter);
		last = mlist->idx;
		mlist->item[idx].vx[v] = mlist->item[last].x - mlist->item[idx].x;
		mlist->item[idx].vy[v++] = mlist->item[last].y - mlist->item[idx].y;
	}
	if (p6>0) {
		ret = search_next(width, thinning, x, y+1, 6, mlist, cnt, iter);
		last = mlist->idx;
		mlist->item[idx].vx[v] = mlist->item[last].x - mlist->item[idx].x;
		mlist->item[idx].vy[v++] = mlist->item[last].y - mlist->item[idx].y;
	}
	if (p8>0) {
		ret = search_next(width, thinning, x-1, y, 8, mlist, cnt, iter);
		last = mlist->idx;
		mlist->item[idx].vx[v] = mlist->item[last].x - mlist->item[idx].x;
		mlist->item[idx].vy[v++] = mlist->item[last].y - mlist->item[idx].y;
	}
	if (p3>0) { 
		ret = search_next(width, thinning, x+1, y-1, 3, mlist, cnt, iter);
		last = mlist->idx;
		mlist->item[idx].vx[v] = mlist->item[last].x - mlist->item[idx].x;
		mlist->item[idx].vy[v++] = mlist->item[last].y - mlist->item[idx].y;
	}
	if (p5>0) {
		ret = search_next(width, thinning, x+1, y+1, 5, mlist, cnt, iter);
		last = mlist->idx;
		mlist->item[idx].vx[v] = mlist->item[last].x - mlist->item[idx].x;
		mlist->item[idx].vy[v++] = mlist->item[last].y - mlist->item[idx].y;
	} 
	if (p7>0) {
		ret = search_next(width, thinning, x-1, y+1, 7, mlist, cnt, iter);
		last = mlist->idx;
		mlist->item[idx].vx[v] = mlist->item[last].x - mlist->item[idx].x;
		mlist->item[idx].vy[v++] = mlist->item[last].y - mlist->item[idx].y;
	}
	if (p9>0) {
		ret = search_next(width, thinning, x-1, y-1, 9, mlist, cnt, iter);
		last = mlist->idx;
		mlist->item[idx].vx[v] = mlist->item[last].x - mlist->item[idx].x;
		mlist->item[idx].vy[v++] = mlist->item[last].y - mlist->item[idx].y;
	}
	
	return ret;
}

int search_next(int width, unsigned char *thinning, int x, int y, int prev, minutiae_list *mlist, int cnt, int iter)
{
	int idx, ret = 0; 
	unsigned char p1 = thinning[y*width+x];
	unsigned char p2 = thinning[(y-1)*width+x];
	unsigned char p3 = thinning[(y-1)*width+x+1];
	unsigned char p4 = thinning[y*width+x+1];
	unsigned char p5 = thinning[(y+1)*width+x+1];
	unsigned char p6 = thinning[(y+1)*width+x];
	unsigned char p7 = thinning[(y+1)*width+x-1];
	unsigned char p8 = thinning[y*width+x-1];
	unsigned char p9 = thinning[(y-1)*width+x-1];
	int A = (p2^p3) + (p3^p4) + (p4^p5) + (p5^p6) + 
			(p6^p7) + (p7^p8) + (p8^p9) + (p9^p2);
	
	if (A==2) {
		mlist->item[mlist->size].x = x;
		mlist->item[mlist->size].y = y;
		mlist->item[mlist->size].type = 1;
		mlist->idx = mlist->size++;
		return cnt;
	} else if (A==6 || A==8) {
		int k;
		if (A==8) 
			printf("Error for tracing A=8!\n");

		mlist->item[mlist->size].x = x;
		mlist->item[mlist->size].y = y;
		mlist->item[mlist->size].type = 3;  // special fork with no vx,vy data
		idx = mlist->size++;		
		if (iter--) {
			cnt = LADDER_THRESHOLD;
			mlist->item[idx].type = 2;
			ret = search_fork(width, thinning, mlist, iter);
		}
		mlist->idx = idx;
		return ret;
	} else if (cnt==0) {
		mlist->item[mlist->size].x = x;
		mlist->item[mlist->size].y = y;
		mlist->item[mlist->size].type = 0;
		mlist->idx = mlist->size++;
		return cnt;
	}

	cnt--;

	if (p2>0 && prev!=5 && prev!=6 && prev!=7)
		ret = search_next(width, thinning, x, y-1, 2, mlist, cnt, iter);
	else if (p4>0 && prev!=7 && prev!=8 && prev!=9)
		ret = search_next(width, thinning, x+1, y, 4, mlist, cnt, iter);
	else if (p6>0 && prev!=9 && prev!=2 && prev!=3)
		ret = search_next(width, thinning, x, y+1, 6, mlist, cnt, iter);
	else if (p8>0 && prev!=3 && prev!=4 && prev!=5)
		ret = search_next(width, thinning, x-1, y, 8, mlist, cnt, iter);
	else if (p3>0 && prev!=6 && prev!=7 && prev!=8)
		ret = search_next(width, thinning, x+1, y-1, 3, mlist, cnt, iter);
	else if (p5>0 && prev!=8 && prev!=9 && prev!=2)
		ret = search_next(width, thinning, x+1, y+1, 5, mlist, cnt, iter);
	else if (p7>0 && prev!=2 && prev!=3 && prev!=4)
		ret = search_next(width, thinning, x-1, y+1, 7, mlist, cnt, iter);
	else if (p9>0 && prev!=4 && prev!=5 && prev!=6)
		ret = search_next(width, thinning, x-1, y-1, 9, mlist, cnt, iter);

	return ret;
}

// [SmallY20160824] change from 16 dir to 32 dir, clockwise to counter-cloclwise
#if defined(DIR16)
int direction(int vx, int vy)
{
	int dir;
	int vx1 = ABS(51*vx);	// tan(11.25) = 51/256
	int vx2 = ABS(171*vx);	// tan(33.75) = 171/256
	int vx3 = ABS(383*vx);	// tan(56.25) = 383/256
	int vx4 = ABS(1287*vx);	// tan(78.75) = 1287/256
	int vy1 = ABS(256*vy);

	if (vx==0) vy1 = ABS(1536*vy);  // >1287 is ok

	if (vx>=0) {
		if (vy>=0) {
			if (vy1<vx1)
				dir = 0;
			else if (vy1>=vx1 && vy1<vx2)
				dir = 1;
			else if (vy1>=vx2 && vy1<vx3)
				dir = 2;
			else if (vy1>=vx3 && vy1<vx4)
				dir = 3;
			else //if (vy1>=vx4)
				dir = 4;
		} else {
			if (vy1<vx1)
				dir = 0;
			else if (vy1>=vx1 && vy1<vx2)
				dir = 15;
			else if (vy1>=vx2 && vy1<vx3)
				dir = 14;
			else if (vy1>=vx3 && vy1<vx4)
				dir = 13;
			else //if (vy1>=vx4)
				dir = 12;
		}
	} else {
		if (vy>=0) {
			if (vy1<vx1)
				dir = 8;
			else if (vy1>=vx1 && vy1<vx2)
				dir = 7;
			else if (vy1>=vx2 && vy1<vx3)
				dir = 6;
			else if (vy1>=vx3 && vy1<vx4)
				dir = 5;
			else //if (vy1>=vx4)
				dir = 4;
		} else {
			if (vy1<vx1)
				dir = 8;
			else if (vy1>=vx1 && vy1<vx2)
				dir = 9;
			else if (vy1>=vx2 && vy1<vx3)
				dir = 10;
			else if (vy1>=vx3 && vy1<vx4)
				dir = 11;
			else //if (vy1>=vx4)
				dir = 12;
		}
	}
	return dir;
}
#endif

#if defined(DIR32)
int direction(int vx, int vy)
{
  int dir;
  int vx1 = ABS(  25 * vx);  // tan( 5.625) =   25/256
  int vx2 = ABS(  78 * vx);  // tan(16.875) =   78/256
  int vx3 = ABS( 137 * vx);  // tan(28.125) =  137/256
  int vx4 = ABS( 210 * vx);  // tan(39.375) =  210/256
  int vx5 = ABS( 312 * vx);  // tan(50.625) =  312/256
  int vx6 = ABS( 479 * vx);  // tan(61.875) =  479/256
  int vx7 = ABS( 844 * vx);  // tan(73.125) =  844/256
  int vx8 = ABS(2599 * vx);  // tan(84.375) = 2599/256
  int vy1 = ABS( 256 * vy);

  if (vx == 0) vy1 = ABS(3000 * vy);  // >2599 is ok

  if (vx >= 0) {
    if (vy >= 0) {
      if (vy1 < vx4) {
        if (vy1 < vx2) {
          if (vy1 < vx1)      dir =  0; // vy1 < vx1
          else                dir = 31; // vy1 >= vx1 && vy1 < vx2
        } else {                        // vy1 >= vx2 && vy1 < vx4
          if (vy1 < vx3)      dir = 30; // vy1 >= vx2 && vy1 < vx3
          else                dir = 29; // vy1 >= vx3 && vy1 < vx4
        }
      } else { // vy1 >= vx4
        if (vy1 < vx6) {
          if (vy1 < vx5)      dir = 28; // vy1 >= vx4 && vy1 < vx5
          else                dir = 27; // vy1 >= vx5 && vy1 < vx6
        } else {                        // vy1 >= vx6
          if (vy1 < vx7)      dir = 26; // vy1 >= vx6 && vy1 < vx7
          else if (vy1 < vx8) dir = 25; // vy1 >= vx7 && vy1 < vx8
          else                dir = 24; // vy1 >= vx8
        }
      }
    } else {                            // vy < 0
      if (vy1 < vx4) {
        if (vy1 < vx2) {
          if (vy1 < vx1)      dir =  0; // vy1 < vx1
          else                dir =  1; // vy1 >= vx1 && vy1 < vx2
        } else {                        // vy1 >= vx2 && vy1 < vx4
          if (vy1 < vx3)      dir =  2; // vy1 >= vx2 && vy1 < vx3
          else                dir =  3; // vy1 >= vx3 && vy1 < vx4
        }
      } else { // vy1 >= vx4
        if (vy1 < vx6) {
          if (vy1 < vx5)      dir =  4; // vy1 >= vx4 && vy1 < vx5
          else                dir =  5; // vy1 >= vx5 && vy1 < vx6
        } else {                        // vy1 >= vx6
          if (vy1 < vx7)      dir =  6; // vy1 >= vx6 && vy1 < vx7
          else if (vy1 < vx8) dir =  7; // vy1 >= vx7 && vy1 < vx8
          else                dir =  8; // vy1 >= vx8
        }
      }
    }
  } else {                              // vx < 0
    if (vy >= 0) {
      if (vy1 < vx4) {
        if (vy1 < vx2) {
          if (vy1 < vx1)      dir = 16; // vy1 < vx1
          else                dir = 17; // vy1 >= vx1 && vy1 < vx2
        } else {                        // vy1 >= vx2 && vy1 < vx4
          if (vy1 < vx3)      dir = 18; // vy1 >= vx2 && vy1 < vx3
          else                dir = 19; // vy1 >= vx3 && vy1 < vx4
        }
      } else { // vy1 >= vx4
        if (vy1 < vx6) {
          if (vy1 < vx5)      dir = 20; // vy1 >= vx4 && vy1 < vx5
          else                dir = 21; // vy1 >= vx5 && vy1 < vx6
        } else {                        // vy1 >= vx6
          if (vy1 < vx7)      dir = 22; // vy1 >= vx6 && vy1 < vx7
          else if (vy1 < vx8) dir = 23; // vy1 >= vx7 && vy1 < vx8
          else                dir = 24; // vy1 >= vx8
        }
      }
    } else {                            // vy < 0
      if (vy1 < vx4) {
        if (vy1 < vx2) {
          if (vy1 < vx1)      dir = 16; // vy1 < vx1
          else                dir = 15; // vy1 >= vx1 && vy1 < vx2
        } else {                        // vy1 >= vx2 && vy1 < vx4
          if (vy1 < vx3)      dir = 14; // vy1 >= vx2 && vy1 < vx3
          else                dir = 13; // vy1 >= vx3 && vy1 < vx4
        }
      } else { // vy1 >= vx4
        if (vy1 < vx6) {
          if (vy1 < vx5)      dir = 12; // vy1 >= vx4 && vy1 < vx5
          else                dir = 11; // vy1 >= vx5 && vy1 < vx6
        } else {                        // vy1 >= vx6
          if (vy1 < vx7)      dir = 10; // vy1 >= vx6 && vy1 < vx7
          else if (vy1 < vx8) dir =  9; // vy1 >= vx7 && vy1 < vx8
          else                dir =  8; // vy1 >= vx8
        }
      }
    }
  }

  return dir;
}
#endif

int remove_false_minutiae(int width, int height, unsigned char *thinning, int size, minutiae *minutiaes) {
  int k, i, j, n, m, cnt, ret;

  // remove short and build direction
  for(i = 0; i < size; i++) {
    if (minutiaes[i].type==1) {
      mlist.item[0].x = minutiaes[i].x;
      mlist.item[0].y = minutiaes[i].y;
      mlist.item[0].type = 1;
      mlist.size = 1;
      ret = search_end(width, thinning, &mlist);
      if (mlist.item[1].type==1) { // remove short
        minutiaes[i].type = 0;
        k = check_minutiae(size, minutiaes, mlist.item[1]);
        if (k >= 0)
        minutiaes[k].type = 0;
      } else {  // good end point
        minutiaes[i].dir = direction(mlist.item[0].vx[0], mlist.item[0].vy[0]);
        if (ret < 1) minutiaes[i].quality = 100;
        else if (ret < 4) minutiaes[i].quality = 66;
        else minutiaes[i].quality = 33;
      }
    }
  }

  // remove break, do first to avoid spike mis-delete
  for(i = 0; i < size; i++) {
    if (minutiaes[i].type == 1) {
      for(j = i + 1; j < size; j++) {
        if (minutiaes[j].type == 1) {
          int dir;
          int vx = minutiaes[i].x - minutiaes[j].x;
          int vy = minutiaes[i].y - minutiaes[j].y;
          dir = direction(vx, vy);
#if defined (DIR16)
					if ((vx * vx + vy * vy) < BREAK_THRESHOLD &&
						  ABS(minutiaes[i].dir - minutiaes[j].dir) >= 6 &&
						  ABS(minutiaes[i].dir - minutiaes[j].dir) <= 10 &&
						  ABS(dir - minutiaes[j].dir) >= 6 &&
						  ABS(dir - minutiaes[j].dir) <= 10) {
#endif
#if defined (DIR32)
					if ((vx * vx + vy * vy) < BREAK_THRESHOLD &&
						  ABS(minutiaes[i].dir - minutiaes[j].dir) >= 12 &&
						  ABS(minutiaes[i].dir - minutiaes[j].dir) <= 20 &&
						  ABS(dir - minutiaes[j].dir) >= 12 &&
						  ABS(dir - minutiaes[j].dir) <= 20) {
#endif
            minutiaes[i].type = 0;
            minutiaes[j].type = 0;
          }
        }
      }
    }
  }

  // remove ladder, hole and build direction
  for (i = 0; i < size; i++) {
    if (minutiaes[i].type == 2) {
      int dir_i[3];
      mlist.item[0].x = minutiaes[i].x;
      mlist.item[0].y = minutiaes[i].y;
      mlist.item[0].type = 2;
      mlist.size = 1;
      ret = search_fork(width, thinning, &mlist, 1);

      // remove hole
      if ((mlist.item[0].vx[0] == mlist.item[0].vx[1] && mlist.item[0].vy[0] == mlist.item[0].vy[1]) ||
          (mlist.item[0].vx[0] == mlist.item[0].vx[2] && mlist.item[0].vy[0] == mlist.item[0].vy[2]) ||
          (mlist.item[0].vx[1] == mlist.item[0].vx[2] && mlist.item[0].vy[1] == mlist.item[0].vy[2])) {
        minutiaes[i].type = 0;
        for (j = 1; j < mlist.size; j++) {
          if (mlist.item[j].type == 2) {
            if ((mlist.item[j].vx[0] == mlist.item[j].vx[1] && mlist.item[j].vy[0] == mlist.item[j].vy[1]) ||
                (mlist.item[j].vx[0] == mlist.item[j].vx[2] && mlist.item[j].vy[0] == mlist.item[j].vy[2]) ||
                (mlist.item[j].vx[1] == mlist.item[j].vx[2] && mlist.item[j].vy[1] == mlist.item[j].vy[2])) {
              k = check_minutiae(size, minutiaes, mlist.item[j]);
              if (k >= 0)
                minutiaes[k].type = 0;
            }
          }
        }
        continue;
      }

      // remove ladder
      dir_i[0] = direction(mlist.item[0].vx[0], mlist.item[0].vy[0]);
      dir_i[1] = direction(mlist.item[0].vx[1], mlist.item[0].vy[1]);
      dir_i[2] = direction(mlist.item[0].vx[2], mlist.item[0].vy[2]);

      for (j = 1; j < mlist.size; j++) {
        if (mlist.item[j].type == 2 && // find second second fork and is not item[0]
                (mlist.item[j].x != mlist.item[0].x || mlist.item[j].y != mlist.item[0].y)) {
          int dir_j[3], n1, n2, m1, m2;
          for(n = 0; n < 3; n++) {  // find connected vector of item[0]
            if (mlist.item[0].x + mlist.item[0].vx[n] == mlist.item[j].x &&
                        mlist.item[0].y + mlist.item[0].vy[n] == mlist.item[j].y)
              break;
          }
          for(m = 0; m < 3; m++) {  // find connected vector of item[j]
            if (mlist.item[j].x + mlist.item[j].vx[m] == mlist.item[0].x &&
                        mlist.item[j].y + mlist.item[j].vy[m] == mlist.item[0].y)
            break;
          }
          dir_j[0] = direction(mlist.item[j].vx[0], mlist.item[j].vy[0]);
          dir_j[1] = direction(mlist.item[j].vx[1], mlist.item[j].vy[1]);
          dir_j[2] = direction(mlist.item[j].vx[2], mlist.item[j].vy[2]);

          // check the other two vector direction
          n1 = ((n + 1) > 2)? n - 2 : n + 1;
          n2 = ((n + 2) > 2)? n - 1 : n + 2;
          m1 = ((m + 1) > 2)? m - 2 : m + 1;
          m2 = ((m + 2) > 2)? m - 1 : m + 2;

#if defined(DIR16)
					if (dir_diff[dir_i[n1]][dir_i[n2]] >= 6 &&
						dir_diff[dir_j[m1]][dir_j[m2]] >= 6 &&
						(dir_diff[dir_i[n1]][dir_j[m2]] >= 6 || dir_diff[dir_i[n1]][dir_j[m2]] <= 2) &&
						(dir_diff[dir_i[n2]][dir_j[m1]] >= 6 || dir_diff[dir_i[n2]][dir_j[m1]] <= 2)) {
#endif
#if defined(DIR32)
					if (dir_diff[dir_i[n1]][dir_i[n2]] >= 12 &&
						dir_diff[dir_j[m1]][dir_j[m2]] >= 12 &&
						(dir_diff[dir_i[n1]][dir_j[m2]] >= 12 || dir_diff[dir_i[n1]][dir_j[m2]] <= 4) &&
						(dir_diff[dir_i[n2]][dir_j[m1]] >= 12 || dir_diff[dir_i[n2]][dir_j[m1]] <= 4)) {
#endif
            minutiaes[i].type = 0;
            k = check_minutiae(size, minutiaes, mlist.item[j]);
            if (k >= 0)
              minutiaes[k].type = 0;
            continue;
          }
        }
      }

      if (minutiaes[i].type == 2) {// still valid
        if (dir_diff[dir_i[0]][dir_i[1]] <= dir_diff[dir_i[0]][dir_i[2]]) {
          if(dir_diff[dir_i[0]][dir_i[1]]<=dir_diff[dir_i[1]][dir_i[2]])
            minutiaes[i].dir = dir_middle[dir_i[0]][dir_i[1]];
          else
            minutiaes[i].dir = dir_middle[dir_i[1]][dir_i[2]];
        } else {
          if(dir_diff[dir_i[0]][dir_i[2]] <= dir_diff[dir_i[1]][dir_i[2]])
            minutiaes[i].dir = dir_middle[dir_i[0]][dir_i[2]];
          else
            minutiaes[i].dir = dir_middle[dir_i[1]][dir_i[2]];
        }
        if (mlist.size == 4)
          minutiaes[i].quality = 100;
        else
          minutiaes[i].quality = 66;
      }
    }
  }

  // remove spike
  for(i = 0; i < size; i++) {
    if (minutiaes[i].type == 1) {
      mlist.item[0].x = minutiaes[i].x;
      mlist.item[0].y = minutiaes[i].y;
      mlist.item[0].type = 1;
      mlist.size = 1;
      ret = search_end(width, thinning, &mlist);
      if (mlist.item[1].type == 3) { // remove short
        minutiaes[i].type = 0;
        k = check_minutiae(size, minutiaes, mlist.item[1]);
        if (k >= 0)
          minutiaes[k].type = 0;
      }
    }
  }

  return 0;
}


/*

int init_finger_extract() {

  return 0;
}

int finger_extract() {
  unsigned char *capture, *median, *mean, *variance, *border, *dir, *enhdir;
  unsigned char *binary, *smooth, *thinning, *tmp;

  return 0;
}
*/



