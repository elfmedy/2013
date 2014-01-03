// createData.cpp : 定义控制台应用程序的入口点。
//

/*
 * use original image and mask image to create object image
 * which can be used for MPEG-7 feature retrieval's test
 * XXX: we just create 50 images in two types
*/
#include "stdafx.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "BlobResult.h"
#include "blob.h"
#include <string>

using namespace cv;
using namespace std;

void readImageSequenceFiles(vector <string> &imgNames, vector <string> &imgNamesMask, vector <string> &imgNmaesWrite);


int _tmain(int argc, _TCHAR* argv[])
{
	vector <string> imgNames;
	vector <string> imgNamesMask;
	vector <string> imgNmaesWrite;
	readImageSequenceFiles(imgNames, imgNamesMask, imgNmaesWrite);

	for(unsigned int i = 0; i < imgNames.size(); i++)
	{
		Mat frame = imread(imgNames[i]);
		Mat maskImage = imread(imgNamesMask[i], 0);
		threshold( maskImage, maskImage, 81, 255, CV_THRESH_BINARY );	//delete shadow
		medianBlur(maskImage, maskImage, 3);							//delete small noise
		IplImage ipl_maskImage = maskImage;
		CBlobResult blobs = CBlobResult( &ipl_maskImage, NULL, 0 );
		blobs.Filter( blobs, B_EXCLUDE, CBlobGetArea(), B_LESS, 30 );	// filter blobs that area smaller than a certern num
																		// XXX: i assume there is only one blob !!
		cv::Mat imgRoi = frame( blobs.GetBlob(0)->GetBoundingBox() );
		imwrite(imgNmaesWrite[i], imgRoi);
	}

	return 0;
}

void readImageSequenceFiles(vector <string> &imgNames, vector <string> &imgNamesMask, vector <string> &imgNmaesWrite)
{
	string path("F:\\sviberesult\\org\\");
	string path2("F:\\sviberesult\\sum\\");
	string outPath("output\\");
	char str[7];
	str[6] = '\0';

	for (int i = 125; i < 150; i++)
	{
		int k = 1;
		for (int j = 5; j >= 0; j--)
		{
			str[j] = i / k % 10 + '0';
			k = k * 10;
		}
		string pcur = path + string(str) + ".png";
		string pcur2 = path2 + string(str) + ".png";

		imgNames.push_back(pcur);
		imgNamesMask.push_back(pcur2);
	}

	for (int i = 900; i < 925; i++)
	{
		int k = 1;
		for (int j = 5; j >= 0; j--)
		{
			str[j] = i / k % 10 + '0';
			k = k * 10;
		}
		string pcur = path + string(str) + ".png";
		string pcur2 = path2 + string(str) + ".png";

		imgNames.push_back(pcur);
		imgNamesMask.push_back(pcur2);
	}

	char str2[3];
	str2[0] = '\0'; str2[1] = '\0'; str2[2] = '\0'; 
	for (int i = 0; i < 50; i++)
	{
		if (i < 10)
		{
			str2[0] = i + '0';
		}
		else
		{
			str2[0] = i / 10 + '0';
			str2[1] = i % 10 + '0';
		}
		string pcur = outPath + string(str2) + ".jpg";
		imgNmaesWrite.push_back(pcur);
	}
}