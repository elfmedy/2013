#include "iTrack.h"
#include "track.h"

IVideoAnalysis* CreateVideoAnalysisObj(VideoAnalysisPara &para)
{
	return new VideoAnalysis(para);
}

void DestroyAnalysisObj(IVideoAnalysis *obj)
{
	obj->Release();
}