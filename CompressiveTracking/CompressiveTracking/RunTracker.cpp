/************************************************************************
* File:	RunTracker.cpp
* Brief: C++ demo for paper: Kaihua Zhang, Lei Zhang, Ming-Hsuan Yang,"Real-Time Compressive Tracking," ECCV 2012.
* Version: 1.0
* Author: Yang Xian
* Email: yang_xian521@163.com
* Date:	2012/08/03
* History:
* Revised by Kaihua Zhang on 14/8/2012, 23/8/2012
* Email: zhkhua@gmail.com
* Homepage: http://www4.comp.polyu.edu.hk/~cskhzhang/
* Project Website: http://www4.comp.polyu.edu.hk/~cslzhang/CT/CT.htm

* Brief : Add kalman filter to ct. i modified the org ct solution files 
* Author : Fei Yu
* Email : flyoverthecity@gmail.com
************************************************************************/
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <list>
#include "CompressiveTracker.h"
#include "mixTracker.h"
#include "BlobResult.h"
#include "blob.h"

using namespace cv;
using namespace std;


void readConfig(char* configFileName, char* imgFilePath, Rect &box);
/*  Description: read the tracking information from file "config.txt"
    Arguments:	
	-configFileName: config file name
	-ImgFilePath:    Path of the storing image sequences
	-box:            [x y width height] intial tracking position
	History: Created by Kaihua Zhang on 15/8/2012
*/
void readImageSequenceFiles(vector <string> &imgNames, vector <string> &imgNamesMask);
/*  Description: search the image names in the image sequences 
    Arguments:
	-ImgFilePath: path of the image sequence
	-imgNames:  vector that stores image name
	History: Created by Kaihua Zhang on 15/8/2012
*/

#define MAX_PATH 100
RNG rng(12345);


class TrackLine
{
public:
	KalmanFilter kf;
	CompressiveTracker ct;
	Rect box;
	float score[5];
	int miss;
public:
	TrackLine(): kf(4, 2, 0), ct(), miss(0) {}
};


