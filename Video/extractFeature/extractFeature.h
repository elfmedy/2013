#ifndef _EXTRACT_FEATURE_H_
#define _EXTRACT_FEATURE_H_

#include "iTrack.h"
#include "time.h"
#include <queue>
#include "Multiview Face Detector Class DLL.h"

#define OTL_ODBC_MSSQL_2005		//use odbc
#include "otlv4.h"				//OTL SQL2005 ODBC



//参数
typedef struct ExtractFeaturePara_struct
{
	int m_NUM_TO_STORE;			//多少个轨迹数据写数据库一次
	int m_CAMERA_ID;			//摄像机ID
	struct tm m_VIDEO_START;	//视频开始的时间
}ExtractFeaturePara;

extern ExtractFeaturePara EXTRACTFEATURE_DEFAULT_PARA;

//物体分类
typedef enum ObjectType_enum
{
	OBJTYPE_PEOPLE,
	OBJTYPE_FACE,
	OBJTYPE_CAR,
	OBJTYPE_PLATE
}ObjectEnum;


//各种特征的集合体，各个函数操作得到最终的。将要被放到数据库中
typedef struct SectionDatabase_struct
{
	//basic
	unsigned int	cameraId;					//相机编号
	struct tm		startTime;					//轨迹开始时间 年月日
	struct tm		endTime;					//轨迹结束时间 年月日
	unsigned int	overallImage;				//概括图像的编号，保存图像的时候这个带0，face带1
	ObjectEnum		objType;					//物体类型

	//addition
	//-personal
	unsigned int    color;						//物体框的整体颜色
												//物体框的颜色编码
	unsigned int    objBoxInfo1;				//--物体框的编码1
	unsigned int    objBoxInfo2;				//--物体框的编码2
	unsigned int    objBoxInfo3;				//--物体框的编码3
	unsigned int    objBoxInfo4;				//--物体框的编码4
												//脸的相关数据 serialNum(4), body(x,y,w,h)(12), face(x,y,w,h)(12)
	unsigned int    faceBoxInfo1;				//--脸在这个序列的第几个
	unsigned int    faceBoxInfo2;				//--物体框在图片中的x,y位置
	unsigned int    faceBoxInfo3;				//--物体框在图片中的width,height大小
	unsigned int    faceBoxInfo4;				//--脸部框在物体框的x,y位置
	unsigned int    faceBoxInfo5;				//--脸部框在物体框的width,height大小
	//-action
}SectionDatabase;


//获取特征的类
class ExtractFeature
{
public:
	ExtractFeature();
	~ExtractFeature();
	int Init(const char *fileName,											//初始化，设置各种配置参数，连接数据库
		DlbkPara &dlbkPara = DLBK_DEFAULT_PARA,								//读取不到视频返回1，初始化轨迹对象失败返回2，连接数据库失败返回3
		VideoAnalysisPara &trackPara = VIDEOANALYSIS_DEFAULT_PARA, 
		ExtractFeaturePara &featurePara = EXTRACTFEATURE_DEFAULT_PARA);
	void Process(const IplImage *pCaptureFrame);							//处理
	void Store();															//储存数据到数据库

public:
	std::queue<SectionDatabase> & GetDataToStore() {return m_dataToStore;}
	int GetVideoWidth() const{return m_videoWidth;}
	int GetVideoHeight() const{return m_videoHeight;}
	int GetVideoFps() const{return m_videoFps;}

public:
	//para
	int m_NUM_TO_STORE;			
	int m_CAMERA_ID;			
	struct tm m_VIDEO_START;

protected:
	void Tm2DateTime(struct tm &oTime, otl_datetime &dTime);	//tm转换为otl_datatime

protected:	
	//trackobj
	IVideoAnalysis *m_videoAnalysisObj;			//提取轨迹对象的指针
	//basic
	int m_videoWidth;
	int m_videoHeight;
	int m_videoFps;
	//odbc
	otl_connect m_db;
	//datatostore
	std::queue<SectionDatabase> m_dataToStore;
	//trackobserver
	std::vector<TrackObserver *> m_ptr2Observer;

private:
	CvCapture *m_tempCapture;
};


//加入基本的信息
//摄像头号，起始终止时间，图片
class AddBasicInfo : public TrackObserver
{
public:
	explicit AddBasicInfo(ExtractFeature *obj){m_obj = obj;};
	~AddBasicInfo(){}
	void OneLineComplete(const TrackLine & trackLine);
protected:
	ExtractFeature *m_obj;
};


//获取颜色
class ObtainColor : public TrackObserver
{
public:
	explicit ObtainColor(ExtractFeature *obj){m_obj = obj;};
	~ObtainColor(){}
	void OneLineComplete(const TrackLine & trackLine);
	void pixRGBtoHSVCommon(const double R, const double G, const double B, double& H, double& S, double& V, const bool NORM);
protected:
	ExtractFeature *m_obj;
};



//加入头像的框框
class AddFaceBox : public TrackObserver
{
public:
	explicit AddFaceBox(class ExtractFeature *obj);
	~AddFaceBox();
	void OneLineComplete(const TrackLine & trackLine);
protected:
	ExtractFeature *m_obj;
	FaceDetectorClass *m_faceObj;
	SFaceItem m_faces[10];
};


#endif