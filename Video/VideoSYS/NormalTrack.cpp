#include "NormalTrack.h"

using namespace std;

struct NormalTrackPara NORMALTRACK_DEFAULT_PARA = {
	200,		//box min
	10000,		//box max
	0.2,		//fraction min
	5.0,		//fraction max
	0,			//is debug?

	20,			//matching min distance
	1,			//how many frame can lost when match
	10,			//the min track-point in a track

	NULL,		//point to CvCapture
	0,			//video width
	0			//video heigh
};

//--------------------------------------------------------------------------------------------------
NTObjBox::NTObjBox(int _x, int _y, int _width, int _height, int _frameCount, unsigned char *_rectImage, unsigned char *_maskImage) :
	x(_x), y(_y), width(_width), height(_height), frameCount(_frameCount), rectImage(_rectImage), maskImage(_maskImage) 
{
}


inline void NTObjBox::DeleteData() 
{ 
	delete rectImage; 
	rectImage = NULL; 
	delete maskImage; 
	maskImage = NULL; 
}

inline int NTObjBox::Area()
{
	return width * height;
}


inline double NTObjBox::Proportion()
{
	return 1.0 * width / height;
}


inline int NTObjBox::CalDistance(const NTObjBox &box) 
{ 
	int dx = ( abs(2 * x + width - 2 * box.x - box.width) - width - box.width ) / 2; 
	int dy = ( abs(2 * y + height - 2 * box.y - box.height) - height - box.height ) / 2; 
	return max(dx, dy); 
}


void NTObjBox::Print() 
{ 
	printf("x:%d y:%d w:%d h:%d fc:%d\n", x, y, width, height, frameCount); 
}

//--------------------------------------------------------------------------------------------------
NTTrackLine::NTTrackLine(int _l, int _s, int _a, double _p, unsigned char *_f, NTObjBox &box) :
	lostCount(_l), startMs(_s), area(_a), proportion(_p), firstFrame(_f)
{
	line.push_back(box);
}


inline void NTTrackLine::DeleteFrameData() 
{ 
	delete firstFrame; 
	firstFrame = NULL; 
}


inline void NTTrackLine::DeleteLineData() 
{ 
	for (std::vector<NTObjBox>::iterator iter = line.begin(); iter != line.end(); iter++) 
		iter->DeleteData(); 
}


inline void NTTrackLine::DeleteAllData() 
{ 
	DeleteFrameData(); 
	DeleteLineData(); 
}


inline int NTTrackLine::CalDistance(const NTObjBox &box)
{
	return line.back().CalDistance(box);
}


void NTTrackLine::Print()
{
	int count = 0;
	printf("nodes(%d), lostCount(%d), startMS(%d), area(%d), proportion(%.2f)\n", line.size(), lostCount, startMs, area, proportion);
	for (vector<NTObjBox>::iterator iter = line.begin(); iter != line.end(); iter++)
	{
		printf("\t%d: ", count++);
		iter->Print();
	}
}

//--------------------------------------------------------------------------------------------------
NormalTrack::NormalTrack()
{
	m_BOUNDING_BOX_MIN = 0;
	m_BOUNDING_BOX_MAX = 0;
	m_BOUNDING_BOX_RATEMIN = 0.0;
	m_BOUNDING_BOX_RATEMAX = 0.0;
	m_ISDEBUG = 0;

	m_DISTANCE_INTER_FRAME_MAX = 0;
	m_TRACK_NUM_THRESHOLD = 0;
	m_CLEARUP_TRACK_POINT_MIN = 0;

	m_videoWidth  = 0;
	m_videoHeight = 0;
	m_frameCount  = 0;
}


NormalTrack::~NormalTrack()
{
	for (list<NTTrackLine>::iterator iter = tracklineList.begin(); iter != tracklineList.end(); iter++)
	{
		iter->DeleteAllData();
	}
	tracklineList.clear();
}


int NormalTrack::Init(void *para)
{
	struct NormalTrackPara *p = (struct NormalTrackPara *)(para);
	m_BOUNDING_BOX_MIN			= p->m_BOUNDING_BOX_MIN;
	m_BOUNDING_BOX_MAX			= p->m_BOUNDING_BOX_MAX;
	m_BOUNDING_BOX_RATEMIN		= p->m_BOUNDING_BOX_RATEMIN;
	m_BOUNDING_BOX_RATEMAX		= p->m_BOUNDING_BOX_RATEMAX;
	m_ISDEBUG					= p->m_ISDEBUG;

	m_DISTANCE_INTER_FRAME_MAX	= p->m_DISTANCE_INTER_FRAME_MAX;
	m_TRACK_NUM_THRESHOLD		= p->m_TRACK_NUM_THRESHOLD;
	m_CLEARUP_TRACK_POINT_MIN	= p->m_CLEARUP_TRACK_POINT_MIN;

	m_videoWidth  = p->m_videoWidth;
	m_videoHeight = p->m_videoHeight;
	m_pCapture = p->m_pCapture;

	return 0;
}


