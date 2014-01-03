#pragma once

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/tracking.hpp>
#include "CompressiveTracker.h"
#include "BlobResult.h"
#include <vector>
#include <list>

using std::vector;
using namespace cv;

/*
class TrackStruct
{
public:
	KalmanFilter *kf;
	CompressiveTracker *ct;

	Rect kfPreBox;
	Rect ctPreBox;

	int state;		//0 - ing ; 1 - miss
	int number;		// - 100 | 0--
	int ctState;	//is ct enable
public:
	void KFInit(Rect box);
	void CTInit();
	void KFRelease();
	void CTRelease();
};

class BoxStruct
{
public:
	CBlobResult blob;

	int isMulti;
public:
	//BoxStruct(int _x, int _y, int _w, int _h, int _isPeople, int _isMulti);
	//BoxStruct(int _x, int _y, int _w, int _h);
	//BoxStruct();
};


//get box
//void GetBoxArr(Mat &maskImage, vector<class BoxStruct> &boxArr);

//predicition
void TrackProcess(list<class TrackStruct> &tracks, vector<class BoxStruct> &blobArr);
*/

