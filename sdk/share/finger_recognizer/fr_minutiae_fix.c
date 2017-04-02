#include <string.h>
#include <malloc.h>
#include <stdlib.h>

#include "fr_minutiae_fix.h"

#pragma hdrstop

#include "oneCountTable.c"
#include "circle.c"
#include "sinTable.c"
#include "sqrtTable.c"
#include "gaussian.c"

UINT16 tempMCC[MCC_SECTION][TEMP_MCC_WIDTH_CELL][TEMP_MCC_WIDTH_CELL];

//---------------------------------------------------------------------------
int sort_function(const void *d1, const void *d2) {
  UINT32 v1, v2;

  v1 = *((UINT32 *)d1);
  v2 = *((UINT32 *)d2);

  if (v1 < v2) return 1;
  else if (v1 > v2) return -1;
  else return 0;
}

void createMCC(fingerType *finger, int minutiaIndex, tempFingerMCCType *fingerMCC) {
  int i1, myX, myY, otherX, otherY, otherRelXFix, otherRelYFix, otherIntX, otherIntY, otherXOff, otherYOff;
  int section, section2, sectionOff, mapSection;
  int x, y, s, strength, neighborNum;
  int cosT, sinT;
  int otherRelX, otherRelY, otherThita, diffThita;
  minutiaType *thisMinutia;
#if defined(MCC_BINARY)
  int acc_value = 0;
  UINT8 *paddedByte;
#endif

  thisMinutia = &(finger->minutiae[minutiaIndex]);
  myX = thisMinutia->X;
  myY = thisMinutia->Y;
  sinT = sinTable[thisMinutia->thita];
  i1 = thisMinutia->thita + 64;
  if (i1 >= 256) i1 -= 256;
  cosT = sinTable[i1];

  neighborNum = 0;
  memset(tempMCC, 0, sizeof(tempMCC));
  for (i1 = 0; i1 < finger->minutiae_no; i1++) {
    if (i1 != minutiaIndex) {
      otherX = finger->minutiae[i1].X;
      otherY = finger->minutiae[i1].Y;
      otherRelX = ((otherX - myX) * cosT + (otherY - myY) * -sinT + 16384) >> 15;
      otherRelY = ((otherX - myX) * sinT + (otherY - myY) * cosT + 16384) >> 15;
      if (otherRelX * otherRelX + otherRelY * otherRelY < NEIGHBOR_RING_2) neighborNum++;
//      if (abs(otherRelX) * PIXEL_CELL_MUL <= NEIGHBOR_RANGE_CELL &&
//                abs(otherRelY) * PIXEL_CELL_MUL <= NEIGHBOR_RANGE_CELL) {
      if (abs(otherRelX) * MCC_WIDTH_CELL / 2 <= NEIGHBOR_RANGE_PIXEL &&
                abs(otherRelY) * MCC_WIDTH_CELL / 2 <= NEIGHBOR_RANGE_PIXEL) {
        // near minutia
        otherThita = finger->minutiae[i1].thita;
        diffThita = otherThita - thisMinutia->thita;
        if (diffThita < -128) diffThita += 256;
        else if (diffThita >= 128) diffThita -= 256;
        // -pi~pi -> 0~2pi
        diffThita += 128;

        // mapping otherNewX, otherNewY & diffThita to get the corresponding gaussian funtion in gInfluence[][][]
        if (otherRelX >= 0) {
//          otherRelXFix = (int)(otherRelX * PIXEL_CELL_MUL * 2 + 0.5);
          otherRelXFix = (otherRelX * PIXEL_CELL_MUL + 32768) >> 16;
          otherIntX = (otherRelXFix >> 1);
        } else {
//          otherRelXFix = (int)(otherRelX * PIXEL_CELL_MUL * 2 - 0.5);
          otherRelXFix = (otherRelX * PIXEL_CELL_MUL + 32768) >> 16;
          otherIntX = ((otherRelXFix - 1) >> 1);
        }
        otherXOff = otherRelXFix & 1;

        if (otherRelY >= 0) {
//          otherRelYFix = (int)(otherRelY * PIXEL_CELL_MUL * 2 + 0.5);
          otherRelYFix = (otherRelY * PIXEL_CELL_MUL + 32768) >> 16;
          otherIntY = (otherRelYFix >> 1);
        } else {
//          otherRelYFix = (int)(otherRelY * PIXEL_CELL_MUL * 2 - 0.5);
          otherRelYFix = (otherRelY * PIXEL_CELL_MUL + 32768) >> 16;
          otherIntY = ((otherRelYFix - 1) >> 1);
        }
        otherYOff = otherRelYFix & 1;

//        section2 = diffThita * MCC_SECTION / M_PI;
//        section2 = (diffThita * MCC_SECTION + 64) / 128;
        section2 = diffThita * MCC_SECTION / 128;
        section = (section2 >> 1);
        sectionOff = section2 & 1;

        // merge the minutia influence to tempMCC
        for (s = 0; s < INFLUENCE_SECTION; s++)
          for (y = 0; y < INFLUENCE_WIDTH_CELL; y++)
            for (x = 0; x < INFLUENCE_WIDTH_CELL; x++) {
              mapSection = section + s - INFLUENCE_SECTION / 2;
              if (mapSection < 0) mapSection += MCC_SECTION;
              else if (mapSection >= MCC_SECTION) mapSection -= MCC_SECTION;
              tempMCC[mapSection][TEMP_MCC_WIDTH_CELL / 2 + otherIntY - INFLUENCE_WIDTH_CELL / 2 + y][TEMP_MCC_WIDTH_CELL / 2 + otherIntX - INFLUENCE_WIDTH_CELL / 2 + x] +=
                        influence[sectionOff][otherYOff][otherXOff][s][y][x];
            }
      }
    }
  }
// debug
#if 0
  {
    FILE *outf;

    outf = fopen("tempMCCtest.txt", "w");

    for (s = 0; s < MCC_SECTION; s++) {
      for (y = 0; y < TEMP_MCC_WIDTH_CELL; y++) {
        for (x = 0; x < TEMP_MCC_WIDTH_CELL; x++) {
          if (x == TEMP_MCC_WIDTH_CELL / 2 && y == TEMP_MCC_WIDTH_CELL / 2)
            fprintf(outf, "%4d ", tempMCC[s][y][x] + 9000);
          else fprintf(outf, "%4d ", tempMCC[s][y][x]);
        } // end of for (ss = 0; ss < INFLUENCE_SECTION; ss++) {
        fprintf(outf, "\n");
      } // end of for (x = 0; x < 2; x++) {
      fprintf(outf, "\n");
    } // end of for (y = 0; y < 2; y++) {
    fclose(outf);
  }

  exit(-1);
#endif

  // copy to minutia's MCC
  strength = 0;
  paddedByte = &(fingerMCC->MCCs[minutiaIndex].MCC[0]);
  for (s = 0; s < MCC_SECTION; s++)
    for (y = 0; y < MCC_WIDTH_CELL; y++) {
      for (x = 0; x < MCC_WIDTH_CELL; x++) {
        int value, value_4, mvalue;

        value = tempMCC[s][INFLUENCE_WIDTH_CELL + y][INFLUENCE_WIDTH_CELL + x] / 4;
        if (value > 255) value = 255;
        mvalue = sqrtTable[value];

        if (mvalue > BINARY_THRESHOLD) mvalue = 1;
        else mvalue = 0;
        strength += mvalue;
        acc_value <<= 1;
        acc_value |= mvalue;
        if (((x + 1) & 7) == 0) {
          *paddedByte = acc_value;
          acc_value = 0;
          paddedByte++;
        }
      }
    }
  fingerMCC->MCCs[minutiaIndex].strength = strength;
  if (neighborNum < ENOUGH_NEIGHBOR_NUM) fingerMCC->MCCs[minutiaIndex].weight = neighborNum;
  else fingerMCC->MCCs[minutiaIndex].weight = ENOUGH_NEIGHBOR_NUM;
#ifdef INACTIVE_ISOLATED_MINUTIAE
  if (neighborNum < ENOUGH_NEIGHBOR_NUM) fingerMCC->active[minutiaIndex] = 0;
  else fingerMCC->active[minutiaIndex] = 1;
#else
  fingerMCC->active[minutiaIndex] = 1;
#endif
}


