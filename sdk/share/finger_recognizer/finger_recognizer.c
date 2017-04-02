#include "ite/itp.h"
#include "string.h"
#include "stdlib.h"
#include "malloc.h"
#include "finger_extract.h"
#include "finger_recognizer.h"

//=============================================================================
//                Constant Definition
//=============================================================================

//=============================================================================
//                Macro Definition
//=============================================================================

//Border definition
#define B_TOP    1
#define B_BOTTOM 1
#define B_LEFT   2
#define B_RIGHT  2
#define CORNER   6

//=============================================================================
//                Structure Definition
//=============================================================================
typedef struct FingerRecognizerConfig
{
    int             simiStepH;
    int             simiStepV;
    int             simiNumH;
    int             simiNumV;
    int             halfSimiNumH;
    int             halfSimiNumV;
    int             simiLeft;
    int             simiTop;
    int             width;
    int             height;
    int             srcRegionLeft;
    int             srcRegionTop;
    int             srcRegionRight;
    int             srcRegionBottom;
    int             activeThreshold;
    int             simiThreshold;
} FingerRecognizerConfig;

//=============================================================================
//                Global Data Definition
//=============================================================================

#define SRC_REGION_LEFT    50
#define SRC_REGION_TOP     30
#define SRC_REGION_RIGHT   290
#define SRC_REGION_BOTTOM  210


static FingerRecognizerConfig gFingerConfig = {
    8, 8, 20, 15, 10, 7, 84, 64, 320, 240, SRC_REGION_LEFT, SRC_REGION_TOP, SRC_REGION_RIGHT, SRC_REGION_BOTTOM, 250, 440, 
};

static int gppPrevValue[64][64] = { 0 };

//=============================================================================
//                Private Function Definition
//=============================================================================

//=============================================================================
//                Public Function Definition
//=============================================================================

//=============================================================================
/**
 * Initialize finger recognizer
 *
 * @param recognizerSetup   finger recognizer setup. if the input is null, use
 *                          default setup.
 * @return              none
 */
//=============================================================================
void
fingerRecognizerInit(
    FingerRecognizerSetup *recognizerSetup)
{
    int i = 0;
    int *pData = NULL;
  
    if (recognizerSetup)
    {
        if (recognizerSetup->fingerWidth <= 0 || recognizerSetup->fingerHeight <= 0
         || recognizerSetup->simiStepH <= 0 || recognizerSetup->simiStepV <= 0
         || recognizerSetup->activeThreshold <= 0 || recognizerSetup->simiThreshold <= 0)
        {
#if 0
            printf("fingerWidth: %d, fingerHeight: %d, simiStepH: %d, simiStepV: %d, activeThreshold: %d, simiThreshold: %d\n",
                   recognizerSetup->fingerWidth, recognizerSetup->fingerHeight,
                   recognizerSetup->simiStepH, recognizerSetup->simiStepV,
                   recognizerSetup->activeThreshold, recognizerSetup->simiThreshold);
            printf("Invalid setup input. value <=0 is not allowed.\n");
#endif
            return;
        }

        gFingerConfig.width = recognizerSetup->fingerWidth;
        gFingerConfig.height = recognizerSetup->fingerHeight;
        gFingerConfig.simiStepH = recognizerSetup->simiStepH;
        gFingerConfig.simiStepV = recognizerSetup->simiStepV;
        gFingerConfig.simiNumH = gFingerConfig.width / 2 / recognizerSetup->simiStepH;
        gFingerConfig.simiNumV = gFingerConfig.height / 2 / recognizerSetup->simiStepV;
        gFingerConfig.halfSimiNumH = gFingerConfig.simiNumH / 2;
        gFingerConfig.halfSimiNumV = gFingerConfig.simiNumV / 2;
        gFingerConfig.simiLeft = gFingerConfig.width / 4 + recognizerSetup->simiStepH / 2;
        gFingerConfig.simiTop = gFingerConfig.height / 4 + recognizerSetup->simiStepV / 2;
        gFingerConfig.activeThreshold = recognizerSetup->activeThreshold;
        gFingerConfig.simiThreshold = recognizerSetup->simiThreshold;
        gFingerConfig.srcRegionLeft = gFingerConfig.width / 8;
        gFingerConfig.srcRegionTop =  gFingerConfig.height / 8;
        gFingerConfig.srcRegionRight = gFingerConfig.width * 7 / 8;
        gFingerConfig.srcRegionBottom = gFingerConfig.height *  7 / 8;
    }

#if 0
    printf("w: %d, h: %d, sh: %d, sv: %d, snh: %d, snv: %d, sl: %d, st: %d, at: %d, st: %d\n",
           gFingerConfig.width, gFingerConfig.height,
           gFingerConfig.simiStepH, gFingerConfig.simiStepV,
           gFingerConfig.simiNumH, gFingerConfig.simiNumV,
           gFingerConfig.simiLeft, gFingerConfig.simiTop,
           gFingerConfig.activeThreshold, gFingerConfig.simiThreshold);
#endif
}

