#include "extractFeature.h"
#include <iostream>
#include <fstream>
#include <windows.h>
#include "math.h"
#include "HKmean/CHIKMTree.h"
#include "Index/Index.h"


using namespace std;

//默认参数
ExtractFeaturePara EXTRACTFEATURE_DEFAULT_PARA = {
	1,			//多少条轨迹存一次数据库
	2,			//摄像机编号
	0			//视屏开始的时间,tm格式
};

ExtractFeature::ExtractFeature() 
{
}

ExtractFeature::~ExtractFeature()
{
	m_db.logoff();
	//删除观察者对象，同时清空装观察者对象的向量
	for (vector<TrackObserver *>::iterator iter = m_ptr2Observer.begin(); iter != m_ptr2Observer.end(); iter++)
	{
		if ((*iter) != NULL)
			delete (*iter);
	}
	m_ptr2Observer.clear();
	m_ptr2Observer.swap(std::vector<TrackObserver *>());
	//删除轨迹提取对象
	if (m_videoAnalysisObj != NULL)
		m_videoAnalysisObj->Release();
	//删除Capture(初始化失败的时候才会执行,因为如果init成功,里面已经删除了)
	if (m_tempCapture != NULL)
		cvReleaseCapture(&m_tempCapture);
}

//初始化
//建立轨迹对象和背景分割对象，传入参数结构体
//读取不到视频返回1，初始化轨迹对象失败返回2，连接数据库失败返回3
int ExtractFeature::Init(const char *fileName, DlbkPara &dlbkPara, VideoAnalysisPara &trackPara, ExtractFeaturePara &featurePara)
{
	m_tempCapture = cvCreateFileCapture(fileName);
	if (m_tempCapture == NULL) return 1;
	m_videoWidth  = static_cast<int>(cvGetCaptureProperty(m_tempCapture, CV_CAP_PROP_FRAME_WIDTH));
	m_videoHeight = static_cast<int>(cvGetCaptureProperty(m_tempCapture, CV_CAP_PROP_FRAME_HEIGHT));
	m_videoFps    = static_cast<int>(cvGetCaptureProperty(m_tempCapture, CV_CAP_PROP_FPS));

	m_videoAnalysisObj = CreateVideoAnalysisObj(trackPara);											//建立轨迹对象，传入参数
	int result = m_videoAnalysisObj->Init(m_videoWidth, m_videoHeight, m_videoFps, dlbkPara);		//初始化轨迹对象(在这里会建立并初始化背景分隔对象)
	m_NUM_TO_STORE = featurePara.m_NUM_TO_STORE;													//特征提取的参数
	m_CAMERA_ID = featurePara.m_CAMERA_ID;
	m_VIDEO_START = featurePara.m_VIDEO_START;
	if (result != 0) return 2;
	
	//加入观察者类
	//new的内存清除的问题：track里面不清除的，只是那个vector里面的指针清空而已
	//加入到extractFeature的链表里面并不做任何的动作，只是结束的时候利用这个链表删除new的对象而已
	//--加入基本信息的类(必须首先加入,因为它建立了存储队列)
	TrackObserver *basic = new AddBasicInfo(this);
	m_ptr2Observer.push_back(basic);
	m_videoAnalysisObj->AttachObserver(basic);
	//--颜色的类
	TrackObserver *objColor = new ObtainColor(this);
	m_ptr2Observer.push_back(objColor);
	m_videoAnalysisObj->AttachObserver(objColor);
	//--加入faceBox类
	TrackObserver *objFaceBox = new AddFaceBox(this);
	m_ptr2Observer.push_back(objFaceBox);
	m_videoAnalysisObj->AttachObserver(objFaceBox);

	//连接数据库
	try
	{
		otl_connect::otl_initialize();
		m_db.rlogon("UID=yufei;PWD=1598741;DSN=video");
		otl_cursor::direct_exec(m_db, "drop table video", otl_exception::disabled); //drop table 测试时候用
		otl_cursor::direct_exec(m_db, 
			"create table video(uid int identity(1,1) primary key not null, cameraid int, starttime datetime, endtime datetime, overallimage int," \
			"faceboxinfo1 int, faceboxinfo2 int, faceboxinfo3 int, faceboxinfo4 int, faceboxinfo5 int," \
			"objtype int," \
			"p1 int, p2 int, p3 int, p4 int, p5 int, a1 int, a2 int, a3 int, a4 int, a5 int)"
		);
	}catch(otl_exception& p){
		cout << p.msg << endl; 
		cout << p.stm_text << endl; 
		cout << p.sqlstate << endl; 
		cout << p.var_info << endl;
		return 3;
	}
	
	cvReleaseCapture(&m_tempCapture);
	m_tempCapture = NULL;
	return 0;
}

