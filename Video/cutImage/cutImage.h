#ifndef _CUT_IMAGE_H_
#define _CUT_IMAGE_H_

#include "highgui.h"
#include "cxcore.h"
#include "cv.h"


#ifdef CUTIMAGE_EXPORTS
#define CUTIMAGE_API __declspec(dllexport)
#else
#define CUTIMAGE_API __declspec(dllimport)
#endif

extern "C" CUTIMAGE_API void _stdcall CutImage(int x, int y, int width, int height, const char *directory, const char *storePath);

#endif
