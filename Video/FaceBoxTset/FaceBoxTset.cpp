// FaceBoxTset.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "cxcore.h"
#include "cv.h"
#include "highgui.h"
#include "ifacebox.h"
#include "FaceDetector.h"
#include "Multiview Face Detector Class DLL.h"

int _tmain(int argc, _TCHAR* argv[])
{
	IplImage *pImage = cvLoadImage("Original01.jpg", CV_LOAD_IMAGE_GRAYSCALE);

	//IFaceDll *p = GetFaceDll();

	
	SFaceItem a;
	SFaceItems faces(10, a);

	//p->Detect(pImage, faces);
	CFaceDetector * p = new CFaceDetector();
	p->Detect(pImage, faces);
	//SFaceItem faces[10];
	//FaceDetectorClass d;
	//int num = d.Detect(pImage, faces, 10);
	

	printf("we have %d faces in this image\n", faces.size());
	//for(int i = 0; i < num; i++)
	{
		//cvRectangle(pImage, cvPoint(faces[i].rect.left, faces[i].rect.top), cvPoint(faces[i].rect.right, faces[i].rect.bottom), cvScalar(255.));
	}

	cvShowImage("Detection Result", pImage);
	cvWaitKey(0);
	cvReleaseImage(&pImage);

	//p->Release();
	return 0;
}

