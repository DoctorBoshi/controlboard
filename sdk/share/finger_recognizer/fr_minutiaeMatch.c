
#include <math.h>
#include <string.h>
#include <malloc.h>
#include "fr_minutiaeMatch.h"

#pragma hdrstop

#include "sinTable.c"

unsigned char matchNeighbors[MINUTIAE_MAX_NO][MINUTIAE_MAX_NO][MINUTIAE_MAX_NO];

#define DIR_SIMI_TH            16
#define DIST_SIMI_TH           120

#define THITA_DIFF_TH          16
#define RADIUS_TH              10
#define RADIUS2_TH             ((RADIUS_TH << 4) * (RADIUS_TH << 4))

#define MULTIPLE_HIGH_RATIO_1  8
#define MULTIPLE_HIGH_RATIO_2  10
#define RELEASE_COUNT_TH       3
#define ENOUGH_SIMI_COUNT      4
#define LOW_SIMI_TH            20
#define LOW_SIMI_WEIGHT        (int)(65535 * 0.1)
#define HIGH_SIMI_TH           40
#define HIGH_SIMI_WEIGHT       (int)(65535 * 0.2)

#define GLOBAL_RELATION_TH     4

//int getFingerMatchScore(fingerMatchType *finger1, fingerMatchType *finger2) {
int getFingerMatchScore(fingerMatchType *fingerprint1, fingerMatchType *fingerprint2, unsigned char mostMatch[][2]) {
  int i1, i2, i3, i4, val, activeMinutiaeNo1, activeMinutiaeNo2, accuSimi, most1, most2, globalRel;
  int thita1, thita2, thita_diff;
  int neighbor1RelX, neighbor1RelY, neighbor2RelX, neighbor2RelY, deltaX, deltaY, cosT, sinT, dist_2;
  unsigned char matchCount[MINUTIAE_MAX_NO][MINUTIAE_MAX_NO], mostCount[MINUTIAE_MAX_NO][2], matchWeighted[MINUTIAE_MAX_NO][MINUTIAE_MAX_NO];
  minutiaMatchType *selfMinutia, *matchMinutia, *neighborMinutia1, *neighborMinutia2;

  memset(matchCount, 0, sizeof(matchCount));
  memset(matchNeighbors, 0, sizeof(matchNeighbors));
  memset(matchWeighted, 0, sizeof(matchWeighted));

  if (fingerprint1->minutiae_no >  MINUTIAE_MAX_NO) activeMinutiaeNo1 = MINUTIAE_MAX_NO;
  else activeMinutiaeNo1 = fingerprint1->minutiae_no;
  if (fingerprint2->minutiae_no >  MINUTIAE_MAX_NO) activeMinutiaeNo2 = MINUTIAE_MAX_NO;
  else activeMinutiaeNo2 = fingerprint2->minutiae_no;

//  for (i1 = 0; i1 < activeMinutiaeNo1 - 1; i1++) {
  for (i1 = 0; i1 < activeMinutiaeNo1; i1++) {
    selfMinutia = &(fingerprint1->minutiae[i1]);
//    for (i2 = 0; i2 < activeMinutiaeNo2 - 1; i2++) {
    for (i2 = 0; i2 < activeMinutiaeNo2; i2++) {
      matchMinutia = &(fingerprint2->minutiae[i2]);
      val = selfMinutia->thita - matchMinutia->thita;
      if (val < 0) val = -val;
      if (val >= DIR_SIMI_TH && val <= (256 - DIR_SIMI_TH)) continue;
      val = selfMinutia->X - matchMinutia->X;
      if (val >= DIST_SIMI_TH || val <= -DIST_SIMI_TH) continue;
      val = selfMinutia->Y - matchMinutia->Y;
      if (val >= DIST_SIMI_TH || val <= -DIST_SIMI_TH) continue;
//      for (i3 = i1 + 1; i3 < activeMinutiaeNo1; i3++) {
      for (i3 = 0; i3 < activeMinutiaeNo1; i3++) {
        if (i1 == i3) continue;
        neighborMinutia1 = &(fingerprint1->minutiae[i3]);
        thita1 = selfMinutia->thita - neighborMinutia1->thita;
//        for (i4 = i2 + 1; i4 < activeMinutiaeNo2; i4++) {
        for (i4 = 0; i4 < activeMinutiaeNo2; i4++) {
          if (i2 == i4) continue;
          neighborMinutia2 = &(fingerprint2->minutiae[i4]);
          thita2 = matchMinutia->thita - neighborMinutia2->thita;
          thita_diff = thita1 - thita2;
          if (thita_diff > 128) {
            if (thita_diff > 384) thita_diff -= 512;
            else thita_diff -= 256;
          } else if (thita_diff < -128) {
            if (thita_diff < -384) thita_diff += 512;
            else thita_diff += 256;
          }
          if (thita_diff > THITA_DIFF_TH || thita_diff < -THITA_DIFF_TH) continue;

          // match position
          neighbor1RelX = neighborMinutia1->X - selfMinutia->X;
          neighbor1RelY = neighborMinutia1->Y - selfMinutia->Y;

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
          if (dist_2 > RADIUS2_TH) continue;

          matchCount[i1][i2]++;
//          matchCount[i3][i4]++;
          matchNeighbors[i1][i2][matchNeighbors[i1][i2][0] + 1] = i3;
          matchNeighbors[i1][i2][0]++;
//          matchNeighbors[i3][i4][matchNeighbors[i3][i4][0] + 1] = i1;
//          matchNeighbors[i3][i4][0]++;
        }
      }
    }
  }

  // two-directional erase multiple high match count
  // 1. do the erase for finger2 and store to matchWeighted
  // 2. do the eaase for finger1 and store to matchCount
  // 3. merge the two matrix with lower count and store to matchCount
  // 1.
  for (i2 = 0; i2 < activeMinutiaeNo2; i2++) {
    most1 = most2 = 0;
    for (i1 = 0; i1 < activeMinutiaeNo1; i1++) {
      if (matchCount[i1][i2] > most1) {
        most2 = most1;
        most1 = matchCount[i1][i2];
      } else if (matchCount[i1][i2] > most2)
        most2 = matchCount[i1][i2];
    }
    if (most2 * MULTIPLE_HIGH_RATIO_2 >= most1 * MULTIPLE_HIGH_RATIO_1)
      for (i1 = 0; i1 < activeMinutiaeNo1; i1++)
        matchWeighted[i1][i2] = 0;
      else
        for (i1 = 0; i1 < activeMinutiaeNo1; i1++)
          matchWeighted[i1][i2] = matchCount[i1][i2];
    }
  // 2.
  for (i1 = 0; i1 < activeMinutiaeNo1; i1++) {
    most1 = most2 = 0;
    for (i2 = 0; i2 < activeMinutiaeNo2; i2++) {
      if (matchCount[i1][i2] > most1) {
        most2 = most1;
        most1 = matchCount[i1][i2];
      } else if (matchCount[i1][i2] > most2)
        most2 = matchCount[i1][i2];
    }
    if (most2 * MULTIPLE_HIGH_RATIO_2 >= most1 * MULTIPLE_HIGH_RATIO_1)
      for (i2 = 0; i2 < activeMinutiaeNo2; i2++)
        matchCount[i1][i2] = 0;
  }
  // 3.
  for (i1 = 0; i1 < activeMinutiaeNo1; i1++)
    for (i2 = 0; i2 < activeMinutiaeNo2; i2++)
      if (matchWeighted[i1][i2] < matchCount[i1][i2])
        matchCount[i1][i2] = matchWeighted[i1][i2];

  for (i1 = 0; i1 < activeMinutiaeNo1; i1++) {
    most1 = 0;
    for (i2 = 0; i2 < activeMinutiaeNo2; i2++)
      if (matchCount[i1][i2] > most1) most1 = matchCount[i1][i2];
    mostCount[i1][0] = most1;
  }

  // release match count without high relation
  // 1. do 1st release from matchCount and store to matchWeighted
  // 2. do 2nd release with matchWeighted and store to matchCount
  // 1.
  for (i1 = 0; i1 < activeMinutiaeNo1; i1++)
    for (i2 = 0; i2 < activeMinutiaeNo2; i2++) {
      if ((most1 = matchCount[i1][i2]) > 0)
        for (i3 = 1; i3 < matchNeighbors[i1][i2][0] + 1; i3++) {
          if (mostCount[matchNeighbors[i1][i2][i3]][0] <= RELEASE_COUNT_TH)
            most1--;
        }
      matchWeighted[i1][i2] = most1;
    }
  for (i1 = 0; i1 < activeMinutiaeNo1; i1++) {
    most1 = 0;
    for (i2 = 0; i2 < activeMinutiaeNo2; i2++)
      if (matchWeighted[i1][i2] > most1) most1 = matchWeighted[i1][i2];
    mostCount[i1][0] = most1;
  }
  // 2.
  for (i1 = 0; i1 < activeMinutiaeNo1; i1++)
    for (i2 = 0; i2 < activeMinutiaeNo2; i2++) {
      if ((most1 = matchCount[i1][i2]) > 0)
        for (i3 = 1; i3 < matchNeighbors[i1][i2][0] + 1; i3++) {
          if (mostCount[matchNeighbors[i1][i2][i3]][0] <= RELEASE_COUNT_TH)
            most1--;
        }
      matchCount[i1][i2] = most1;
    }

  for (i1 = 0; i1 < activeMinutiaeNo1; i1++) {
    most1 = 0;
    for (i2 = 0; i2 < activeMinutiaeNo2; i2++)
      if (matchCount[i1][i2] > most1) most1 = matchCount[i1][i2];
    mostCount[i1][0] = most1;
  }

  // distribute relation to neighbors
  for (i1 = 0; i1 < activeMinutiaeNo1; i1++)
    for (i2 = 0; i2 < activeMinutiaeNo2; i2++) {
      if (matchCount[i1][i2] < ENOUGH_SIMI_COUNT) continue;

      most1 = 0;
      for (i3 = 1; i3 < matchNeighbors[i1][i2][0] + 1; i3++)
        most1 += mostCount[matchNeighbors[i1][i2][i3]][0];
      if (most1 > 255)
        matchWeighted[i1][i2] = 255;
      else
        matchWeighted[i1][i2] = most1;
    }

  for (i1 = 0; i1 < activeMinutiaeNo1; i1++) {
    most1 = most2 = 0;
    for (i2 = 0; i2 < activeMinutiaeNo2; i2++)
      if (matchWeighted[i1][i2] > most1) {
        most1 = matchWeighted[i1][i2];
        most2 = i2;
      }
    mostCount[i1][0] = most1;
    mostCount[i1][1] = most2;
    if (mostMatch != NULL) {
      mostMatch[i1][0] = most1;
      mostMatch[i1][1] = most2;
    }
  }

  // use matchCount[][0] to store the high weighted minutiae index in fingerprint1
  // use matchCount[][1] to store the matched minutiae index in fingerprint2
  // use matchCount[][2] to store the weighted value
  // use matchcount[][3] to store the global relation
  i4 = 0;
  for (i1 = 0; i1 < activeMinutiaeNo1; i1++) {
    if (mostCount[i1][0] > LOW_SIMI_TH) {
      matchCount[i4][0] = i1;
      matchCount[i4][1] = mostCount[i1][1];
      matchCount[i4][2] = mostCount[i1][0];
      matchCount[i4][3] = 0;
      i4++;
    }
  }

  // chech correctness of global relation
  // use matchCount[][2] to store the global relation
  for (i1 = 0; i1 < i4 - 1; i1++) { // loop for selfMinutia and the corresponging matchMinutia
    if (matchCount[i1][3] >= GLOBAL_RELATION_TH) continue;  // correct global relation is enough
    selfMinutia = &(fingerprint1->minutiae[matchCount[i1][0]]);
    matchMinutia = &(fingerprint2->minutiae[matchCount[i1][1]]);
    for (i2 = i1 + 1; i2 < i4; i2++) { // loop for neighborMinutia in fingerprint1 and corresponding in fingerprint2
      neighborMinutia1 = &(fingerprint1->minutiae[matchCount[i2][0]]);
      neighbor1RelX = neighborMinutia1->X - selfMinutia->X;
      neighbor1RelY = neighborMinutia1->Y - selfMinutia->Y;
      neighborMinutia2 = &(fingerprint2->minutiae[matchCount[i2][1]]);
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
        matchCount[i1][3]++;
        matchCount[i2][3]++;
      }
    }
  }

  accuSimi = 0;
  for (i1 = 0; i1 < i4; i1++) {
    if (matchCount[i1][3] >= GLOBAL_RELATION_TH) {
      if (matchCount[i1][2] > HIGH_SIMI_TH)
        accuSimi += (HIGH_SIMI_WEIGHT * (65536 - accuSimi) + 32768) >> 16;
      else // if (matchCount[i1][2] > LOW_SIMI_TH)
        accuSimi += (LOW_SIMI_WEIGHT * (65536 - accuSimi) + 32768) >> 16;
    }
  }

  return (accuSimi * 10000 + 32768) >> 16;
}