int main(int argc, char * argv[])
{
	vector <string> imgNames;
	vector <string> imgNamesMask;
	char strFrame[20];
	readImageSequenceFiles(imgNames, imgNamesMask);

	list<TrackLine> trackLineArr;

	// read org frame and forground for process
	// you can modify it to read video by add a segment alg
	for(unsigned int i = 40; i < imgNames.size() - 1; i++)
	{ 		
		Mat frame = imread(imgNames[i]);
		Mat grayImg;
		cvtColor(frame, grayImg, CV_RGB2GRAY);

		Mat maskImage = imread(imgNamesMask[i], 0);

		// get blobs and filter them using its area
		// use 'cvblobslib' to get the object blobs
		threshold( maskImage, maskImage, 81, 255, CV_THRESH_BINARY );
		medianBlur(maskImage, maskImage, 3);
			
		IplImage ipl_maskImage = maskImage;
		CBlobResult blobs = CBlobResult( &ipl_maskImage, NULL, 0 );
		blobs.Filter( blobs, B_EXCLUDE, CBlobGetArea(), B_LESS, 30 );	// filter blobs that area smaller than a certern num
		
		list<CBlob *> remBlob;
		for (int k = 0; k < blobs.GetNumBlobs(); k++)
		{
			remBlob.push_back(blobs.GetBlob(k));
		}

		printf("%d\n", trackLineArr.size());
		for (list<TrackLine>::iterator trackIter = trackLineArr.begin(); trackIter != trackLineArr.end(); )
		{
			//kf predicition, get kfRect
			Mat kfPrediction = (trackIter->kf).predict();
			Point kfPrePt((int)(kfPrediction.at<float>(0)), (int)(kfPrediction.at<float>(1)));
			Rect kfRect(kfPrePt.x - (trackIter->box).width / 2, kfPrePt.y - (trackIter->box).height / 2, (trackIter->box).width, (trackIter->box).height);
			//ct predicition, get ctRect
			int ctError = 0;
			Rect ctRect(trackIter->box);
			float score = (trackIter->ct).predicition(grayImg, ctRect);
			rectangle(frame, kfRect, Scalar(0, 200, 0));	//green, kf predicition box
			rectangle(frame, ctRect, Scalar(0, 0, 200));	//red, ct predicition box
			//union predicit rectangle
			//if they have no same area, we consider ct is wrong, because kalman is physical movement
			float areaScale = (float)(sqrt((kfRect & ctRect).area() *1.0 / kfRect.area()));
			Point movePoint((int)((ctRect.x - kfRect.x) * areaScale), (int)((ctRect.y - kfRect.y) * areaScale));
			Rect unionPreRect = kfRect + movePoint;

			//calc object box
			Rect objRect;
			int j = 0;
			for (list<CBlob *>::iterator blobIter = remBlob.begin(); blobIter != remBlob.end(); )
			{
				Rect detRect((*blobIter)->GetBoundingBox());
				float detArea = (float)((*blobIter)->Area());
				if ((unionPreRect & detRect).area() > 0)
				{
					if (j++ == 0) objRect = detRect;
					else objRect = objRect | detRect;
					blobIter = remBlob.erase(blobIter);
				}
				else blobIter++;
			}

			// let box's area equal
			float objArea = (float)(objRect.area());
			objRect = Rect((int)(objRect.x + objRect.width / 2.0 - unionPreRect.width / 2.0), 
				(int)(objRect.y + objRect.height / 2.0 - unionPreRect.height / 2.0), 
				unionPreRect.width, unionPreRect.height);

			float detAreaScale = (float)(sqrt(objArea * 1.0 / unionPreRect.area()));
			if (detAreaScale > 1.0) detAreaScale = 1.0;
			Point detMovePoint((int)((objRect.x - unionPreRect.x) * detAreaScale), (int)((objRect.y - unionPreRect.y) * detAreaScale));
			Rect unionCorrRect = unionPreRect + detMovePoint;

			// if detect area > 0
			if (objArea > 0)
			{
				trackIter->box = unionCorrRect;
				rectangle(frame, unionCorrRect, Scalar(200,0,0), 1);
				//kf correct
				Mat_<float> measurement(2,1);
				measurement(0) = (float)((trackIter->box).x + (trackIter->box).width / 2.0);
				measurement(1) = (float)((trackIter->box).y + (trackIter->box).height / 2.0);
				(trackIter->kf).correct(measurement);
				//ct update
				(trackIter->ct).update(grayImg, trackIter->box);

				trackIter++;
			}
			// else we beleve tracking miss
			else
			{
				if ((trackIter->miss)++ == 5) trackIter = trackLineArr.erase(trackIter);
				else trackIter++;
			}
		}

		// !!!
		// use a sample way to get a new track init object box, i just add all others box toghter and expand it bigger
		// it's not a good idea when two object appear at the same time will lead only one init box
		// and, this sample is reasonless. so, i suggest you to use another method to get the init box
		// here, i just give a tracking alg, with a bad method to get init box, all -_-!
		// !!!
		Rect tmprect;
		int u = 0;
		for (list<CBlob *>::iterator blobIter = remBlob.begin(); blobIter != remBlob.end(); blobIter++)
		{
			if (u++ == 0) tmprect = Rect((*blobIter)->GetBoundingBox());
			else tmprect = tmprect | Rect((*blobIter)->GetBoundingBox());
		}
		if (tmprect.area() > 0) tmprect = Rect(tmprect.x - 5, tmprect.y - 8, tmprect.width + 10, tmprect.height + 16);
		if (tmprect.area() > 0 && tmprect.x != 0 && tmprect.y != 0 && (tmprect.x + tmprect.width) != 319 && (tmprect.y + tmprect.height) != 239)
		{
			TrackLine track;
			track.kf.transitionMatrix = *(Mat_<float>(4, 4) << 1,0,1,0,   0,1,0,1,  0,0,1,0,  0,0,0,1);
			track.kf.measurementMatrix = *(Mat_<float>(2, 4) << 1,0,0,0,   0,1,0,0);
			setIdentity(track.kf.processNoiseCov, Scalar::all(1e-4));
			setIdentity(track.kf.measurementNoiseCov, Scalar::all(1e-1));
			setIdentity(track.kf.errorCovPost, Scalar::all(.1));
			// kf init
			track.kf.statePre.at<float>(0) = (float)(tmprect.x + tmprect.width / 2.0);
			track.kf.statePre.at<float>(1) = (float)(tmprect.y + tmprect.height / 2.0);
			track.kf.statePre.at<float>(2) = 0;
			track.kf.statePre.at<float>(3) = 0;
			track.kf.statePost.at<float>(0) = (float)(tmprect.x + tmprect.width / 2.0);
			track.kf.statePost.at<float>(1) = (float)(tmprect.y + tmprect.height / 2.0);
			track.kf.statePost.at<float>(2) = 0;
			track.kf.statePost.at<float>(3) = 0;
			// ct init
			track.ct.init(grayImg, tmprect);

			rectangle(frame, tmprect, Scalar(255, 0, 0), 2, 7);
			track.box = tmprect;

			trackLineArr.push_back(track);
		}

		sprintf(strFrame, "#%d ",i) ;
		putText(frame,strFrame,cvPoint(0,20),2,1,CV_RGB(25,200,25));

		char outstr[20];
		//if (0)
		//if ((i >= 450 && i <= 600) || (i >= 930 && i <= 960) || (i >= 1420 && i <= 1450))
		{
			sprintf(outstr, "output\\%d.png", i);
			string outstring(outstr);
			imwrite(outstring, frame);
			sprintf(outstr, "output\\mask_%d.png", i);
			string outstring2(outstr);
			imwrite(outstring2, maskImage);
		}

		//imshow("ORG", frame);
		//imshow("mask", maskImage);
		//waitKey(1);
	}

	return 0;
}



