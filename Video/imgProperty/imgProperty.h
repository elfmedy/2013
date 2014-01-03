#ifndef _IMG_PROPERTY_
#define _IMG_PROPERTY_

#include "highgui.h"
#include "cxcore.h"
#include "cv.h"


#ifdef IMGPROPERTY_EXPORTS
#define IMGPROPERTY_API __declspec(dllexport)
#else
#define IMGPROPERTY_API __declspec(dllimport)
#endif

extern "C" IMGPROPERTY_API int * _stdcall GetImgProperty(const char *directory);

extern "C" IMGPROPERTY_API void _stdcall ReleaseArray(int *arr);

#endif