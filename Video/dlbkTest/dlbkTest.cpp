// dblkTest.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "cxcore.h"
#include "cv.h"
#include "highgui.h"
#include "dlbk.h"


int _tmain(int argc, _TCHAR* argv[])
{
	IplImage* pFrame = NULL; 
	CvCapture* pCapture = NULL;

	pCapture = cvCaptureFromFile("synopsis1_cam2_0.avi");
	Cdlbk testModule;

	while(pFrame = cvQueryFrame( pCapture ))
    {
		testModule.Process(pFrame->width, pFrame->height, (unsigned char *)pFrame->imageData);

		//IplImage *bk = cvCreateImage( cvSize(pFrame->width, pFrame->height), IPL_DEPTH_8U, 3 );
		//IplImage *seg = cvCreateImage( cvSize(pFrame->width, pFrame->height), IPL_DEPTH_8U, 1 );
		//memcpy(bk->imageData, testModule.GetBack(), sizeof(uchar)*320*240*3);
		//memcpy(seg->imageData, testModule.GetFore(), sizeof(uchar)*320*240*1);
		//cvShowImage("video", bk);
		//cvShowImage("video1", seg);
		//cvReleaseImage(&bk);
		//cvReleaseImage(&seg);

		//cvShowImage("video", testModule.GetBack());
		//cvShowImage("video", pFrame);
		//如果有按键事件，则跳出循环
		//此等待也为cvShowImage函数提供时间完成显示
		//等待时间可以根据CPU速度调整
		cvWaitKey(10);
		//break;
    }

	cvReleaseCapture(&pCapture);

	return 0;
}

