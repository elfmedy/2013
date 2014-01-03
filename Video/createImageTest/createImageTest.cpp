// createImageTest.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "createImage.h"
#include "highgui.h"
#include "cxcore.h"
#include "cv.h"


int _tmain(int argc, _TCHAR* argv[])
{
	//IplImage *c = cvLoadImage("000000009.jpg");
	//cvShowImage("c", c);
	//cvWaitKey(0);
	for (int i = 1; i <= 307; i++)
		CreateImage(i, "K:\\beijing\\Video\\VideoSYS\\overallImage\\", "K:\\beijing\\Video\\Video\\tmpimg\\");
	return 0;
}

