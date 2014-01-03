#include "cutImage.h"
#include <string>

using namespace std;

void _stdcall CutImage(int x, int y, int width, int height, const char *directory, const char *storePath)
{
	IplImage *img = cvLoadImage(directory);
	cvSetImageROI(img , cvRect(x, y, width, height));
	IplImage *dImg = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, img->nChannels);
	cvCopy(img, dImg, 0);
	cvResetImageROI(img);
	cvSaveImage(storePath, dImg);
	cvReleaseImage(&img);
	cvReleaseImage(&dImg);
}