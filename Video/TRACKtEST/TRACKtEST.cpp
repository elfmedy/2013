// TRACKtEST.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "iTrack.h"
#include "highgui.h"
#include "cxcore.h"
#include "cv.h"

/*
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
*/

/*
typedef struct OBJECT_STRUCT_S{
	int x_pos;		//左下角
	int y_pos;
	int x_pos_dec;	//中心
	int y_pos_dec;
	int obj_width;
	int obj_height;
	unsigned char *p_data_org;		//rgb  原始区块
	unsigned char *p_data_mask;		//gray 区块中的前景
	unsigned int frame_time;//represented by the frame number in the video seq
	unsigned int total_frame_num;//
	int month;
	int day;
	int hour;
	int minute;
	int second;
	int N;							//去除和第一个框相等的点得到的路径中框的数目
	int start_time;
} OBJECT_STRUCT;

std::vector<std::vector<OBJECT_STRUCT> >  trajectory_association_quence;

void ConvertResultStyle(const std::vector<std::vector<ObjectBox> > &source, std::vector<std::vector<OBJECT_STRUCT> > &dest)
{
	for (std::vector<std::vector<ObjectBox> >::const_iterator iterLine = source.begin(); iterLine != source.end(); iterLine++)
	{
		std::vector<OBJECT_STRUCT> tmpVect;
		for (std::vector<ObjectBox>::const_iterator iterBox = (*iterLine).begin(); iterBox != (*iterLine).end(); iterBox++)
		{
			OBJECT_STRUCT tmpBox;
			tmpBox.x_pos = (*iterBox).x_pos;
			tmpBox.y_pos = (*iterBox).y_pos + (*iterBox).box_height;
			tmpBox.x_pos_dec = (*iterBox).x_pos + (*iterBox).box_width / 2;
			tmpBox.y_pos_dec = (*iterBox).y_pos + (*iterBox).box_height / 2;
			tmpBox.obj_width = (*iterBox).box_width;
			tmpBox.obj_height = (*iterBox).box_height;
			tmpBox.p_data_org = (*iterBox).data_org;
			tmpBox.p_data_mask = (*iterBox).data_mask;
			tmpBox.frame_time = (*iterBox).frame_count;
			tmpBox.total_frame_num = 0;
			tmpBox.month = 0;
			tmpBox.day = 0;
			tmpBox.hour = 0;
			tmpBox.minute = 0;
			tmpBox.second = 0;
			tmpBox.N = 0;
			tmpBox.start_time = 0;

			tmpVect.push_back(tmpBox);
		}
		dest.push_back(tmpVect);
	}
}
*/

int _tmain(int argc, _TCHAR* argv[])
{
	_CrtSetBreakAlloc(194);
	_CrtSetBreakAlloc(204);
	_CrtSetBreakAlloc(212);
	//_CrtSetBreakAlloc(703);

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
		obj->GetBackground();
	}
	cvReleaseImage(&pCaptureFrame);
	cvReleaseCapture(&pCapture);

	printf("over");
	getchar();

	CvCapture *pCaptureDebug = cvCreateFileCapture("TownCentre.avi");
	IplImage *pCaptureFrameDebug = cvCreateImage(cvSize(videoWidth, videoHeight), 8, 3);
	int frameCount = 0;
	while (pCaptureFrameDebug = cvQueryFrame(pCaptureDebug))
	{
		frameCount++;
		obj->DebugDisLine(pCaptureFrameDebug, frameCount);
	}
	cvReleaseImage(&pCaptureFrameDebug);
	cvReleaseCapture(&pCaptureDebug);
	//
	////ConvertResultStyle(obj->GetCompleteLine(), trajectory_association_quence);
	DestroyAnalysisObj(obj);

	//getchar();

	_CrtDumpMemoryLeaks();
	return 0;
	*/
	IVideoAnalysis *obj = CreateVideoAnalysisObj();

	CvCapture *pCapture = cvCreateFileCapture("synopsis1_cam2_0.avi");
	int videoWidth  = static_cast<int>(cvGetCaptureProperty(pCapture, CV_CAP_PROP_FRAME_WIDTH));
	int videoHeight = static_cast<int>(cvGetCaptureProperty(pCapture, CV_CAP_PROP_FRAME_HEIGHT));
	int videoFps    = static_cast<int>(cvGetCaptureProperty(pCapture, CV_CAP_PROP_FPS));
	int result = obj->Init(videoWidth, videoHeight, videoFps);

	IplImage *pCaptureFrame;
	while (pCaptureFrame = cvQueryFrame(pCapture))
	{
		obj->Process(pCaptureFrame);
		//obj->GetBackground();
	}
	cvReleaseImage(&pCaptureFrame);
	cvReleaseCapture(&pCapture);
	DestroyAnalysisObj(obj);

	printf("over");

	getchar();

	_CrtDumpMemoryLeaks();
	return 0;
}