//=============================================================================
/**
 * Check if the input finger data is stable or not.
 * @param fingerImage   finger sensor Y data and area information.
 * @return              true - stable finger image, false - not stable.
 */
//=============================================================================
bool
isStableFingerPrint(
    FingerImage *fingerImage)
{
    static bool bFirstFrame = true, bHasFinger = false;
    static int prevAct = 0;
    int i, v, h, partAct[4], partNum, simi = 0, act = 0;
    int hh, vv, diff, diff_h, diff_v, count = 0, pitch = fingerImage->pitchY;
    int step_h, step_v, idx_v, idx_h, sum;
    unsigned char *capData = fingerImage->fingerData;

#if 0
    static struct timeval startTime = { 0 };
    struct timeval curTime = { 0 };

    if (bFirstFrame)
    {
        gettimeofday(&startTime, NULL);
    }
#endif

    for (v = gFingerConfig.simiTop, idx_v = 0; idx_v < gFingerConfig.simiNumV; v += gFingerConfig.simiStepV, idx_v++)
    {
        for (h = gFingerConfig.simiLeft, idx_h = 0; idx_h < gFingerConfig.simiNumH; h += gFingerConfig.simiStepH, idx_h++)
        {
            sum = 0;
            for (vv = v - 1; vv <= v + 2; vv++)
            {
                for (hh = h - 1; hh <= h + 2; hh++)
                {
                    sum += *(capData + vv * pitch + hh);
                }
            }
            sum = (sum + 8) >> 4;
            diff = abs(sum - gppPrevValue[idx_v][idx_h]);

            if (bFirstFrame)
            {
                simi += 255;
            }
            else
            {
                simi += diff;
            }
            gppPrevValue[idx_v][idx_h] = sum;

		    diff_h = *(capData + v * pitch + h) - *(capData + v * pitch + h + 4);
		    act += abs(diff_h);
		    diff_v = *(capData + v * pitch + h) - *(capData + (v + 4) * pitch + h);
		    act += abs(diff_v);
            count++;

            if (idx_h < gFingerConfig.halfSimiNumH) // left jalf part
            {
                if (idx_v < gFingerConfig.halfSimiNumV)
                {
                    partNum = 0; // top half part
                }
                else
                {
                    partNum = 2; // bottom half part
                }
            }
            else // right half part
            {
                if (idx_v < gFingerConfig.halfSimiNumV)
                {
                    partNum = 1; // top half part
                }
                else
                {
                    partNum = 3; // bottom half part
                }
            }
            partAct[partNum] += abs(diff_h);
            partAct[partNum] += abs(diff_v);
        }
    }

    if (bFirstFrame)
    {
        bFirstFrame = 0;
    }

    //act = (act * 16 + count / 2) / count;
    act = ((act << 4) + (count >> 1)) / count;
    for (i = 0; i < 4; i++)
    {
        //partAct[i] = (partAct[i] * 64 + count / 2) / count;
        partAct[i] = ((partAct[i] << 6) + (count >> 1)) / count;
    }

    //printf("act:%d, simi: %d, hasFinger: %d\n", act, simi, bHasFinger);
    if (bHasFinger == false)
    {
        if (simi > 0 && simi < gFingerConfig.simiThreshold && act > gFingerConfig.activeThreshold
         && partAct[0] > gFingerConfig.activeThreshold && partAct[1] > gFingerConfig.activeThreshold
         && partAct[2] > gFingerConfig.activeThreshold && partAct[3] > gFingerConfig.activeThreshold)
        {
            bHasFinger = true;
        }
    }
    else
    {
        if (act <= gFingerConfig.activeThreshold || simi >= gFingerConfig.simiThreshold)
        {
            bHasFinger = false;
        }
        else //if (act < prevAct) //get stable fingerprint
        {
            bHasFinger = false;
            bFirstFrame = true;
            prevAct = 0;
#if 0
            gettimeofday(&curTime, NULL);
            printf("get good finger time: %d ms\n", itpTimevalDiff(&startTime, &curTime));
#endif
            return true;
        }
    }
    prevAct = act;
    return false;
}

//=============================================================================
/**
 * extract and generate minutiaes of input finger data.
 * @param fingerImage   finger sensor Y data and area information.
 * @param fingerInfo    output of finger minutiaes info.
 * @return              true - success, false - failed.
 */
