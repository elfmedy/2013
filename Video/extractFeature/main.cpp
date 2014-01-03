#include "stdafx.h"
#include "extractFeature.h"
#include <iostream>



int _tmain(int argc, _TCHAR* argv[])
{
	_CrtSetBreakAlloc(231);

	/*
	IVideoAnalysis *obj = CreateVideoAnalysisObj();

	CvCapture *pCapture = cvCreateFileCapture("synopsis1_cam2_0.avi");
	int videoWidth  = static_cast<int>(cvGetCaptureProperty(pCapture, CV_CAP_PROP_FRAME_WIDTH));
	int videoHeight = static_cast<int>(cvGetCaptureProperty(pCapture, CV_CAP_PROP_FRAME_HEIGHT));
	int videoFps    = static_cast<int>(cvGetCaptureProperty(pCapture, CV_CAP_PROP_FPS));
	int result = obj->Init(videoWidth, videoHeight, videoFps);

	IplImage *pCaptureFrame = cvCreateImage(cvSize(videoWidth, videoHeight), 8, 3);
	while (pCaptureFrame = cvQueryFrame(pCapture))
	{
		obj->Process(pCaptureFrame);
	}
	cvReleaseImage(&pCaptureFrame);
	cvReleaseCapture(&pCapture);

	printf("over");
	*/
	//ExtractFeature obj;
	ExtractFeature *oobj = new ExtractFeature();
	//obj.Init("synopsis1_cam2_0.avi"); // synopsis1_cam2_0.avi
	oobj->Init("synopsis1_cam2_0.avi"); // synopsis1_cam2_0.avi

	CvCapture *pCapture = cvCreateFileCapture("synopsis1_cam2_0.avi");
	int videoWidth  = static_cast<int>(cvGetCaptureProperty(pCapture, CV_CAP_PROP_FRAME_WIDTH));
	int videoHeight = static_cast<int>(cvGetCaptureProperty(pCapture, CV_CAP_PROP_FRAME_HEIGHT));
	int videoFps    = static_cast<int>(cvGetCaptureProperty(pCapture, CV_CAP_PROP_FPS));
	IplImage *pCaptureFrame = cvCreateImage(cvSize(videoWidth, videoHeight), 8, 3);
	while (pCaptureFrame = cvQueryFrame(pCapture))
	{
		//obj.Process(pCaptureFrame);
		oobj->Process(pCaptureFrame);
		cvShowImage("original", pCaptureFrame);


		cvWaitKey(1);
	}
	cvReleaseImage(&pCaptureFrame);
	cvReleaseCapture(&pCapture);
	delete oobj;

	//getchar();
	_CrtDumpMemoryLeaks();

	return 0;
}