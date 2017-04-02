
#ifndef __FACE_RECOGNIZER__H__
#define __FACE_RECOGNIZER__H__

#include <cv.h>

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                Constant Definition
//=============================================================================

#define PERSON_NAME_SIZE		64

/****************************************************************************************\
*                                  PCA Face recognizer                                      *
\****************************************************************************************/
#define PCA_RECOGNIZED_HIGH			60000
#define PCA_RECOGNIZED_LOW			30000
#define PCA_MAX_COMPONENTS			50

/****************************************************************************************\
*                                  LDA Face recognizer                                      *
\****************************************************************************************/
#define LDA_RECOGNIZED_HIGH			60000
#define LDA_RECOGNIZED_LOW			30000
#define LDA_MAX_COMPONENTS			50

/****************************************************************************************\
*                                  LBP Face recognizer                                      *
\****************************************************************************************/
#define LBP_DATA_SIZE               3248
#define LBP_BLOCK_X                 30
#define LBP_BLOCK_Y                 30
#define LBP_BINS                    58
#define LBP_RECOGNIZED_HIGH         25000
#define LBP_RECOGNIZED_LOW          20000
#define LBP_KEY_THRESHOLD           3000

//=============================================================================
//                Structure Definition
//=============================================================================

typedef struct
{
    const char name[64];  // name
    void (*create)(void);
    void (*release)(void);
	int (*load)(const char *filename);
	int (*save)(const char *filename);
	int (*train)(int nTrainFaces, IplImage** faceImgArr);
	int (*predict)(IplImage* faceImg);
	int (*status)(void);
} FaceRecognizer;

typedef struct
{
    short dataBuffer[LBP_DATA_SIZE];
} LBP_FaceData;

//=============================================================================
//                Public Function Definition
//=============================================================================

/****************************************************************************************\
*                                  Face recognizer                                      *
\****************************************************************************************/
FaceRecognizer *createEigenFaceRecognizer(void);
void releaseEigenFaceRecognizer(void);

FaceRecognizer *createFisherFaceRecognizer(void);
void releaseFisherFaceRecognizer(void);

FaceRecognizer *createLBPFaceRecognizer(void);
void releaseLBPFaceRecognizer(void);

void LBP_create(void);
void LBP_release(void);
int LBP_load(const char *filename);
int LBP_save(const char *filename);
int LBP_train(int nTrainFaces, IplImage** faceImgArr) ;
int LBP_predict(IplImage* faceImg);
int LBP_status(void);

int LBP_generate(IplImage* faceImg, short* lbpData);
int LBP_updateDB(int nTrainFaces, short* lbpDataArr);

#ifdef __cplusplus
}
#endif

#endif

/* End of file. */