//=============================================================================
bool
getFingerInfo(
    FingerImage         *fingerImage,
    FingerInfo          *fingerInfo)
{
    // do finger extract
    int ret, i1, activeNimutiae, h, v, idx, size;
    int f_width = fingerImage->width, f_height = fingerImage->height;
    unsigned char *capture, *median, *mean, *variance, *border, *dir, *enhdir;
    unsigned char *image0, *image1, *image2;
    unsigned char *block0, *block1, *block2, *block3, *block4;
    unsigned char *binary, *smooth, *thinning, *tmp;
    int *integralValue = NULL;
    fingerMCCType   *mccInfo = &fingerInfo->mccInfo;
    fingerMatchType *relationInfo = &fingerInfo->relationInfo;
    fingerType      tmpFingerData = { 0 };

    image0 = (unsigned char*) malloc(gFingerConfig.width * gFingerConfig.height);
    image1 = (unsigned char*) malloc(gFingerConfig.width * gFingerConfig.height);
    image2 = (unsigned char *)malloc(gFingerConfig.width * gFingerConfig.height);
    block0 = (unsigned char *)malloc((gFingerConfig.width/WIN8)*(gFingerConfig.height/WIN8));
    block1 = (unsigned char *)malloc((gFingerConfig.width/WIN8)*(gFingerConfig.height/WIN8));
    block2 = (unsigned char *)malloc((gFingerConfig.width/WIN8)*(gFingerConfig.height/WIN8));
    block3 = (unsigned char *)malloc((gFingerConfig.width/WIN8)*(gFingerConfig.height/WIN8));
    block4 = (unsigned char *)malloc((gFingerConfig.width/WIN8)*(gFingerConfig.height/WIN8));
    integralValue = (int *)malloc(gFingerConfig.width * gFingerConfig.height * sizeof(int));

    median = image1;
    capture = fingerImage->fingerData;
    // step 1: median filter

///    ret = median_filter(f_width, f_height, capture, median);
    ret = median_filter(f_width, f_height, capture, median, gFingerConfig.srcRegionLeft, gFingerConfig.srcRegionTop, gFingerConfig.srcRegionRight, gFingerConfig.srcRegionBottom);

    localShadingCorrection(median, f_width, f_height, integralValue);
    // [SmallY20160715] scale up 4/3 for 320x240 image
    imageBilinearScalePasteY(image2, f_width, f_height, 0, 0, f_width, f_height, median, f_width, f_height, gFingerConfig.srcRegionLeft, gFingerConfig.srcRegionTop, gFingerConfig.srcRegionRight, gFingerConfig.srcRegionBottom);
    median = image2;

    globalNormalize(median, f_width, f_height);

    // step 2: find direction
    dir = block0;
    ret = find_dir(f_width, f_height, median, dir);

    // step 3: find border

    mean = block1;
    variance = block2;
    border = block3;
    ret = find_mean(f_width, f_height, median, mean);
    ret = find_variance(f_width, f_height, median, mean, variance);
    ret = find_border(f_width, f_height, median, mean, variance, border);

    // for scaling up 4/3 to 320x240
    for (v = 0; v < f_height / WIN8; v++)
    {
        for (h = 0; h < f_width / WIN8; h++)
        {
            idx = v * f_width / WIN8 + h;
            if (v < B_TOP)
            {
                border[idx] = 1;
            }
            else if (v > f_height / WIN8 - B_BOTTOM - 1)
            {
                border[idx] = 1;
            }
            if (h < B_LEFT)
            {
                border[idx] = 1;
            }
            else if (h > f_width / WIN8 - B_RIGHT - 1)
            {
                border[idx] = 1;
            }
            if (v - B_TOP + h - B_LEFT < CORNER)
            {
                border[idx] = 1;
            }
            if (v - B_TOP + f_width / WIN8 - 1 - B_RIGHT - h < CORNER)
            {
                border[idx] = 1;
            }
            if (f_height / WIN8 - 1 - B_BOTTOM - v + h - B_LEFT < CORNER)
            {
                border[idx] = 1;
            }
            if (f_height / WIN8 - 1 - B_BOTTOM - v + f_width / WIN8 - 1 - B_RIGHT - h < CORNER)
            {
                border[idx] = 1;
            }
        }
    }

    // step 4: direction  filter
    // step 2: find direction

    enhdir = image0;

    imageBlur(block4, mean, f_width / WIN8, f_height / WIN8);
    mean = block4;
    imageBlur(block1, variance, f_width / WIN8, f_height / WIN8);
    variance = block1;

    ret = dir_filter(f_width, f_height, median, dir, mean, variance, enhdir);

    // step 5: local segmentation
    binary = image1;
    ret = find_mean(f_width, f_height, enhdir, mean);

    imageBlur(block2, mean, f_width / WIN8, f_height / WIN8);
    mean = block2;

    ret = binary_filter(f_width, f_height, enhdir, mean, binary);

    // step 6: smooth filter and clear border
    ret = clear_border(f_width, f_height, binary, border, binary);
    smooth = image0;
///    ret = smooth_filter(f_width, f_height, binary, smooth);
    ret = smooth_filter(f_width, f_height, binary, smooth, B_LEFT * WIN8, B_TOP * WIN8, f_width - B_RIGHT * WIN8, f_height - B_BOTTOM * WIN8);

    // step 7: thinning filter
    thinning = image0;
    tmp = image1;
///    ret = thinning_filter(f_width, f_height, thinning, tmp);
    ret = thinning_filter(f_width, f_height, thinning, tmp, B_LEFT * WIN8, B_TOP * WIN8, f_width - B_RIGHT * WIN8, f_height - B_BOTTOM * WIN8);

    // step 8: find minutiae
    ret = dilation_border(f_width/WIN8, f_height/WIN8, border, block0);
    size = find_minutiae(f_width, f_height, thinning, block0, minutiaes);

    // step 9: remove false minutiae
    ret = remove_false_minutiae(f_width, f_height, thinning, size, minutiaes);

    //minutiaes
    relationInfo->width = tmpFingerData.width = f_width;
    relationInfo->height = tmpFingerData.height = f_height;
    relationInfo->hori_res = 192;
    relationInfo->vert_res = 192;

    for (i1 = activeNimutiae = 0; i1 < size && activeNimutiae < MINUTIAE_MAX_NO; i1++)
    {
        if (minutiaes[i1].type > 0)
        {
          relationInfo->minutiae[activeNimutiae].X = tmpFingerData.minutiae[activeNimutiae].X = minutiaes[i1].x;
          relationInfo->minutiae[activeNimutiae].Y = tmpFingerData.minutiae[activeNimutiae].Y = minutiaes[i1].y;
///          fingerInfo->minutiae[activeNimutiae].thita = (256 - minutiaes[i1].dir * 16) & 0x0FF; // for DIR16
          relationInfo->minutiae[activeNimutiae].thita = minutiaes[i1].dir * 8; // for DIR32
          activeNimutiae++;
        }
    }
    relationInfo->minutiae_no = tmpFingerData.minutiae_no = activeNimutiae;
    createMCCs(&tmpFingerData, mccInfo); 

    printf("\tBase finger has %d minutiae\n", activeNimutiae);

    free(image0);
    free(image1);
    free(image2);
    free(block0);
    free(block1);
    free(block2);
    free(block3);
    free(block4);
    free(integralValue);
}

