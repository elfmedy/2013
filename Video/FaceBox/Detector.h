
/************************************************************************\
Module:
	Detector
	
Abstract:

	1) SDetectorConfig
		Usage:
		Pass it as configuration to CDetector when calling DetectInImage.

	2) CDetector
		Usage:
		Call DetectInImage to detect face in an image. Outputs SFaceItems
		(and SampleDB).
Notes:
	
History:
	Created by sticky, 11/05/2004
	Last modified by sticky, 11/06/2004
\************************************************************************/

#ifndef _DETECTOR_H_
#define _DETECTOR_H_

#include "TreeClassifier.h"
#include "NewIntegralImage.h"
#include "FaceItem.h"
#include <math.h>

#include <iostream>

using namespace std;

//
// Tree configuration
//
enum TreeSelection
{
	SELECT_FRONTAL,
	SELECT_FRONTAL_RIP_360,
	SELECT_HALF_MULTI_VIEW,
	SELECT_FULL_MUTLI_VIEW,
	SELECT_HALF_OMNI_VIEW,
	SELECT_FULL_OMNI_VIEW
};

class STreeConfig
{
public:
	//int m_iLayerNum[MAX_VIEWS];
	double m_dSpeedUpRatio;
	int m_iLayerNum;
	TreeSelection m_TreeSelection;
	BOOL m_bDetector[MAX_VIEWS];
	BOOL m_bDetectorAvailable[MAX_VIEWS];

	// constant max image size
	BOOL m_bConstMaxImageWidth;
	int m_iMaxImageWidth;

	STreeConfig()
	{
		m_iLayerNum = -1;
		// Initialize as multi-view
		for(int i=0;i<MAX_VIEWS;i++){
			m_bDetector[i] = FALSE;
			m_bDetectorAvailable[i] = FALSE;
		}

		m_TreeSelection = SELECT_FRONTAL;
		m_dSpeedUpRatio = 0;

		m_bConstMaxImageWidth = FALSE;
		m_iMaxImageWidth = 320;
	}

};



//
// Detector configuration
//
class SDetectorConfig
{
public:
	SDetectorConfig(int posC2Fstep = 3,			// Coarse to fine step 
					int scale_image = 1,	// whether scale the image
					int scaleNum = 3,			// Scale ratio will be 2^(- 1 / scaleNum)
					double scale_ratio = 1.25,	// if scale template, use scale ratio
					int startSubWindowW = 24,	// Start sub-window width
					int endSubwindowW = 256,	// End sub-window width
					double postThre = 0,	// Post verify threshold
					int initialX = -1,			// Position to start search
					int initialY = -1,			// default will be half coarse to fine step
					double minDev = 100,			// Max deviation of sub-window
					double maxDev = 65484,		// Min deviation of sub-window
					int maxFace = 4096,			//max faces output
					int sparseFace = FALSE,		//make responses of faces sparse
					int SmoothIntensity = 0	//preprocess the input image
					)
	{
		m_iPosC2FStep = posC2Fstep;
		m_iPosC2FHalfStep = posC2Fstep / 2;

		m_iStartX = initialX;
		m_iStartY = initialY;
		if (m_iStartX < 0) m_iStartX = m_iPosC2FHalfStep;
		if (m_iStartY < 0) m_iStartY = m_iPosC2FHalfStep;

		m_ScaleImage = scale_image;
		if(m_ScaleImage){
			m_iScaleNum = scaleNum;
			m_dScaleRatio = pow(2.0, 1.0 / m_iScaleNum);
		}else{
			m_dScaleRatio = scale_ratio;
			m_iScaleNum = 0;
		}

		m_iStartSubWindowW = m_iStartSubWindowH = startSubWindowW;
		m_iEndSubWindowW = m_iEndSubWindowH = endSubwindowW;

		m_dPostThreshold = postThre;

		m_dMinDeviation = minDev;
		m_dMaxDeviation = maxDev;
		m_iMaxFace = maxFace;
		m_bSparseFace = sparseFace;

		m_iSmoothIntensity = SmoothIntensity;
	}


	void SetInitialPosition(int pos)
	{
		m_iStartX = pos%3;
		m_iStartY = pos/3;
	}
	
