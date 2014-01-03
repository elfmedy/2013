/************************************************************************\
Module:
	New Integral image
	
Abstract:
	1) The class CNewIntegralImage	
	modified based on CIntegralImage to support pixel features

Notes:
	
History:
	Created by smooth, 12/23/2004
	
\************************************************************************/

#ifndef _NEW_INTEGRAL_IMAGE_H_
#define _NEW_INTEGRAL_IMAGE_H_

#include <windows.h>
#include "Common.h"

//	_SHRINK_TO_HALF_QUICKLY_ means keeping the original integral image
// 	_SHRINK_TO_HALF_ACCURATELY_ means keeping the size of the integral image in accord with that of bitmap image
//#define _SHRINK_TO_HALF_QUICKLY_
#define _SHRINK_TO_HALF_ACCURATELY_
#define DEFAULT_BANDWIDTH 4

#define MAX_BANDWIDTH 4			//this parameter can not be changed since the structure of new integral image is basend on it.

#define LEFT_SHIFT_BITS 10

class CNewIntegralImage			
{
public:

	//
	// Integral image
	//
	// memory block to store images with different bandwidth
	int m_iBandWidth;
	unsigned char *m_pImages;

	// A memory block to store the integral image
	long *m_pSumImage;			

	// A memory block to store the squared integral image
	double *m_pSum2Image;			

	int inv_ratio_i, mask_decemal;
	
	//
	// Basic infomation
	//

	// Actual width and height of current integral image:
	int m_iIImageW, m_iIImageH, m_iIImgStep;
	// Actual size of bitmap image
	int m_iBitmapW, m_iBitmapH, m_iBitmapStep, m_iBitmapSize;
	
	// in detectin program, it is m_pImages[(y*m_iBitmapStep + x)*4 + bandwidth], where the maximum of bandwidth is 3, i.e. a pixel of size 8 by 8
	// while in training program, the memory address of pixel (x, y, bandwidth) is m_pImages[bandwidth*m_iBitmapSize + y*m_iBitmapStep + x]
	// see the codes in function void SSinglePixelFeature::ScaleImage(CNewIntegralImage &iimg)

	// downscaling times. when ShrinkToHalf, it increases by 1
	int m_iDownScaledTime;

	// Current scale ratio relative to the original size; it changes when ShrinkToHalf is called
	double m_dScaleRatio; 

	// position of current scaning window
	int m_iCurX, m_iCurY;
	
	// Size of the current rect
	SIZE m_sizeCurSubwindow;	

	//////////////////////////////////////////////////////////////////////////
	// Redundant data to speed up computation								//
	//////////////////////////////////////////////////////////////////////////

	// Reciprocal of the area of the current rect
	double m_dReciprocal_of_areaSub;	
	
	// Mean value of the image data in the current rect
	double m_dMean;		
	int m_iMean;

	// Reciprocal of the standard deviation of the image data in the current rect
	double m_dReciprocal_of_cov;

	// Address in the bitmap image of the left top position of the current rect
	unsigned char *m_ref;	

	// Address in the integral image of the left top position of the current rect
	long *m_ref1;	

	// Address in the square integral image of the left top position of the current rect
	double *m_ref2;

	// IR offset between the left-top vertex and the right-top vertex of the current rect. just for integral image.
	int m_off12;	

	// IR offset between the left-top vertex and the left-bottom vertex of the current rect. just for integral image.
	int m_off13;	

	// IR offset between the left-top vertex and the right-bottom vertex of the current rect. just for integral image.
	int m_off14;

	//////////////////////////////////////////////////////////////////////////
	//					Method												//
	//////////////////////////////////////////////////////////////////////////
	
	//Constructor
	CNewIntegralImage();

	// Destructor
	virtual ~CNewIntegralImage();

	BOOL Init(int w, int h, int step, double ratio, int real_bitmapStep, int real_bitmapH, int real_iimgStep);
	
	// Calculate integral image with a scale ratio
	BOOL ComputeWithInterpolation(unsigned char* pImage,
								int w, 
								int h, 
								int step, 
								double ratio,
								//fix size
								int real_bitmapStep,
								int real_bitmapH,
								int real_iimgStep 
								);
	
	// Down sample integral image by half
	BOOL ShrinkToHalf();

	//
	// Set / Get sub-window info
	//

	// When to scan with a new scale, set the new scale size
	void SetSubWindowSize(SIZE size);

	// When scanning to a new line, set the new position of the current rect
	// Return value is the standard deviation
	double SetSubWindowPos(int x, int y);	

	// Get the x coordinates of the current rect
	void GetSubWindowXPos(long &x) { x = m_iCurX;}


	// Get the x coordinates of the current rect
	void GetSubWindowYPos(long &y) {	y = m_iCurY;}
	
	// 
	// Get size
	//

	// Get size of the integral image
	void GetSize(SIZE &IImgSize) { IImgSize.cx = m_iIImageW; IImgSize.cy = m_iIImageH; }
	
	// Get the size of current sub-window
	void GetScanWindowSize(SIZE &subWinowSize) { subWinowSize = m_sizeCurSubwindow;}

	// Get the original size of image
	void GetOriginalImageSize(SIZE &imageSize) { imageSize.cx = m_iBitmapW; imageSize.cy = m_iBitmapH; }

	// Get the additional width of integral image	//add by smooth
	static int GetAdditionalWidth() {return 2;}

	BOOL ShrinkToHalfAccurately();
	BOOL ShrinkToHalfQuickly();

private:
	void ComputeBitmapOfBandwidth();


};

#endif