//=============================================================================
/**
 * get two finger minutiaes info compared score.
 * @param fingerInfo0   first finger minutiaes info
 * @param fingerInfo1   second finger minutiaes info
 * @return              range from 0 ~ 10000 (larger is better)
 */
//=============================================================================
int
compareFingerInfo(
    FingerInfo *fingerInfo0,
    FingerInfo *fingerInfo1)
{
    int mccScore = getFingerSimi(&fingerInfo0->mccInfo,&fingerInfo1->mccInfo);

    printf("mcc score: %d\n", mccScore);
    if (mccScore > 2000)
    {
        return getFingerMatchScore(&fingerInfo0->relationInfo, &fingerInfo1->relationInfo, NULL);
    }
    else
    {
        return 0;
    }
}

//=============================================================================
/**
 * get two finger mcc comparision result.
 * @param fingerInfo0   first finger minutiaes info
 * @param fingerInfo1   second finger minutiaes info
 * @return              range from 0 ~ 10000 (larger is better)
 */
//=============================================================================
int
getFingerMccResult(
    FingerInfo *fingerInfo0,
    FingerInfo *fingerInfo1)
{
    return getFingerSimi(&fingerInfo0->mccInfo,&fingerInfo1->mccInfo);
}

//=============================================================================
/**
 * get two finger relation match comparison result.
 * @param fingerInfo0   first finger minutiaes info
 * @param fingerInfo1   second finger minutiaes info
 * @return              range from 0 ~ 10000 (larger is better)
 */
//=============================================================================
int
getFingerRelationResult(
    FingerInfo *fingerInfo0,
    FingerInfo *fingerInfo1)
{
    return getFingerMatchScore(&fingerInfo0->relationInfo, &fingerInfo1->relationInfo, NULL);
}


