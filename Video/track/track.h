#ifndef _VIDEO_TRACK_H_
#define _VIDEO_TRACK_H_


#include "iTrack.h"


//得出轨迹的类
class VideoAnalysis : public IVideoAnalysis
{
public:
	explicit VideoAnalysis(VideoAnalysisPara &para = VIDEOANALYSIS_DEFAULT_PARA);
	~VideoAnalysis();
	virtual int Init(int width, int height, int fps, DlbkPara &dlbkPara = DLBK_DEFAULT_PARA);		//初始化
	virtual void Process(const IplImage *pCaptureFrame);											//进程
	virtual IplImage const * GetBackground() const{return m_pBackground;}							//返回背景
	virtual void AttachObserver(TrackObserver *observer);		//增加观察者
	virtual void DetachObserver(TrackObserver *observer);		//减少观察者

	virtual void Release();										//release，回收所有内存

protected:
	void ProcessTrackLine(const IplImage *pCaptureFrame);									//将当前帧和路径匹配
	void ProcessClearUpTrackLine();															//对路径进行处理提取出完整路径，去掉没用的	
	double CalDistance(TrackLine &trackLine, const ObjectBox &box);							//判断路径最后一个和当前box时候匹配
	void UpdateBestScoreLinePara(TrackLine &trackLine,										//判断所有小于阈值distance的框，选出最好的
		const std::vector<std::pair<std::vector<ObjectBox>::iterator, double> > &iterVect,	
		std::vector<ObjectBox> &originalBox, 
		std::vector<ObjectBox> &copyBox);	
	void DeleteSingleFrameBoxData(std::vector<ObjectBox> &singleFrameBox);					//删除一帧中所有box的存储空间
																							

protected:
	//params
	int    m_BOUNDING_BOX_MIN;			//图像中框的大小下限
	int    m_BOUNDING_BOX_MAX;			//图像中框的大小上限
	double m_BOUNDING_BOX_RATEMIN;		//长宽比最小值(长不一定比宽大,只是width/height)
	double m_BOUNDING_BOX_RATEMAX;		//长宽比最大值
	double m_DISTANCE_INTER_FRAME_MAX;	//两次匹配之间允许的最小的距离!!
	double m_TRACK_TIME_THRESHOLD;		//容许丢失几秒钟 frame = fps * t，将时间换算成可以丢失的帧
	int    m_CLEARUP_TRACK_POINT_MIN;	//轨迹中最少的有效点的数目

	//video attr
	long		m_frameCount;
	int			m_videoWidth;
	int			m_videoHeight;
	int         m_videoFps;

	//data
	Cdlbk       *m_pDlbkObj;
	IplImage    *m_pTmpFrameImg;								//临时的前景，从背景算法中复制而来，用来提取出物体框
	IplImage	*m_pBackground;
	std::vector<ObjectBox> m_boxInSingleFrame;					//一帧中的box
	std::vector<TrackLine> m_trackLineVector;					//正在匹配中的轨迹

private:
	//
	int m_trackFrameThreshold;	//匹配帧阈值 frame = fps * t
	//obersver vector
	std::vector<TrackObserver *> m_observer;
};


#endif