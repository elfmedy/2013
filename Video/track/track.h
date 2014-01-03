#ifndef _VIDEO_TRACK_H_
#define _VIDEO_TRACK_H_


#include "iTrack.h"


//�ó��켣����
class VideoAnalysis : public IVideoAnalysis
{
public:
	explicit VideoAnalysis(VideoAnalysisPara &para = VIDEOANALYSIS_DEFAULT_PARA);
	~VideoAnalysis();
	virtual int Init(int width, int height, int fps, DlbkPara &dlbkPara = DLBK_DEFAULT_PARA);		//��ʼ��
	virtual void Process(const IplImage *pCaptureFrame);											//����
	virtual IplImage const * GetBackground() const{return m_pBackground;}							//���ر���
	virtual void AttachObserver(TrackObserver *observer);		//���ӹ۲���
	virtual void DetachObserver(TrackObserver *observer);		//���ٹ۲���

	virtual void Release();										//release�����������ڴ�

protected:
	void ProcessTrackLine(const IplImage *pCaptureFrame);									//����ǰ֡��·��ƥ��
	void ProcessClearUpTrackLine();															//��·�����д�����ȡ������·����ȥ��û�õ�	
	double CalDistance(TrackLine &trackLine, const ObjectBox &box);							//�ж�·�����һ���͵�ǰboxʱ��ƥ��
	void UpdateBestScoreLinePara(TrackLine &trackLine,										//�ж�����С����ֵdistance�Ŀ�ѡ����õ�
		const std::vector<std::pair<std::vector<ObjectBox>::iterator, double> > &iterVect,	
		std::vector<ObjectBox> &originalBox, 
		std::vector<ObjectBox> &copyBox);	
	void DeleteSingleFrameBoxData(std::vector<ObjectBox> &singleFrameBox);					//ɾ��һ֡������box�Ĵ洢�ռ�
																							

protected:
	//params
	int    m_BOUNDING_BOX_MIN;			//ͼ���п�Ĵ�С����
	int    m_BOUNDING_BOX_MAX;			//ͼ���п�Ĵ�С����
	double m_BOUNDING_BOX_RATEMIN;		//�������Сֵ(����һ���ȿ��,ֻ��width/height)
	double m_BOUNDING_BOX_RATEMAX;		//��������ֵ
	double m_DISTANCE_INTER_FRAME_MAX;	//����ƥ��֮���������С�ľ���!!
	double m_TRACK_TIME_THRESHOLD;		//����ʧ������ frame = fps * t����ʱ�任��ɿ��Զ�ʧ��֡
	int    m_CLEARUP_TRACK_POINT_MIN;	//�켣�����ٵ���Ч�����Ŀ

	//video attr
	long		m_frameCount;
	int			m_videoWidth;
	int			m_videoHeight;
	int         m_videoFps;

	//data
	Cdlbk       *m_pDlbkObj;
	IplImage    *m_pTmpFrameImg;								//��ʱ��ǰ�����ӱ����㷨�и��ƶ�����������ȡ�������
	IplImage	*m_pBackground;
	std::vector<ObjectBox> m_boxInSingleFrame;					//һ֡�е�box
	std::vector<TrackLine> m_trackLineVector;					//����ƥ���еĹ켣

private:
	//
	int m_trackFrameThreshold;	//ƥ��֡��ֵ frame = fps * t
	//obersver vector
	std::vector<TrackObserver *> m_observer;
};


#endif