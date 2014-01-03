#ifndef _OBSERVE_LAST_H_
#define _OBSERVE_LAST_H_


#include "ITrackModel.h"
#include "SqlStore.h"

//������ݱ��浽���ݿ�
class ObserveLast : public IObserver
{
public:
	ObserveLast(ITrackModel *mod, SqlStore *db) : IObserver(mod), database(db) {}
	void Update(void *para);
private:
	SqlStore *database;
};


#endif