void createMCCs(fingerType *finger, fingerMCCType *fingerMCC) {
  int i1;
  tempFingerMCCType tempFingerMCC;

  for (i1 = 0; i1 < finger->minutiae_no; i1++)
    createMCC(finger, i1, &tempFingerMCC);

  // delete weak minutiae
#if 1
  {
    int i2, minutiaIndex;
    UINT32 minutiaeList[256];

/*    maxStrength = 0;
    for (i1 = 0; i1 < finger->minutiae_no; i1++)
      if (tempFingerMCC.MCCs[i1].strength > maxStrength)
        maxStrength = tempFingerMCC.MCCs[i1].strength;
    maxStrength++;*/

    for (i1 = 0; i1 < finger->minutiae_no; i1++) {
      minutiaeList[i1] = i1 + tempFingerMCC.MCCs[i1].strength * 256;
      tempFingerMCC.active[i1] = 0;
    }
    qsort(minutiaeList, finger->minutiae_no, sizeof(minutiaeList[0]), sort_function);

    for (i1 = 0; i1 < finger->minutiae_no; i1++) {
      tempFingerMCC.active[minutiaeList[i1] & 0x0FF] = 1;
      if (i1 >= ACTIVE_MCC_NUM - 1) break;
    }

    minutiaIndex = 0;
    for (i1 = 0; i1 < finger->minutiae_no; i1++) {
      if (tempFingerMCC.active[i1] == 1) {
        for (i2 = 0; i2 < MCC_TOTAL_BYTE; i2++)
          fingerMCC->MCCs[minutiaIndex].MCC[i2] = tempFingerMCC.MCCs[i1].MCC[i2];

        fingerMCC->MCCs[minutiaIndex].strength = tempFingerMCC.MCCs[i1].strength;
        fingerMCC->MCCs[minutiaIndex].weight = tempFingerMCC.MCCs[i1].weight;

        fingerMCC->MCCs[minutiaIndex].X = finger->minutiae[i1].X;
        fingerMCC->MCCs[minutiaIndex].Y = finger->minutiae[i1].Y;
        fingerMCC->MCCs[minutiaIndex].thita = finger->minutiae[i1].thita;

        minutiaIndex++;
        if (minutiaIndex >= ACTIVE_MCC_NUM) break;
      }
    }
    if (finger->minutiae_no < ACTIVE_MCC_NUM) fingerMCC->activeNum = finger->minutiae_no;
    else fingerMCC->activeNum = ACTIVE_MCC_NUM;

  }

#endif
}

