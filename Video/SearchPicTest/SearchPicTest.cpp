// SearchPicTest.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "SearchPic.h"


int _tmain(int argc, _TCHAR* argv[])
{
	int arr[100];
	int hash;
	int count;
	SearchPic("K:\\beijing\\Video\\Video\\tmpupload\\s20134563500708.jpg", "K:\\beijing\\Video\\VideoSYS\\pictureFeat\\feat_001", arr, hash, count);
	//SearchVal("E:\\Video\\VideoSYS\\pictureFeat\\", 1, 7, arr);

	printf("hash : %d  count: %d", hash, count);
	getchar();
	return 0;
}

