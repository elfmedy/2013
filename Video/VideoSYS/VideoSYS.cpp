#include "stdio.h"
#include "cxcore.h"
#include "cv.h"
#include "highgui.h"
#include "Dlbk.h"
#include "NormalTrack.h"
#include "SqlStore.h"
#include "ObserveBasic.h"
#include "ObserveFace.h"
#include "ObservePicture.h"
#include "ObserveLast.h"
#include <fstream>


//display process fraction.if isPrint == 1, output to console, else to file
void ProcessBar(int curFrame, time_t curTimeS, int sumCount, int isPrint)
{
	static double s_fraction = 0.0;
	double rate = curFrame * 1.0 / sumCount;
	if (rate >= (s_fraction + 0.01))
	{
		s_fraction = (int)(rate * 100) / 100.0;
		int remainTime = (int)((int)curTimeS * 1.0 / curFrame * (sumCount - curFrame));
		if (isPrint)
		{
			printf("Percent : %.0f%%\tUsed : %ds\tRemain : %ds\n", s_fraction * 100, (int)curTimeS, remainTime);
		}
		else
		{
			std::fstream fout("process\\process", std::ios_base::out);
			fout << (int)(s_fraction * 100) << "  " << (int)(curTimeS) << "  " << remainTime;
			fout.close();
		}

	}
}

int Run()
{
	CvCapture* pCapture = cvCaptureFromFile("synopsis1_cam2_0.avi");
	if (pCapture == NULL)
	{
		printf("open video failed!\n");
		return -1;
	}
	int video_width    = (int)(cvGetCaptureProperty( pCapture,CV_CAP_PROP_FRAME_WIDTH)); 
	int video_height   = (int)(cvGetCaptureProperty( pCapture,CV_CAP_PROP_FRAME_HEIGHT)); 
	int video_fps      = (int)(cvGetCaptureProperty( pCapture,CV_CAP_PROP_FPS));
	int video_sumCount = (int)(cvGetCaptureProperty( pCapture,CV_CAP_PROP_FRAME_COUNT));

	//back model
	struct DlbkPara dlbkPara = DLBK_DEFAULT_PARA;
	dlbkPara.videoWidth  = video_width;
	dlbkPara.videoHeight = video_height;
	IBackModel *backModel = new Dlbk();
	backModel->Init((void *)(&dlbkPara));

	//track model
	struct NormalTrackPara normalTrackPara = NORMALTRACK_DEFAULT_PARA;
	normalTrackPara.m_videoWidth  = video_width;
	normalTrackPara.m_videoHeight = video_height;
	normalTrackPara.m_pCapture    = pCapture;
	normalTrackPara.m_ISDEBUG     = 0;
	ITrackModel *trackModel = new NormalTrack();
	trackModel->Init((void *)(&normalTrackPara));

	//database
	struct SqlStorePara sqlStorePara = SQLSTORE_DEFAULT_PARA;
	sqlStorePara.m_isNewStart  = 1;
	sqlStorePara.m_cameraID    = 3;
	sqlStorePara.m_startYear   = 2013;
	sqlStorePara.m_startMonth  = 4;
	sqlStorePara.m_startDay    = 5;
	sqlStorePara.m_startHour   = 0;
	sqlStorePara.m_startMinute = 0;
	sqlStorePara.m_startSecond = 0;
	SqlStore *database = new SqlStore();
	database->Init(&sqlStorePara);

	//observer
	IObserver *basicObserver = new ObserveBasic(trackModel, database, video_width, video_height, video_fps);	//basic
	//FaceDetectorClass *faceDetector = new FaceDetectorClass();
	//IObserver *faceObserver = new ObserveFace(trackModel, database, faceDetector);								//face
	IObserver *pictureObserver = new ObservePicture(trackModel, database);										//picture feat
	IObserver *lastObserver = new ObserveLast(trackModel, database);											//last to store

	//CvVideoWriter *writer = cvCreateVideoWriter( "synopsis1_cam2_0_out.avi",  CV_FOURCC('X','V','I','D'), video_fps, cvSize(video_width, video_height), 0);

	int videoOverFlag = 0;
	int frameCount    = 0;
	int startTimeS    = (int)time(NULL);
	IplImage *pCapture_frame = NULL;
	while(1)
	{	
		for (int i = 0; i < 1; i++)
		{
			pCapture_frame = cvQueryFrame(pCapture);
			frameCount++;
			if (!pCapture_frame)
			{
				videoOverFlag = 1;
				break;
			}
		}
		if (videoOverFlag == 0)
		{
			backModel->Process((unsigned char *)(pCapture_frame->imageData));
			trackModel->Process((unsigned char *)(pCapture_frame->imageData), backModel->GetMask(), backModel->GetBack());
			//IplImage *tmp = cvCreateImage(cvSize(video_width, video_height), IPL_DEPTH_8U, 1);
			//memcpy(tmp->imageData, backModel->GetMask(), tmp->imageSize);
			//cvWriteFrame( writer, tmp );
			//cvReleaseImage(&tmp);
			cvWaitKey(1);
		}
		else
		{
			backModel->EndProcess();
			//trackModel->EndProcess();
			//database->EndStore();
			break;
		}
		ProcessBar(frameCount, time(NULL) - startTimeS, video_sumCount, 1);
	}

	delete lastObserver;
	delete pictureObserver;
	//delete faceObserver;
	//delete faceDetector;
	delete basicObserver;
	delete database;
	delete trackModel;
	delete backModel;
	cvReleaseCapture(&pCapture);

	//cvReleaseVideoWriter(&writer);

	return 0;
}


int main()
{
	Run();
	return 0;
}
