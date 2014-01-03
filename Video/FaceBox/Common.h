/************************************************************************\
Module:
	Common definitions
	
Abstract:

Notes:
	
History:
	Created by sticky, 11/01/2004
	Rewrited by smooth, 12/27/2004
	Rewrited by smooth, 8/29/2005
	
\************************************************************************/

#ifndef _COMMON_H_
#define _COMMON_H_

//
// General 
//

#define PI				3.1415926

#define REF_W			24
#define REF_H			24

#define LARGE_REF_W		26
#define LARGE_REF_H		26

//
// For classifiers
//

#define MAX_NUM_LUT		3
#define MAX_VIEWS			61

#define LEFT_FULL_PROFILE -90
#define LEFT_HALF_PROFILE -45
#define FRONTAL 0
#define RIGHT_HALF_PROFILE 45
#define RIGHT_FULL_PROFILE 90

//
// Some functions 
//

#define NIP(x,up,down) (x>up ? up : (x<down ? down:x))

//use or not use compact weak classifier. this technique can speed up the detection
#define _USE_COMPACT_WEAK_CLASSIFIER_		

//the sparse feature is balanced or not.
#define _BALANCED_SPARSE_FEATURE_		

#define _TRAIN_HALF_DETECTOR_

#endif
