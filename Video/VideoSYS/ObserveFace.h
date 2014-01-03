#ifndef _OBSERVE_FACE_H_
#define _OBSERVE_FACE_H_

#include "ITrackModel.h"
#include "SqlStore.h"
#include "Multiview Face Detector Class DLL.h"

//face observer
class ObserveFace : public IObserver
{
public:
	ObserveFace(ITrackModel *mod, SqlStore *db, FaceDetectorClass *det) : IObserver(mod), database(db), detector(det) {}
	void Update(void *para);
private:
	SqlStore *database;
	FaceDetectorClass *detector;
	SFaceItem faces[10];
};

#endif