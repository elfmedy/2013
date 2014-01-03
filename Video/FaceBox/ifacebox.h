#ifndef _I_FACE_BOX_H_
#define _I_FACE_BOX_H_

#include "cxcore.h"
#include "cv.h"
#include "highgui.h"
#include "FaceItem.h"

#ifdef FACEBOX_EXPORTS
#define FACEBOX_API __declspec(dllexport)
#else
#define FACEBOX_API __declspec(dllimport)
#endif

class IFaceDll
{
public:
	virtual void Detect(IplImage * srcImage, SFaceItems &faceItems) = 0;
	virtual void Release() = 0;
};

extern "C" FACEBOX_API IFaceDll * _stdcall GetFaceDll();
extern "C" FACEBOX_API void _stdcall ReleaseFaceDll(IFaceDll *obj);

#endif
