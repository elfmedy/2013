#ifndef _VIDEO_ITRACK_H_
#define _VIDEO_ITRACK_H_

#include <vector>
#include "cv.h"
#include "trackStruct.h"
#include "dlbk.h"

#ifdef TRACK_EXPORTS
#define TRACK_API __declspec(dllexport)
#else
#define TRACK_API __declspec(dllimport)
#endif

//Ä¬ÈÏ²ÎÊý
extern TRACK_API VideoAnalysisPara VIDEOANALYSIS_DEFAULT_PARA;

class TRACK_API IVideoAnalysis
{
public:
	virtual int  Init(int width, int height, int fps, DlbkPara &dlbkPara = DLBK_DEFAULT_PARA)   = 0;
	virtual void Process(const IplImage *pCaptureFrame)  = 0;
	virtual IplImage const * GetBackground() const = 0;
	virtual void AttachObserver(TrackObserver *observer) = 0;
	virtual void DetachObserver(TrackObserver *observer) = 0;

	virtual void Release() = 0;
};

TRACK_API IVideoAnalysis* CreateVideoAnalysisObj(VideoAnalysisPara &para = VIDEOANALYSIS_DEFAULT_PARA);
TRACK_API void DestroyAnalysisObj(IVideoAnalysis *obj);

#endif