void NormalTrack::Process(unsigned char *pInput, const unsigned char *pMask, const unsigned char *pBack)
{
	m_frameCount++;

	IplImage *pMaskImage = cvCreateImage(cvSize(m_videoWidth, m_videoHeight), 8, 1);
	memcpy(pMaskImage->imageData, pMask, m_videoWidth * m_videoHeight);

	vector<NTObjBox> boxVector;
	CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq* contour = 0;														//storage memory£¬don't nees to delete
	cvFindContours(pMaskImage, storage, &contour, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0)); 
	while (contour)
	{
		double tmpArea = fabs(cvContourArea(contour));
		if (tmpArea > m_BOUNDING_BOX_MIN && tmpArea <= m_BOUNDING_BOX_MAX)
		{
			CvRect tmpRect = cvBoundingRect(contour, 0);
			if (((float)tmpRect.width / tmpRect.height) > m_BOUNDING_BOX_RATEMIN && ((float)tmpRect.width / tmpRect.height) < m_BOUNDING_BOX_RATEMAX)
			{
				unsigned char *data_org =  new unsigned char[(tmpRect.width * 3 + 3) / 4 * 4 * tmpRect.height];
				unsigned char *data_mask = new unsigned char[(tmpRect.width * 1 + 3) / 4 * 4 * tmpRect.height];
				unsigned char *tmpDesOrg = data_org; unsigned char *tmpDesMask = data_mask;
				const unsigned char *tmpSourOrg = pInput + 3 * tmpRect.y * m_videoWidth + 3 * tmpRect.x;
				const unsigned char *tmpSourMask = pMask + tmpRect.y * m_videoWidth + tmpRect.x;
				for(int y = 0; y < tmpRect.height; y++)
				{
					memcpy(tmpDesOrg, tmpSourOrg, tmpRect.width * 3);	
					memcpy(tmpDesMask, tmpSourMask, tmpRect.width);	
					tmpDesOrg += (tmpRect.width * 3 + 3) / 4 * 4; tmpDesMask += (tmpRect.width * 1 + 3) / 4 * 4;
					tmpSourOrg = tmpSourOrg + 3 * m_videoWidth; tmpSourMask += m_videoWidth;						
				}
				boxVector.push_back( NTObjBox(tmpRect.x, tmpRect.y, tmpRect.width, tmpRect.height, m_frameCount, data_org, data_mask) );
			}
		}
		contour = contour->h_next;
	}
	cvReleaseMemStorage(&storage);
	cvReleaseImage(&pMaskImage);
	
	if (m_ISDEBUG) 
	{
		vector<NTObjBox> copyBoxVector(boxVector);
		AddBoxVector(pInput, boxVector);
		Display(pInput, pMask, copyBoxVector);
	}
	else
	{
		AddBoxVector(pInput, boxVector);
	}
	CompleteTrack();
}


void NormalTrack::Display(unsigned char *pInput, const unsigned char *pMask, const vector<NTObjBox> &boxes)
{
	Print();

	IplImage* pOriginal = cvCreateImage(cvSize(m_videoWidth, m_videoHeight), IPL_DEPTH_8U, 3);
	memcpy(pOriginal->imageData, pInput, m_videoWidth * m_videoHeight * 3);

	IplImage* pSegImg = cvCreateImage(cvSize(m_videoWidth, m_videoHeight), IPL_DEPTH_8U, 1);
	memcpy(pSegImg->imageData, pMask, m_videoWidth * m_videoHeight);		

	IplImage *pBoxImage = cvCreateImage(cvSize(m_videoWidth, m_videoHeight), 8, 3);
	cvZero(pBoxImage);
	cvNot(pBoxImage, pBoxImage);
	for (vector<NTObjBox>::const_iterator iterDis = boxes.begin(); iterDis != boxes.end(); iterDis++)
	{
		NTObjBox tmpBox = (*iterDis);
		for (int imgHeight = 0; imgHeight < tmpBox.height; imgHeight++)
		{
			int tmpStart = 3 * (tmpBox.y + imgHeight) * m_videoWidth + 3 * tmpBox.x;
			memcpy(pBoxImage->imageData + tmpStart, (unsigned char*)(pInput + tmpStart), tmpBox.width * 3);
		}	
		cvRectangle(pBoxImage, cvPoint(tmpBox.x, tmpBox.y), \
			cvPoint(tmpBox.x + tmpBox.width, tmpBox.y + tmpBox.height), cvScalar(255,0,0), 1);
	}
	cvShowImage("original", pOriginal);
	cvShowImage("seg & filter", pSegImg);
	cvShowImage("box & limit", pBoxImage);
	cvReleaseImage(&pBoxImage);
	cvReleaseImage(&pSegImg);
	cvReleaseImage(&pOriginal);
	int key = cvWaitKey(1);	
}


void NormalTrack::Print()
{
	printf("------------------------this frame have %d tracks------------------------\n", tracklineList.size());
	int i = 0;
	for (list<NTTrackLine>::iterator iterTrackLine = tracklineList.begin(); iterTrackLine != tracklineList.end(); iterTrackLine++, i++)
	{
		printf("track(%d), ", i);
		iterTrackLine->Print();
	}
}