void ExtractFeature::Process(const IplImage *pCaptureFrame)
{
	//ExtractFeature::dataToStore.uid = m_uid++;
	//通过回调函数对轨迹做相应的处理来提取特征
	m_videoAnalysisObj->Process(pCaptureFrame);
	if (static_cast<int>(m_dataToStore.size()) >= m_NUM_TO_STORE)
	{
		Store();
	}
}

void ExtractFeature::Tm2DateTime(struct tm &oTime, otl_datetime &dTime)
{
	dTime.year   = oTime.tm_year + 1900;
	dTime.month  = oTime.tm_mon + 1;
	dTime.day    = oTime.tm_mday;
	dTime.hour   = oTime.tm_hour;
	dTime.minute = oTime.tm_min;
	dTime.second = oTime.tm_sec;
	dTime.frac_precision = 0;
	dTime.fraction		 = 0;
}


void ExtractFeature::Store()
{
	while (!m_dataToStore.empty())
	{
		otl_stream out(1,
			"insert into video (cameraid, starttime, endtime, overallimage, faceboxinfo1, faceboxinfo2, faceboxinfo3, faceboxinfo4, faceboxinfo5, objtype, p1, p2, p3, p4, p5, a1, a2, a3, a4, a5)" \
			"values(:cameraid<int>, :starttime<timestamp>, :endtime<timestamp>, :overallimage<int>," \
			":faceboxinfo1<int>, :faceboxinfo2<int>, :faceboxinfo3<int>, :faceboxinfo4<int>, :faceboxinfo5<int>," \
			":objtype<int>," \
			":color<int>, :p2<int>, :p3<int>, :p4<int>, :p5<int>," \
			":a1<int>, :a2<int>, :a3<int>, :a4<int>, :a5<int>)",
			m_db
		);

		SectionDatabase tmpDataToStore = m_dataToStore.front();
		m_dataToStore.pop();
		//time
		otl_datetime ts, te;
		Tm2DateTime(tmpDataToStore.startTime, ts);
		Tm2DateTime(tmpDataToStore.endTime, te);

		//write to sql server
		try
		{
			out << (signed)(tmpDataToStore.cameraId) << ts << te << (signed)(tmpDataToStore.overallImage) \
				<< (signed)(tmpDataToStore.faceBoxInfo1) << (signed)(tmpDataToStore.faceBoxInfo2) << (signed)(tmpDataToStore.faceBoxInfo3) << (signed)(tmpDataToStore.faceBoxInfo4) << (signed)(tmpDataToStore.faceBoxInfo5) \
				<< (signed)(tmpDataToStore.objType) \
				<< (signed)(tmpDataToStore.color) << (signed)(tmpDataToStore.otherP2) << (signed)(tmpDataToStore.otherP3) << (signed)(tmpDataToStore.otherP4) << (signed)(tmpDataToStore.otherP5) \
				<< (signed)(tmpDataToStore.otherA1) << (signed)(tmpDataToStore.otherA2) << (signed)(tmpDataToStore.otherA3) << (signed)(tmpDataToStore.otherA4) << (signed)(tmpDataToStore.otherA5);
		}
		catch(otl_exception& p)
		{
			  cerr << p.msg << endl; 
			  cerr << p.stm_text << endl; 
			  cerr << p.sqlstate << endl; 
			  cerr << p.var_info << endl; 
		}
	}
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//建立了SectionDatabase的临时变量

//int			cameraId;          yes
//struct tm		startTime;         yes
//struct tm		endTime;           yes
//unsigned long	overallImage;      yes 
//ObjectEnum	objType;

//unsigned int    color;
//unsigned int    otherP2;
//unsigned int    otherP3;
//unsigned int    otherP4;
//unsigned int    otherP5;

//unsigned int    otherA1;
//unsigned int    otherA2;
//unsigned int    otherA3;
//unsigned int    otherA4;
//unsigned int    otherA5;

void AddBasicInfo::OneLineComplete(const TrackLine & trackLine)
{
	//新建类推入队列
	SectionDatabase tmp;

	//camera id
	tmp.cameraId = m_obj->m_CAMERA_ID;

	//时间
	time_t videoStartTime = mktime(&(m_obj->m_VIDEO_START));
	time_t tmpTime1 = videoStartTime + (trackLine.line.begin())->frame_count / m_obj->GetVideoFps();
	time_t tmpTime2 = videoStartTime + (trackLine.line.end() - 1)->frame_count / m_obj->GetVideoFps();
	localtime_s(&(tmp.startTime), &tmpTime1);
	localtime_s(&(tmp.endTime), &tmpTime2);

	//overall image
	ObjectBox firstBox = *(trackLine.line.begin());
	int width = m_obj->GetVideoWidth();
	int height = m_obj->GetVideoHeight();
	//-深度复制数据
	IplImage *tmpImage = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);
	memcpy(tmpImage->imageData, trackLine.firstFrame, width * height * 3);
	//-画框
	cvRectangle(tmpImage, cvPoint(firstBox.x_pos, firstBox.y_pos), \
				cvPoint(firstBox.x_pos + firstBox.box_width, firstBox.y_pos + firstBox.box_height), cvScalar(255,0,0), 1);
	//-画轨迹线
	for (vector<ObjectBox>::const_iterator iterBox = trackLine.line.begin() + 1; iterBox != trackLine.line.end(); iterBox++)
	{
		cvLine(tmpImage, cvPoint((*(iterBox-1)).x_pos, (*(iterBox-1)).y_pos + (*(iterBox-1)).box_height), cvPoint((*iterBox).x_pos, (*iterBox).y_pos + (*iterBox).box_height), cvScalar(0,0,255));
	}
	//-保存图像
	char saveTime[64];
	srand(static_cast<unsigned int>(time(NULL)));
	string path;
	fstream fileToSave;
	do
	{
		sprintf_s(saveTime, 64, "%05d%05d", (unsigned int)(time(NULL)) % 65536, rand() % 65536);	//32位机器//32+16+1
		path = string("..\\Video\\overallimage\\") + string(saveTime) + string("0.jpg");			//序号的最后一个是0
		fileToSave.open(path.c_str(), ios::in);
	}while(fileToSave);
	fileToSave.close();
	cvSaveImage(path.c_str(), tmpImage);
	tmp.overallImage = (unsigned int)(atoi(saveTime));

	//push
	(m_obj->GetDataToStore()).push(tmp);
	cvReleaseImage(&tmpImage);
}


