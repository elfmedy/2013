#ifndef _CUT_IMAGE_H_
#define _CUT_IMAGE_H_

#include "highgui.h"
#include "cxcore.h"
#include "cv.h"


#ifdef SEARCHPIC_EXPORTS
#define SEARCHPIC_API __declspec(dllexport)
#else
#define SEARCHPIC_API __declspec(dllimport)
#endif

/*
struct PictureFeatStruct
{
	int count;
	unsigned char originalArr[7];
	int idArr[100];
};

a feat file have struct PictureFeatStruct[100,003], each element is a hash value
eg. an image with id=23, process this pic, we get hash value 1234, then we set struct PictureFeatStruct[1234]'s idArr 23
idArr size is 100, if id is more than 100, we create another file, feat_001, feat_002, etc.
count, originalArr in first feat file is correct, other files is null
*/

// picFile				in  : image file, name with path
// firstDataFile		in  : first feat file, name with path
// idArr				out : array to contain id, size is 100
// hashValue			out : image file's hash value
// idCount				out : id's number in a hash value
extern "C" SEARCHPIC_API void _stdcall SearchPic(const char *picFile, const char *firstDataFile, int *idArr, int &hashValue, int &idCount);


// dataFilePath			in  : feat file's path(without name)
// fileSerialNum		in  : feat file's num(eg. feat_001 is 1, feat_009 is 9, etc.)
// hashValue			in  : hash value
// idArr				out : array to contain id, size is 100
extern "C" SEARCHPIC_API void _stdcall SearchVal(const char *dataFilePath, int fileSerialNum, int hashValue, int *idArr);

#endif

