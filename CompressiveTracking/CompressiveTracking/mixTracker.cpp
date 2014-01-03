
#include "mixTracker.h"

/*
using std::vector;
using std::list;

using namespace cv;


//--------------------------------------------------------------------------------------------
void TrackStruct::KFInit(Rect box)
{
	kf = &(KalmanFilter(4, 2, 0));
	kf->transitionMatrix = *(Mat_<float>(4, 4) << 1,0,1,0,   0,1,0,1,  0,0,1,0,  0,0,0,1);
	setIdentity(kf->measurementMatrix);

	// init...
	kf->statePre.at<float>(0) = box.x + box.width / 2.0;
	kf->statePre.at<float>(1) = box.y + box.height / 2.0;
	kf->statePre.at<float>(2) = 0;
	kf->statePre.at<float>(3) = 0;

	setIdentity(kf->processNoiseCov, Scalar::all(1e-4));
	setIdentity(kf->measurementNoiseCov, Scalar::all(1e-1));
	setIdentity(kf->errorCovPost, Scalar::all(.1));

}

void TrackStruct::CTInit()
{

}

void TrackStruct::KFRelease()
{

}

void TrackStruct::CTRelease()
{

}


//--------------------------------------------------------------------------------------------
BoxStruct::BoxStruct(int _x, int _y, int _w, int _h, int _isPeople, int _isMulti)
		: x(_x), y(_y), w(_w), h(_h), isPeople(_isPeople), isMulti(_isMulti)
	{
	}

BoxStruct::BoxStruct(int _x, int _y, int _w, int _h)
		: x(_x), y(_y), w(_w), h(_h), isPeople(0), isMulti(0)
	{
	}

BoxStruct::BoxStruct()
		: x(0), y(0), w(0), h(0), isPeople(0), isMulti(0)
	{
	}


int IsTrackNear(class TrackStruct &track, class BoxStruct &blob)
{
	return 0;
}


//-------------------------------------------------------------------------------------------
//tracks
//boxArr
#define ASS_ARR_SIZE 20

void TrackProcess(list<class TrackStruct> &tracks, vector<class BoxStruct> &blobArr)
{
	//tracks's ass
	int tracksCount = 0;
	int blobArrCount = 0;
	int assArr[ASS_ARR_SIZE + 1][ASS_ARR_SIZE + 1] = {0};
	for (list<class TrackStruct>::iterator singleTrack = tracks.begin(); singleTrack != tracks.end(); singleTrack++, tracksCount++)
	{
		//kf predicition
		Mat kfPrediction = singleTrack->kf->predict();
		Point kfPrePt(kfPrediction.at<float>(0), kfPrediction.at<float>(1));
		//ct prediction
		if (singleTrack->ctState == 1)
		{

		}

		//ass
		blobArrCount = 0;
		for (vector<class BoxStruct>::iterator singleBlob = blobArr.begin(); singleBlob != blobArr.end(); singleBlob++, blobArrCount++)
		{
			if (IsTrackNear(*singleTrack, *singleBlob))
			{
				assArr[blobArrCount][tracksCount] = 1;
				assArr[blobArrCount][ASS_ARR_SIZE]++;
				assArr[ASS_ARR_SIZE][tracksCount]++;
			}
		}
	}

	tracksCount = 0;
	for (list<class TrackStruct>::iterator singleTrack = tracks.begin(); singleTrack != tracks.end(); singleTrack++, tracksCount++)
	{
		//1.this track can't find obj
		if (assArr[ASS_ARR_SIZE][tracksCount] == 0)
		{
			if (singleTrack->ctState == 1)
			{
				//ct 
			}
		}
		else
		{
			blobArrCount = 0;
			for (vector<class BoxStruct>::iterator singleBlob = blobArr.begin(); singleBlob != blobArr.end(); singleBlob++, blobArrCount++)
			{
				if (assArr[blobArrCount][tracksCount] == 1)
				{
					//2.this track find one obj
					if (assArr[blobArrCount][ASS_ARR_SIZE] == 1)
					{

					}
					//3.this track find more than one obj
					else
					{

					}
					break;		
				}
			}
		}
	}

	//obj's ass



		//kf correct
		Mat_<float> measurement(2,1);
		//measurement(0) = box.x + box.width / 2.0;
		//measurement(1) = box.y + box.height / 2.0;
		Mat estimated = ((*singleTrack).kf)->correct(measurement);
	}
}

*/