UINT32 getMCCSimi(MCCType *MCC1, MCCType *MCC2) {
  int i1;
  UINT16 strength1, strength2, strength, weight, value, dot;
  UINT32 simi;

  UINT8 *paddedByte1 = &(MCC1->MCC[0]);
  UINT8 *paddedByte2 = &(MCC2->MCC[0]);


  dot = strength = 0;
  strength1 = MCC1->strength;
  strength2 = MCC2->strength;
  if (strength1 * 3 < strength2 || strength2 * 3 < strength1) return 0;
  strength = strength1 + strength2;

  weight = MCC1->weight;
  if (MCC2->weight < weight)
    weight = MCC2->weight;
  for (i1 = 0; i1 < MCC_TOTAL_BYTE; i1++) {
    value = *paddedByte1 & *paddedByte2;
    dot += oneCountTable[value];
    paddedByte1++;
    paddedByte2++;
  }

  if (strength == 0) simi = 0;
  else simi = (dot << (12 + 1)) * weight / (strength << 2);

  return simi;
}

#define TOTAL_SIMILIST_COUNT 8
#define DIR_SIMI_TH            16
#define DIST_SIMI_TH           120
#define RADIUS_TH              10
#define RADIUS2_TH             ((RADIUS_TH << 4) * (RADIUS_TH << 4))
UINT32 getFingerSimi(fingerMCCType *finger1, fingerMCCType *finger2) {
  int i1, i2, i3, i4, listFillNum, val, matchCount[8], minutiaeNo[2][8];
  UINT32 MCCSimi, simiList[8], fingerSimi;
  int neighbor1RelX, neighbor1RelY, neighbor2RelX, neighbor2RelY, thita_diff, deltaX, deltaY, cosT, sinT, dist_2;
  MCCType *selfMinutia, *matchMinutia, *neighborMinutia1, *neighborMinutia2;

  listFillNum = 1;
  for (i1 = 0; i1 < TOTAL_SIMILIST_COUNT; i1++) simiList[i1] = 0;
//  simiList[0] = 0;
  for (i1 = 0; i1 < finger1->activeNum; i1++) {
    selfMinutia = &(finger1->MCCs[i1]);
    for (i2 = 0; i2 < finger2->activeNum; i2++) {
      matchMinutia = &(finger2->MCCs[i2]);

      val = selfMinutia->thita - matchMinutia->thita;
      if (val < 0) val = -val;
      if (val >= DIR_SIMI_TH && val <= (256 - DIR_SIMI_TH)) continue;
      val = selfMinutia->X - matchMinutia->X;
      if (val >= DIST_SIMI_TH || val <= -DIST_SIMI_TH) continue;
      val = selfMinutia->Y - matchMinutia->Y;
      if (val >= DIST_SIMI_TH || val <= -DIST_SIMI_TH) continue;

//      MCCSimi = getMCCSimi(&(finger1->MCCs[i1]), &(finger2->MCCs[i2]));
      MCCSimi = (getMCCSimi(&(finger1->MCCs[i1]), &(finger2->MCCs[i2])) << 16) | (i1 << 8) | i2;
      if (MCCSimi > simiList[listFillNum - 1]) {
        if (listFillNum > 1) {
          for (i3 = listFillNum; i3 > 0; i3--)
            if (MCCSimi < simiList[i3 - 1]) break;
        } else i3 = 0;
        if (i3 < TOTAL_SIMILIST_COUNT - 1)
          for (i4 = listFillNum - 1; i4 >= i3; i4--)
            simiList[i4 + 1] = simiList[i4];
        simiList[i3] = MCCSimi;
        if (listFillNum < 7) listFillNum++;
      }
    }
  }

  for (i1 = 0; i1 < TOTAL_SIMILIST_COUNT; i1++) {
    matchCount[i1] = 0;
    minutiaeNo[0][i1] = (simiList[i1] >> 8) & 0x0FF;
    minutiaeNo[1][i1] = simiList[i1] & 0x0FF;
  }
  for (i1 = 0; i1 < TOTAL_SIMILIST_COUNT - 1; i1++) {
    selfMinutia = &(finger1->MCCs[minutiaeNo[0][i1]]);
    matchMinutia = &(finger2->MCCs[minutiaeNo[1][i1]]);
    for (i2 = i1 + 1; i2 < TOTAL_SIMILIST_COUNT; i2++) {
      neighborMinutia1 = &(finger1->MCCs[minutiaeNo[0][i2]]);
      neighbor1RelX = neighborMinutia1->X - selfMinutia->X;
      neighbor1RelY = neighborMinutia1->Y - selfMinutia->Y;
      neighborMinutia2 = &(finger2->MCCs[minutiaeNo[1][i2]]);
      thita_diff = matchMinutia->thita - selfMinutia->thita;
      if (thita_diff < 0) thita_diff += 256;
      sinT = sinTable[thita_diff];
      thita_diff += 64;
      if (thita_diff >= 256) thita_diff -= 256;
      cosT = sinTable[thita_diff];
      neighbor2RelX = ((neighborMinutia2->X - matchMinutia->X) * cosT + (neighborMinutia2->Y - matchMinutia->Y) * -sinT + 1024) >> 11;
      neighbor2RelY = ((neighborMinutia2->X - matchMinutia->X) * sinT + (neighborMinutia2->Y - matchMinutia->Y) * cosT + 1024) >> 11;
      deltaX = (neighbor1RelX << 4) - neighbor2RelX;
      deltaY = (neighbor1RelY << 4) - neighbor2RelY;
      dist_2 = deltaX * deltaX + deltaY * deltaY;
      if (dist_2 <= RADIUS2_TH) {
        matchCount[i1]++;
        matchCount[i2]++;
      }
    }
  }

  for (i1 = 0; i1 < TOTAL_SIMILIST_COUNT; i1++) {
    static int matchWeight[TOTAL_SIMILIST_COUNT] = {0.1 * 256, 0.3 * 256, 0.8 * 256, 1.0 * 256, 1.1 * 256, 1.2 * 256, 1.4 * 256, 1.6 * 256};
    i2 = simiList[i1] >> 16;
    i2 = (i2 * matchWeight[matchCount[i1]] + 128) >> 8;
    if (i2 >= 4096) i2 = 4096;
    simiList[i1] = i2;
  }

  fingerSimi = 0;
//  for (i1 = 0; i1 < listFillNum; i1++) FingerSimi += simiList[i1];
  for (i1 = 0; i1 < TOTAL_SIMILIST_COUNT; i1++) fingerSimi += simiList[i1];

  fingerSimi = (fingerSimi * 10000) >> (12 + 3);
  return fingerSimi;
}


