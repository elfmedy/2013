#ifndef _OBSERVE_PICTURE_H_
#define _OBSERVE_PICTURE_H_


#include "ITrackModel.h"
#include "SqlStore.h"
#include <bitset>

//picture feat observer
class ObservePicture : public IObserver
{
public:
	ObservePicture(ITrackModel *mod, SqlStore *db) : IObserver(mod), database(db) {}
	void Update(void *para);
private:
	SqlStore *database;
};


#endif
