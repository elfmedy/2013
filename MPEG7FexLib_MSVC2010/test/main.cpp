/***************************************************************
 * edit by : elfmedy
 * this is a application for video retrieval
 * INFO: just a test app
		 eg, i use array instead of file, only for very less
		 number of image
 **************************************************************/

#include <iostream>
#include "FexWrite.h"
#include "tpl.h"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <vector>

using namespace cv;
using namespace std;

#define NUM 10

void CreateDB()
{
    Mat image;
	unsigned char *dcdDataStore = new unsigned char[32768 * 101];   //3M
	unsigned char *ehdDataStore = new unsigned char[1048576 * 101];	//100M
	for (int i = 0; i < 32768; i++)
		dcdDataStore[i * 101] = 0;
	for (int i = 0; i < 1048576; i++)
		ehdDataStore[i * 101] = 0;

	for (unsigned char i = 0; i < 50; i++)		//we calc 50 images in the folder
	{
		// load an image
		char arrStr[15];
		sprintf(arrStr, "img\\%d.jpg", i);
		image = imread(arrStr);

		// create a Frame object (see include/Frame.h)
		// allocate memory for 3-channel color and 1-channel gray image and mask
		Frame* frame = new Frame( image.cols, image.rows, true, true, true);
		// set the image of the frame
		frame->setImage(image);

		// DCD with normalization (to MPEG-7 ranges), without variance, without spatial coherency
		unsigned int arr[8];
		int count = FexWrite::computeWriteDCD( frame, i, arr, true, false, false);
		for (int j = 0; j < count; j++)
		{
			int number = ++dcdDataStore[arr[j] * 101];
			dcdDataStore[arr[j] * 101 + number] = i;
		}

		// EHD
		unsigned int ehdValue = FexWrite::computeWriteEHD( frame , i);
		int number2 = ++ehdDataStore[ehdValue * 101];
		ehdDataStore[ehdValue * 101 + number2] = i;

		delete frame;
	}

	//pack to file
    tpl_node *tn1 = tpl_map("c#", dcdDataStore, 32768 * 101);
    tpl_pack(tn1, 0);                         // pack all elements at once 
    tpl_dump(tn1, TPL_FILE, "feature\\dcd.tpl");
    tpl_free(tn1);

    tpl_node *tn2 = tpl_map("c#", ehdDataStore, 1048576 * 101);
    tpl_pack(tn2, 0);                         // pack all elements at once 
    tpl_dump(tn2, TPL_FILE, "feature\\ehd.tpl");
    tpl_free(tn2);

	printf("\nover, press any key\n");
	getchar();
}

void MergeResult(vector<unsigned char> &v1, vector<unsigned char> &v2, vector<unsigned char> &result)
{
	result.clear();

	vector<unsigned char>::iterator p1 = v1.begin();
	vector<unsigned char>::iterator p2 = v2.begin();
	
	while (p1 != v1.end() && p2 != v2.end())
	{
		if ( (*p1) == (*p2) )
		{
			result.push_back(*p1);
			p1++;
			p2++;
		}
		else if (*p1 < *p2)
		{
			p1++;
		}
		else
		{
			p2++;
		}
	}
}

void Test(int imgNum)
{
    Mat image;
	unsigned char *dcdDataStore = new unsigned char[32768 * 101];   //3M
	unsigned char *ehdDataStore = new unsigned char[1048576 * 101];	//100M
	
	// unpack 
    tpl_node *tn1 = tpl_map("c#", dcdDataStore, 32768 * 101);
    tpl_load(tn1, TPL_FILE, "feature\\dcd.tpl");
    tpl_unpack(tn1, 0);  
    tpl_free(tn1);

    tpl_node *tn2 = tpl_map("c#", ehdDataStore, 1048576 * 101);
    tpl_load(tn2, TPL_FILE, "feature\\ehd.tpl");
    tpl_unpack(tn2, 0); 
    tpl_free(tn2);
	

	int i = imgNum;

	// load an image
	char arrStr[15];
	sprintf(arrStr, "img\\%d.jpg", i);
	image = imread(arrStr);

	// create a Frame object (see include/Frame.h)
	// allocate memory for 3-channel color and 1-channel gray image and mask
	Frame* frame = new Frame( image.cols, image.rows, true, true, true);
	// set the image of the frame
	frame->setImage(image);

	// EHD
	unsigned int ehdValue = FexWrite::computeWriteEHD( frame , i);
	//printf("EHD have %d ids\n    The id is:", ehdDataStore[ehdValue * 101]);
	vector<unsigned char> ehdVec;
	for (int k = 0; k < ehdDataStore[ehdValue * 101]; k++)
	{
		//printf("%d ", ehdDataStore[ehdValue * 101 + k + 1]);
		ehdVec.push_back(ehdDataStore[ehdValue * 101 + k + 1]);
	}
	//printf("\n");

	vector<unsigned char> sumVec;
	// DCD with normalization (to MPEG-7 ranges), without variance, without spatial coherency
	unsigned int arr[8];
	int count = FexWrite::computeWriteDCD( frame, i, arr, true, false, false);
	//printf("DCD domain color num = %d\n", count);
	vector<unsigned char> dcdVec;
	for (int j = 0; j < count; j++)
	{
		//printf("    DCD%d, this single color have %d ids\n    The id is:", j + 1, dcdDataStore[arr[j] * 101]);
		vector<unsigned char> tmpVec;
		for (int k = 0; k < dcdDataStore[arr[j] * 101]; k++)
		{
			//printf("%d ", dcdDataStore[arr[j] * 101 + k + 1]);
			tmpVec.push_back(dcdDataStore[arr[j] * 101 + k + 1]);
		}
		//printf("\n");
		if (j == 0)
		{
			if (tmpVec.size() < NUM)
			{
				sumVec = tmpVec;
				break;
			}
			MergeResult(tmpVec, ehdVec, sumVec);
		}
		else
		{
			vector<unsigned char> reTmp;
			MergeResult(sumVec, tmpVec, reTmp);
			sumVec = reTmp;
		}
		if (sumVec.size() < NUM)
		{
			break;
		}
	}

	printf("\nSUM RESULT have %d ids, they are ", sumVec.size());
	for (int k = 0; k < sumVec.size(); k++)
		printf("%d ", sumVec[k]);
	printf("\n");

	delete frame;

	printf("\nover, press any key\n");
	getchar();
}


int main( int argc, char* argv[] )
{
	//CreateDB();
	Test(2);

    return 0;
}
