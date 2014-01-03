#ifndef _OBSERVE_BASIC_H_
#define _OBSERVE_BASIC_H_

#include "ITrackModel.h"
#include "SqlStore.h"


//basic oberver: time period, overallImage, basic color
class ObserveBasic : public IObserver
{
public:
	ObserveBasic(ITrackModel *mod, SqlStore *db, int width, int height, int fps) : IObserver(mod), database(db), videoWidth(width), videoHeight(height), videoFps(fps){}
	void Update(void *para);
private:
	SqlStore *database;
	int videoWidth;
	int videoHeight;
	int videoFps;
};

#endif

