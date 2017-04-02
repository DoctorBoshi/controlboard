#ifndef _FR_MINUTIAEMATCH_H_
#define _FR_MINUTIAEMATCH_H_

#include "fr_match_type.h"

int getMinutiaMatchNo(fingerMatchType *finger1, int index1, fingerMatchType *finger2, int index2, unsigned char neighborList[]);
//int getFingerMatchScore(fingerMatchType *finger1, fingerMatchType *finger2);
int getFingerMatchScore(fingerMatchType *finger1, fingerMatchType *finger2, unsigned char mostMatch[][2]);


#endif