	// Sub-window size
	int m_iStartSubWindowW;
	int m_iStartSubWindowH;
	int m_iEndSubWindowW;
	int m_iEndSubWindowH;

	// Coarse search start position
	int m_iStartX;
	int m_iStartY;

	// Coarse to fine ratio (x, y)
	int m_iPosC2FStep;		// 3 by default
	int m_iPosC2FHalfStep;	// 1 by default
	
	// Coarse to fine ratio (scale)
	//int m_iScaleC2FStep;	// not used
	//int m_iScaleC2FHalfStep;// not used

	// Scale number, scale ratio will be 2^(- 1 / scaleNum)
	int m_iScaleNum;
	double m_dScaleRatio;

	// Post verify threshold
	double m_dPostThreshold;

	// Valid deviation for sub-window to be classified.
	double m_dMinDeviation;
	double m_dMaxDeviation;

	// scale image or scale template
	int m_ScaleImage;

	// max faces output
	int m_iMaxFace;

	// sparse face
	BOOL m_bSparseFace;

	// smooth filter
	int m_iSmoothIntensity;

	// Other possible member variables
	// 1) Reference window size is determined by STreeClassifier
	// 2) ...
};

//
// Detector
//
#define MAX_IMAGE_WIDTH 4096

class CDetector
{
public:

	//
	// Variables (for search)
	//

	// For classifying by a node
	BOOL m_bPassed[MAX_NUM_LUT];

	// WFS queue
	SNodeClassifier* m_NodeList[MAX_VIEWS];
	int m_iNodeListHead, m_iNodeListTail;

	// Output
	// Number of passed views
	int m_iPassedViewNum;
	// Id of passed views
	int m_iPassedViews[MAX_VIEWS];
	// Confidence of passed views
	// output layer
	int m_iPassedLayer[MAX_VIEWS];
	// output of passed views
	float m_fPassedViewOutput[MAX_VIEWS];

	// Vector of integral images
	CNewIntegralImage* m_vIImages;

	double m_pCacheMean[MAX_IMAGE_WIDTH];
	double m_pCacheReciprocalCov[MAX_IMAGE_WIDTH];
	double m_pCacheCov[MAX_IMAGE_WIDTH];
	int m_pCacheOffset[MAX_IMAGE_WIDTH];
	int m_pCacheInt[MAX_IMAGE_WIDTH];
	double m_pCacheDouble[MAX_IMAGE_WIDTH];

	// Number of scaned sub-window in a image		
	//added by smooth
	int m_iScanedWindowC;
	int m_iScanedWindowF;
	int m_iDetectedViewC;
	int m_iInputPatchNumC;
	int m_iPassPatchNumC;
	int m_iDetectedViewF;
	int m_iInputPatchNumF;
	int m_iPassPatchNumF;
	double m_dFeatureCalculated;

	bool m_bScaled;
	

	//
	// Methods
	// 

	// Constructor
	CDetector()
	{
		m_vIImages= NULL;
		m_bScaled = false;
	}

	// Destructor
	~CDetector()
	{
		delete []m_vIImages;
	}

	// Delete integral images
	void DeleteIImgs();

	bool Init(int srcW, int srcH, SDetectorConfig& config, STreeConfig& tree_config, vector<STreeClassifier*> &trees);

	// Detect faces in an image  
	//modified by smooth
	void DetectInImage(
						IplImage* srcImage,				// Source image
						SDetectorConfig& config,			// Configuration
						STreeConfig& tree_config,
						vector<STreeClassifier*> &trees,		// Prepared tree 
						SFaceItems& faces					// Result face items
						);

							
	// Scan method
	// return TRUE if successful
	BOOL ScanImage(CNewIntegralImage &oIImage, 
								int x_start, int x_end, 
								int y_start, int y_end,
								SDetectorConfig& config,			// Configuration
								vector<STreeClassifier*> &trees,
								SFaceItems &faces, 
								SIZE &faceSize);

	// Search methods
	void InitializeSearch(vector<SNodeClassifier*> &root_nodes);
	BOOL SearchUntil(CNewIntegralImage &image, int &layer_index);
	void ContinueWithDFS(CNewIntegralImage &image);

	// Transform pose information
	void GetPose(int rr,			// ID of the view
					int &tilted,	// Tilt angle
					int &yaw);		// Yaw

};


#endif
