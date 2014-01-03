
/************************************************************************\
Module:
	Preprocess
	
Abstract:

	1) SmoothFilter
		used to eliminate high frequency noise

Notes:
	
History:
	Created by smooth, 5/10/2006
	Last modified by smooth, 5/10/2006
\************************************************************************/

#ifndef _PREPROCESS_H_
#define _PREPROCESS_H_

#include "windows.h"

BOOL SmoothFilter(BYTE *pOriImage, int iW, int iH, int iStep, BYTE *&pImage);


#endif