void readConfig(char* configFileName, char* imgFilePath, Rect &box)	
{
	int x;
	int y;
	int w;
	int h;

	fstream f;
	char cstring[1000];
	int readS=0;

	f.open(configFileName, fstream::in);

	char param1[200]; strcpy(param1,"");
	char param2[200]; strcpy(param2,"");
	char param3[200]; strcpy(param3,"");

	f.getline(cstring, sizeof(cstring));
	readS=sscanf (cstring, "%s %s %s", param1,param2, param3);

	strcpy(imgFilePath,param3);

	f.getline(cstring, sizeof(cstring)); 
	f.getline(cstring, sizeof(cstring)); 
	f.getline(cstring, sizeof(cstring));


	readS=sscanf (cstring, "%s %s %i %i %i %i", param1,param2, &x, &y, &w, &h);

	box = Rect(x, y, w, h);
	
}

/*
void readImageSequenceFiles(char* imgFilePath,vector <string> &imgNames)
{	
	imgNames.clear();

	char tmpDirSpec[MAX_PATH+1];
	sprintf (tmpDirSpec, "%s/*", imgFilePath);

	WIN32_FIND_DATA f;
	HANDLE h = FindFirstFile(tmpDirSpec , &f);
	if(h != INVALID_HANDLE_VALUE)
	{
		FindNextFile(h, &f);	//read ..
		FindNextFile(h, &f);	//read .
		do
		{
			imgNames.push_back(f.cFileName);
		} while(FindNextFile(h, &f));

	}
	FindClose(h);	
}
*/

void readImageSequenceFiles(vector <string> &imgNames, vector <string> &imgNamesMask)
{
	string path("F:\\sviberesult\\org\\");
	string path2("F:\\sviberesult\\sum\\");
	char str[7];
	str[6] = '\0';

	for (int i = 1; i < 1502; i++)
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
	//IplImage *frame = cvLoadImage(pcur.c_str());
	//return frame;
}
