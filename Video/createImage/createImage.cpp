#include "createImage.h"
#include <string>
#include <fstream>
#include <string>
#include "string.h"

using namespace std;

void _stdcall CreateImage(int id, const char *inDir, const char *outDir)
{
	char tmp[256];

	sprintf(tmp, "%s%09d", inDir, id);
	fstream fin(tmp, ios_base::in | ios_base::binary);
	int width, height;
	fin.read((char *)&width, sizeof(int));
	fin.read((char *)&height, sizeof(int));
	unsigned char *myImageData = new unsigned char[width * height * 3];
	fin.read((char *)myImageData, width * height * 3);

	IplImage *tmpHeader = cvCreateImageHeader(cvSize(width, height), IPL_DEPTH_8U, 3);
	cvSetImageData(tmpHeader, myImageData, width * 3);
 
	sprintf(tmp, "%s%09d_img.jpg", outDir, id);
	cvSaveImage(tmp, tmpHeader);

	delete [] myImageData;
	cvReleaseImageHeader(&tmpHeader);
}