// ######################################################################
//输入的时候RGB要先保证是无符号类型的，不然有可能转换成负数
// T. Nathan Mundhenk
// mundhenk@usc.edu
// C/C++ Macro RGB to HSV
//Floats can be used in place of doubles. It depends on whether you want precision or speed. 
// The boolean value NORM is used to decide whether to output traditional HSV values where 
// 0 <= S <= 100 and 0 <= V <= 255. 
// Else we keep the values at a norm where

// 0 <= S <= 1 and 0 <= V <= 1. 

// The latter is faster for executing your own code, but the former should be used for compatibility.

// 0 <= 	Hue 	<= 360 
// 0 <= 	Sat 	<= 100 
// 0 <= 	Val 	<= 255
void ObtainColor::pixRGBtoHSVCommon(const double R, const double G, const double B, double& H, double& S, double& V, const bool NORM)
{
	if((B > G) && (B > R))                                                 
	{                                                                      
		V = B;                                                               
		if(V != 0)                                                           
		{                                                                    
			double min;                                                        
			if(R > G) min = G;                                                 
			else      min = R;                                                 
			const double delta = V - min;                                      
			if(delta != 0) { S = (delta/V); H = 4 + (R - G) / delta;}                      
			else { S = 0;         H = 4 + (R - G); }                              
			H *=   60; 
			if(H < 0) H += 360;                                     
			if(!NORM) V =  (V/255);                                            
			else      S *= (100);                                              
		}                                                                    
		else { S = 0; H = 0; }                                                   
	}                                                                      
	else if(G > R)                                                         
	{                                                                      
		V = G;                                                               
		if(V != 0)                                                           
		{                                                                    
			double min;                                                        
			if(R > B) min = B;                                                 
			else      min = R;                                                 
			const double delta = V - min;                                      
			if(delta != 0) {S = (delta/V); H = 2 + (B - R) / delta; }                      
			else {S = 0;         H = 2 + (B - R); }                              
			H *=   60; 
			if(H < 0) H += 360;                                     
			if(!NORM) V =  (V/255);                                            
			else      S *= (100);                                              
		}                                                                    
		else { S = 0; H = 0;}                                                   
	}                                                                      
	else                                                                   
	{                                                                      
		V = R;                                                               
		if(V != 0)                                                           
		{                                                                    
			double min;                                                        
			if(G > B) min = B;                                                 
			else      min = G;                                                 
			const double delta = V - min;                                      
			if(delta != 0) { S = (delta/V); H = (G - B) / delta; }                          
			else { S = 0;         H = (G - B); }                                  
			H *=   60; if(H < 0) H += 360;                                     
			if(!NORM) V =  (V/255);                                            
			else      S *= (100);                                              
		}                                                                    
		else { S = 0; H = 0;}                                                   
	}
}

