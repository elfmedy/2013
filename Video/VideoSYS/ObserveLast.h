#ifndef _OBSERVE_LAST_H_
#define _OBSERVE_LAST_H_


#include "ITrackModel.h"
#include "SqlStore.h"

//最后将数据保存到数据库
class ObserveLast : public IObserver
{
public:
	ObserveLast(ITrackModel *mod, SqlStore *db) : IObserver(mod), database(db) {}
	void Update(void *para);
private:
	SqlStore *database;
};


#endif
