#ifndef _NORMAL_TRACK_H_
#define _NORMAL_TRACK_H_

#include <vector>
#include <list>
#include <limits>
#include "ITrackModel.h"
#include "cxcore.h"
#include "cv.h"
#include "highgui.h"
#include "math.h"

//����
struct NormalTrackPara
{
	int    m_BOUNDING_BOX_MIN;			//ͼ���п�Ĵ�С����
	int    m_BOUNDING_BOX_MAX;			//ͼ���п�Ĵ�С����
	double m_BOUNDING_BOX_RATEMIN;		//�������Сֵ(����һ���ȿ��,ֻ��width/height)
	double m_BOUNDING_BOX_RATEMAX;		//��������ֵ
	int	   m_ISDEBUG;					//�ǲ���������debugģʽ��

	int m_DISTANCE_INTER_FRAME_MAX;		//����ƥ��֮���������С�ľ���
	int m_TRACK_NUM_THRESHOLD;			//ƥ��켣ʱ�켣�м�����ʧ��֡
	int m_CLEARUP_TRACK_POINT_MIN;		//�켣�����ٵ���Ч�����Ŀ

	CvCapture *m_pCapture;
	int m_videoWidth;
	int m_videoHeight;
};

//default parament
extern struct NormalTrackPara NORMALTRACK_DEFAULT_PARA;

//object box
class NTObjBox
{
public:
	int x;
	int y;
	int width;
	int height;
	int frameCount;
	unsigned char *rectImage;
	unsigned char *maskImage;
public:
	NTObjBox(int _x, int _y, int _width, int _height, int _frameCount, unsigned char *_rectImage, unsigned char *_maskImage);
	void DeleteData();
	int  Area();
	double Proportion();
	int  CalDistance(const NTObjBox &box);
	void Print();
};

//track
class NTTrackLine
{
public:
	int lostCount;
	int startMs;
	int area;
	double proportion;
	unsigned char *firstFrame;
	std::vector<NTObjBox> line;
public:
	NTTrackLine(int _l, int _s, int _a, double _p, unsigned char *_f, NTObjBox &box);
	void DeleteFrameData();
	void DeleteLineData();
	void DeleteAllData();
	int  CalDistance(const NTObjBox &box);
	void Print();
};

//normal track, interface ITrackModel, we use min distance to match a track
class NormalTrack : public ITrackModel
{
public:
	NormalTrack();
	~NormalTrack();
	int Init(void *para);
	void Process(unsigned char *pInput, const unsigned char *pMask, const unsigned char *pBack);
	void EndProcess();
	void Display(unsigned char *pInput, const unsigned char *pMask, const std::vector<NTObjBox> &boxes);
	void Print();
private:
	void AddBoxVector(unsigned char *pInput, std::vector<NTObjBox> &boxVector);
	int  PickBestBox(std::vector<std::pair<int, int> > &matchVector, std::vector<NTObjBox> &boxVector, NTTrackLine &line);
	void CompleteTrack();
private:
	std::list<NTTrackLine> tracklineList;
	int    m_BOUNDING_BOX_MIN;
	int    m_BOUNDING_BOX_MAX;
	double m_BOUNDING_BOX_RATEMIN;
	double m_BOUNDING_BOX_RATEMAX;
	int	   m_ISDEBUG;

	int m_DISTANCE_INTER_FRAME_MAX;
	int m_TRACK_NUM_THRESHOLD;
	int m_CLEARUP_TRACK_POINT_MIN;

	int m_videoWidth;
	int m_videoHeight;
	int m_frameCount;
	CvCapture *m_pCapture;
};

#endif

