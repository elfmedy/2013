// dblkTest.cpp : �������̨Ӧ�ó������ڵ㡣
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
		//����а����¼���������ѭ��
		//�˵ȴ�ҲΪcvShowImage�����ṩʱ�������ʾ
		//�ȴ�ʱ����Ը���CPU�ٶȵ���
		cvWaitKey(10);
		//break;
    }

	cvReleaseCapture(&pCapture);

	return 0;
}

