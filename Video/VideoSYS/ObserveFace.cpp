#include "ObserveFace.h"
#include "NormalTrack.h"

using namespace std;

/*
	TODO:this part haven't finish!
	i write some tips for you below, better not save feat in sqlserver database because it's feat is too long, and searching is difficulity
	you can store them in files like i did with picture feat
*/

void ObserveFace::Update(void *para)
{
	NTTrackLine *p = (NTTrackLine *)(para);

	int maxFaceArea = 0;
	int maxCount = -1;

	int count = 0;
	for (vector<NTObjBox>::iterator iterBox = p->line.begin(); iterBox != p->line.end(); iterBox++, count++)
	{
		NTObjBox tmpBox = *iterBox;
		IplImage *tmpImage = cvCreateImageHeader(cvSize(tmpBox.width, tmpBox.height), IPL_DEPTH_8U, 3);
		cvSetImageData(tmpImage, tmpBox.rectImage, (tmpBox.width * 3 + 3) / 4 * 4);
		IplImage *tmpImageGray = cvCreateImage(cvSize(tmpBox.width, tmpBox.height), IPL_DEPTH_8U, 1);
		cvCvtColor(tmpImage,tmpImageGray,CV_BGR2GRAY);
		int num = detector->Detect(tmpImageGray, faces, 10);
		if (num > 0)
		{
			//only get first face in the track line(it should be only one)
			int tmpWidth = faces[0].rect.right - faces[0].rect.left;
			int tmpHeight = faces[0].rect.bottom - faces[0].rect.top;
			if (tmpWidth * tmpHeight > maxFaceArea)
			{
				maxFaceArea = tmpWidth * tmpHeight;
				maxCount = count;
				//save tmp - count | top | right | bottom | left
			}
		}
		cvReleaseImage(&tmpImageGray);
		cvReleaseImageHeader(&tmpImage);
	}
	if (maxCount != -1)
	{
		//get tmp and save feat to someplace 
	}
}
