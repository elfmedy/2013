

#include "stdafx.h"
#include "svibe.h"
#include "cv.h"
#include "highgui.h"
#include <string>

using namespace cv;
using namespace std;


IplImage *ReadPic()
{
	static int i = 1;
	//string path("F:\\dataset\\shadow\\busStation\\input\\in");
	//string path("F:\\dataset\\baseline\\highway\\input\\in");
	//string path("F:\\dataset\\cameraJitter\\sidewalk\\input\\in");
	//string path("F:\\dataset\\dynamicBackground\\canoe\\input\\in");
	string path("F:\\dataset\\intermittentObjectMotion\\sofa\\input\\in");
	//string path("F:\\dataset\\thermal\\park\\input\\in");
	//in000001.jpg
	
	char str[7];
	str[6] = '\0';
	int k = 1;
	for (int j = 5; j >= 0; j--)
	{
		str[j] = i / k % 10 + '0';
		k = k * 10;
	}
	string pcur = path + string(str) + ".jpg";
	i++;

	IplImage *frame = cvLoadImage(pcur.c_str());
	return frame;
}

void SavePic(IplImage *pic)
{
	static int i = 1;
	//string path("F:\\myresults\\shadow\\busStation\\bin");
	//string path("F:\\myresults\\baseline\\highway\\bin");
	//string path("F:\\myresults\\cameraJitter\\sidewalk\\bin");
	//string path("F:\\myresults\\dynamicBackground\\canoe\\bin");
	string path("F:\\myresults\\intermittentObjectMotion\\sofa\\bin");
	//string path("F:\\myresults\\thermal\\park\\bin");
	//in000001.jpg
	
	char str[7];
	str[6] = '\0';
	int k = 1;
	for (int j = 5; j >= 0; j--)
	{
		str[j] = i / k % 10 + '0';
		k = k * 10;
	}
	string pcur = path + string(str) + ".png";
	i++;

	cvSaveImage(pcur.c_str(), pic);
}

void SavePic2(IplImage *pic)
{
	static int i = 1;
	string path("F:\\sviberesult2\\org\\");
	
	char str[7];
	str[6] = '\0';
	int k = 1;
	for (int j = 5; j >= 0; j--)
	{
		str[j] = i / k % 10 + '0';
		k = k * 10;
	}
	string pcur = path + string(str) + ".png";
	i++;

	cvSaveImage(pcur.c_str(), pic);
}

void SavePic3(IplImage *pic)
{
	static int i = 1;
	string path("F:\\sviberesult2\\sum\\");
	
	char str[7];
	str[6] = '\0';
	int k = 1;
	for (int j = 5; j >= 0; j--)
	{
		str[j] = i / k % 10 + '0';
		k = k * 10;
	}
	string pcur = path + string(str) + ".png";
	i++;

	cvSaveImage(pcur.c_str(), pic);
}

int _tmain(int argc, _TCHAR* argv[])
{
	VibeModel* vbM = NULL;
	unsigned char*  image_data = NULL; // image data in current image
    unsigned char*  output_data = NULL; // image data after background subtraction

	//CvCapture* video = cvCaptureFromFile("K:\\FFoutput\\DSCF1006.avi");
	//CvCapture* video = cvCaptureFromFile("K:\\beijing\\Backup\\±³¾°½¨Ä£\\testLbp\\testLbp\\synopsis4_cam1_new.avi");
	//CvCapture* video = cvCaptureFromFile("K:\\beijing\\Video\\TRACKtEST\\road3.avi");
	//CvCapture* video = cvCaptureFromFile("K:\\beijing\\Video\\TRACKtEST\\highwayII_raw.AVI");
	
	//IplImage *frame = cvQueryFrame(video);
	IplImage *frame = ReadPic();
	int width = frame->width;
	int height = frame->height;

	//foreground and greyimg
	output_data = (unsigned char *) malloc (width * height * 1);
    IplImage *foreground = cvCreateImage(cvGetSize(frame), frame->depth, 1);
	foreground->imageData = (char *) output_data;
	IplImage* greyImg = cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U, 1);
	//IplImage* pBack = cvCloneImage(frame);

	cvNamedWindow("FG",1);
	cvNamedWindow("ORI",1);
	int ccc = 0;
	while (frame)
	{
		//cvSmooth(frame, frame, CV_MEDIAN, 9);
		cvCvtColor(frame, greyImg, CV_BGR2GRAY);
		image_data = (unsigned char *) greyImg->imageData;

		if (!vbM)
			//vbM = VibeModelInit(width, height, 1, image_data);
			vbM = VibeModelInitC3(width, height, 3, (uint8_t *)(frame->imageData), image_data);
		//VibeModelUpdateC1(vbM, image_data, output_data);
		VibeModelUpdateC3(vbM, (uint8_t *)(frame->imageData), image_data, output_data);

		cvShowImage("ORI", frame);
		cvShowImage("FG", foreground);
		//SavePic2(frame);
		//SavePic3(foreground);

		if(cvWaitKey( 1 ) == 27) break;
		//if (ccc++ == 1500) break;
		//if (ccc++ == 300) cvSaveImage("F:\\svibe.png", foreground);
	
		cvReleaseImage(&frame);
		//frame = cvQueryFrame(video);
		frame = ReadPic();
	}	
	return 0;
}