//AddBasicInfo中已经建立了一个SectionDatabase的临时变量,然后push到了m_dataToStore中。
//所以这里去除队列的最后一个添加数据就OK
void ObtainColor::OneLineComplete(const TrackLine &trackLine)
{
	SectionDatabase &tmp = (m_obj->GetDataToStore()).back();	//获得数据库结构的引用
	ObjectBox firstBoxInLine = *trackLine.line.begin();			//路径上的第一个框

	//判断颜色的table(0表示黑色,1表示白色,2表示彩色)纵轴V，横轴S
	//V坐标是从下往上的
	//总共11行11列是因为范围是[0-1],最后一行和最后一列是重复了
	const unsigned char table[11][11] =	{
		{1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2},
		{1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2},
		{1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2},
		{1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2},
		{1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2},
		{0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2},
		{0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2},
		{0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
	};
	const unsigned char colorTable[37] = {	//红黄绿蓝红[0-360]
		0, 0,								 
		1, 1, 1, 1, 1,						 
		2, 2, 2, 2, 2, 2, 2, 2, 2, 2,		 
		3, 3, 3, 3, 3, 3, 3, 3, 3, 3,		 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0			
	};										

	int hsvClassify[4+1+1] = {0};		//红黄绿蓝  黑白
	int point = 0;
	unsigned int color = 0x00;

	for (int i = 0; i < firstBoxInLine.box_height * firstBoxInLine.box_width; i++)
	{
		if ((firstBoxInLine.data_mask)[i] != 0)
		{

			double h, s, v;
			pixRGBtoHSVCommon( (unsigned char)(firstBoxInLine.data_org)[3*i+2], (unsigned char)(firstBoxInLine.data_org)[3*i+1], (unsigned char)(firstBoxInLine.data_org)[3*i], h, s, v, 0);
			int tmpColor = table[(int)(10 - v*10)][(int)(s*10)];		//s方向,左上角[0-20)内都是白色,所以int(s*10) | v方向,反的,[0-20)都是黑色
			if (0 == tmpColor) hsvClassify[4]++;
			else if (1 == tmpColor) hsvClassify[5]++;
			else hsvClassify[colorTable[(int)(h/10)]]++;
			point++;
		}
	}
	point = (int)(point * 0.3);		//大于30%的区域
	for (int i = 0; i < 6; i++)
	{																//			白		黑		蓝		绿		黄		红
		if (hsvClassify[i] > point) color |= (0x01 << i);			//MSB 0  0	0		0		0		0		0		0  
	}
	tmp.color = color;
	//if (0x01 & color) printf("颜色：红\n");
	//if (0x02 & color) printf("颜色：黄\n");
	//if (0x04 & color) printf("颜色：绿\n");
	//if (0x08 & color) printf("颜色：蓝\n");
	//if (0x10 & color) printf("颜色：黑\n");
	//if (0x20 & color) printf("颜色：白\n");
	//printf("---------------------\n");
}



//加入人脸的框框
AddFaceBox::AddFaceBox(class ExtractFeature *obj)
{
	m_obj = obj;
	//初始化人脸识别模块
	m_faceObj = new FaceDetectorClass();
}


AddFaceBox::~AddFaceBox()
{
	delete m_faceObj;
}

void AddFaceBox::OneLineComplete(const TrackLine & trackLine)
{
	SectionDatabase &tmp = (m_obj->GetDataToStore()).back();	//获得数据库结构的引用
	int maxCount = 0, maxX = 0, maxY = 0, maxW = 0, maxH = 0, maxFX = 0, maxFY = 0, maxFW = 0, maxFH = 0;
	//遍历轨迹，得到最大的脸部的框\
	//这里是每一帧都监测，然后找出监测到的脸部最大的一帧。如果为了提高速度，可以先找出最大的物体框再监测一次
	int count = 0;
	for (vector<ObjectBox>::const_iterator iterBox = trackLine.line.begin(); iterBox != trackLine.line.end(); iterBox++, count++)
	{
		ObjectBox tmpBox = *iterBox;
		IplImage *tmpImage = cvCreateImage(cvSize(tmpBox.box_width, tmpBox.box_height), IPL_DEPTH_8U, 3);
		IplImage *tmpImageGray = cvCreateImage(cvSize(tmpBox.box_width, tmpBox.box_height), IPL_DEPTH_8U, 1);
		unsigned char *desStart = (unsigned char *)(tmpImage->imageData);
		unsigned char *srcStart = (unsigned char *)(tmpBox.data_org);
		for (int i = 0; i < tmpImage->height; i++)
		{
			memcpy(desStart, srcStart, tmpImage->width * 3);
			desStart += tmpImage->widthStep;
			srcStart += tmpImage->width * 3;
		}
		cvCvtColor(tmpImage,tmpImageGray,CV_BGR2GRAY);
		int num = m_faceObj->Detect(tmpImageGray, m_faces, 10);
		if (num > 0)
		{
			//只取第一个头像
			int tmpWidth = m_faces[0].rect.right - m_faces[0].rect.left;
			int tmpHeight = m_faces[0].rect.bottom - m_faces[0].rect.top;
			if (tmpWidth * tmpHeight > maxFW * maxFH)
			{
				maxCount = count, maxX = tmpBox.x_pos, maxY = tmpBox.y_pos, maxW = tmpBox.box_width, maxH = tmpBox.box_height, maxFW = tmpWidth, maxFH = tmpHeight;
			}
		}
		//printf("共有 %d 个头像\n", num);
		cvReleaseImage(&tmpImageGray);
		cvReleaseImage(&tmpImage);
	}
	//最后存数据
	if (maxCount > 0)
	{
		ObjectBox tmpBox = (trackLine.line)[maxCount];
		IplImage *tmpImage = cvCreateImage(cvSize(tmpBox.box_width, tmpBox.box_height), IPL_DEPTH_8U, 3);
		unsigned char *desStart = (unsigned char *)(tmpImage->imageData);
		unsigned char *srcStart = (unsigned char *)(tmpBox.data_org);
		for (int i = 0; i < tmpImage->height; i++)
		{
			memcpy(desStart, srcStart, tmpImage->width * 3);
			desStart += tmpImage->widthStep;
			srcStart += tmpImage->width * 3;
		}
		char saveFace[64];
		sprintf_s(saveFace, 64, "%010d", tmp.overallImage);
		string path = string("..\\Video\\faceimage\\") + string(saveFace) + string("1.jpg");			//序号的最后一个是1
		cvSaveImage(path.c_str(), tmpImage);
		cvReleaseImage(&tmpImage);
	}
	tmp.faceBoxInfo1 = maxCount;
	char faceInfo[20];
	sprintf_s(faceInfo, 20, "%05d%05d", maxX, maxY);
	tmp.faceBoxInfo2 = (unsigned int)(atoi(faceInfo));
	sprintf_s(faceInfo, 20, "%05d%05d", maxW, maxH);
	tmp.faceBoxInfo3 = (unsigned int)(atoi(faceInfo));
	sprintf_s(faceInfo, 20, "%05d%05d", maxFX, maxFY);
	tmp.faceBoxInfo4 = (unsigned int)(atoi(faceInfo));
	sprintf_s(faceInfo, 20, "%05d%05d", maxFW, maxFH);
	tmp.faceBoxInfo5 = (unsigned int)(atoi(faceInfo));
}