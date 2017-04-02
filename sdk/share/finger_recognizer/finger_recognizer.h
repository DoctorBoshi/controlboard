#ifndef _FINGER_RECOGNIZER_H__
#define _FINGER_RECOGNIZER_H__

#include "fr_minutiae_fix.h"
#include "fr_minutiaeMatch.h"

#ifdef __cplusplus
extern C {
#endif

//=============================================================================
//                Constant Definition
//=============================================================================

//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================

typedef struct FingerImage
{
    unsigned char *fingerData;
    int           pitchY;
    int           width;
    int           height;
} FingerImage;

typedef struct FingerRecognizerSetup
{
    int fingerWidth;
    int fingerHeight;
    int simiStepH;       //H direction step (default: 8)
    int simiStepV;       //V direction step (default: 8)
    int activeThreshold; //for valid finger check (default: 250)
    int simiThreshold;   //for valid finger check (default: 440)
} FingerRecognizerSetup;

typedef struct FingerInfo
{
    fingerMCCType   mccInfo;
    fingerMatchType relationInfo;
} FingerInfo;

//=============================================================================
//                Global Data Definition
//=============================================================================

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
    FingerRecognizerSetup *recognizerSetup);

//=============================================================================
/**
 * Check if the input finger data is stable or not.
 * @param fingerImage   finger sensor Y data and area information.
 * @return              true - stable finger image, false - not stable.
 */
//=============================================================================
bool
isStableFingerPrint(
    FingerImage *fingerImage);

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
    FingerInfo          *fingerInfo);


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
    FingerInfo *fingerInfo1);

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
    FingerInfo *fingerInfo1);

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
    FingerInfo *fingerInfo1);


#ifdef __cplusplus
}
#endif

#endif

