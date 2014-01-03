#ifndef _CREATE_IMAGE_H_
#define _CREATE_IMAGE_H_

#include "highgui.h"
#include "cxcore.h"
#include "cv.h"


#ifdef CREATEIMAGE_EXPORTS
#define CREATEIMAGE_API __declspec(dllexport)
#else
#define CREATEIMAGE_API __declspec(dllimport)
#endif

//Ä¬ÈÏdepth 3
extern "C" CREATEIMAGE_API void _stdcall CreateImage(int id, const char *inDir, const char *outDir);

#endif