void NormalTrack::AddBoxVector(unsigned char *pInput, std::vector<NTObjBox> &boxVector)
{
	//to track: if a box in frame match this track, add it in. else , track's lost count+1. a box in frame can only be used once
	for (list<NTTrackLine>::iterator iterTrackLine = tracklineList.begin(); iterTrackLine != tracklineList.end(); iterTrackLine++)
	{
		vector< pair<int, int> > matchVector;
		int i = 0;
		for (vector<NTObjBox>::iterator iterBoxInCurFrame = boxVector.begin(); iterBoxInCurFrame != boxVector.end(); iterBoxInCurFrame++, i++)
		{
			if (iterBoxInCurFrame->width != 0)	//box haven't be used
			{
				int distance = iterTrackLine->CalDistance(*iterBoxInCurFrame);
				if (distance < m_DISTANCE_INTER_FRAME_MAX)
				{
					matchVector.push_back(pair<int, int>(i, distance));
				}
			}
		}
		if (!matchVector.empty())
		{
			int matchedNum = PickBestBox(matchVector, boxVector, *iterTrackLine);
			iterTrackLine->lostCount = 0;
			iterTrackLine->area = boxVector[matchedNum].Area();
			iterTrackLine->proportion = boxVector[matchedNum].Proportion();
			iterTrackLine->line.push_back(boxVector[matchedNum]);
			boxVector[matchedNum].width = 0;	//if a box is matched, mark it, we can't use it later
		}
		else
		{
			(iterTrackLine->lostCount)++;
		}
	}

	//to box in frame: if a box match a track, add it to track. else, use this box creat a new track 
	for (vector<NTObjBox>::iterator iterBox = boxVector.begin(); iterBox != boxVector.end(); iterBox++)
	{
		if (iterBox->width != 0)	//if box is not mask, creat new track
		{	
			unsigned char *tmpImg = new unsigned char[m_videoHeight * m_videoWidth * 3];
			memcpy(tmpImg, pInput, m_videoHeight * m_videoWidth * 3);
			int startMs    = (int)(cvGetCaptureProperty( m_pCapture,CV_CAP_PROP_POS_MSEC ));
			tracklineList.push_back( NTTrackLine(0, startMs, iterBox->width * iterBox->height, 1.0 * iterBox->width / iterBox->height, tmpImg, *iterBox) );
		}
	}
}


int NormalTrack::PickBestBox(vector<pair<int, int> > &matchVector, vector<NTObjBox> &boxVector, NTTrackLine &line)
{
	if (matchVector.size() == 1)
	{
		return matchVector.front().first;
	}
	else
	{
		int minScore = numeric_limits<int>::max();
		int minCount = 0;
		for (vector<pair<int, int> >::iterator iter = matchVector.begin(); iter != matchVector.end(); iter++)
		{
			int score = 0;
			NTObjBox box = boxVector[iter->first];
			
			if (iter->second <= (m_DISTANCE_INTER_FRAME_MAX / 2.0)) score += 1;
			else score += 6;

			double areaScale = 1.0 * box.Area() / line.area;
			if (areaScale > 0.5 && areaScale < 2.0) score += 1;
			else score += 4;

			double proScale = 1.0 * line.proportion / box.Proportion();
			if (proScale > 0.7 && proScale < 1.5) score += 1;
			else score += 4;

			if (score < minScore)
			{
				minScore = score;
				minCount = iter->first;
			}
		}
		return minCount;
	}
}


void NormalTrack::CompleteTrack()
{
	for (list<NTTrackLine>::iterator iterTrackLine = tracklineList.begin(); iterTrackLine != tracklineList.end(); )
	{
		//if a track is lost, we call it over. then if it's point big than given num, we use it, or just del.
		//if a track is too long, we also have to cut it to prevent it use too many memory
		if (((int)(iterTrackLine->line.size()) > m_CLEARUP_TRACK_POINT_MIN && iterTrackLine->lostCount >= m_TRACK_NUM_THRESHOLD) || (int)(iterTrackLine->line.size()) > 500)
		{
			Notify( (void *)(&(*iterTrackLine)) );
			iterTrackLine->DeleteAllData();
			iterTrackLine = tracklineList.erase(iterTrackLine);
		}
		else if (iterTrackLine->lostCount < m_TRACK_NUM_THRESHOLD)
		{
			iterTrackLine++;
		}
		else
		{
			iterTrackLine->DeleteAllData();
			iterTrackLine = tracklineList.erase(iterTrackLine);
		}
	}
}


void NormalTrack::EndProcess()
{
	for (list<NTTrackLine>::iterator iterTrackLine = tracklineList.begin(); iterTrackLine != tracklineList.end(); )
	{
		//at the end, careless lost count, if a track is long enough, we use it, or just del. 
		if ( (int)(iterTrackLine->line.size()) > m_CLEARUP_TRACK_POINT_MIN)
		{
			Notify( (void *)(&(*iterTrackLine)) );
			iterTrackLine->DeleteAllData();
			iterTrackLine = tracklineList.erase(iterTrackLine);
		}
		else
		{
			iterTrackLine->DeleteAllData();
			iterTrackLine = tracklineList.erase(iterTrackLine);
		}
	}	
}