void getFingerSimiArray(fingerMCCType *finger1, fingerMCCType *finger2, UINT32 simiIndexList[8]) {
  int i1, i2, i3, i4, listFillNum;
  UINT32 MCCSimi, simiList[8] = { 0 }, fingerSimi;

  listFillNum = 1;
  for (i1 = 0; i1 < 8; i1++) simiList[i1] = 0;
//  simiList[0] = 0;
  for (i1 = 0; i1 < finger1->activeNum; i1++)
    for (i2 = 0; i2 < finger2->activeNum; i2++) {
      MCCSimi = getMCCSimi(&(finger1->MCCs[i1]), &(finger2->MCCs[i2]));
      if (MCCSimi > simiList[listFillNum - 1]) {
        if (listFillNum > 1) {
          for (i3 = listFillNum; i3 > 0; i3--)
            if (MCCSimi < simiList[i3 - 1]) break;
        } else i3 = 0;
        if (i3 < 7)
          for (i4 = listFillNum - 1; i4 >= i3; i4--)
            simiList[i4 + 1] = simiList[i4];
        simiList[i3] = MCCSimi;
        simiIndexList[i3] = i1;
        if (listFillNum < 7) listFillNum++;
      }
    }

  //fingerSimi = 0;
//  for (i1 = 0; i1 < listFillNum; i1++) FingerSimi += simiList[i1];
  //for (i1 = 0; i1 < 8; i1++) fingerSimi += simiList[i1];

  //fingerSimi = (fingerSimi * 10000) >> (12 + 3);
  //return fingerSimi;
}




