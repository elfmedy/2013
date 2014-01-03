#ifndef _EXTRACT_FEATURE_H_
#define _EXTRACT_FEATURE_H_

#include "iTrack.h"
#include "time.h"
#include <queue>
#include "Multiview Face Detector Class DLL.h"

#define OTL_ODBC_MSSQL_2005		//use odbc
#include "otlv4.h"				//OTL SQL2005 ODBC



//����
typedef struct ExtractFeaturePara_struct
{
	int m_NUM_TO_STORE;			//���ٸ��켣����д���ݿ�һ��
	int m_CAMERA_ID;			//�����ID
	struct tm m_VIDEO_START;	//��Ƶ��ʼ��ʱ��
}ExtractFeaturePara;

extern ExtractFeaturePara EXTRACTFEATURE_DEFAULT_PARA;

//�������
typedef enum ObjectType_enum
{
	OBJTYPE_PEOPLE,
	OBJTYPE_FACE,
	OBJTYPE_CAR,
	OBJTYPE_PLATE
}ObjectEnum;


//���������ļ����壬�������������õ����յġ���Ҫ���ŵ����ݿ���
typedef struct SectionDatabase_struct
{
	//basic
	unsigned int	cameraId;					//������
	struct tm		startTime;					//�켣��ʼʱ�� ������
	struct tm		endTime;					//�켣����ʱ�� ������
	unsigned int	overallImage;				//����ͼ��ı�ţ�����ͼ���ʱ�������0��face��1
	ObjectEnum		objType;					//��������

	//addition
	//-personal
	unsigned int    color;						//������������ɫ
												//��������ɫ����
	unsigned int    objBoxInfo1;				//--�����ı���1
	unsigned int    objBoxInfo2;				//--�����ı���2
	unsigned int    objBoxInfo3;				//--�����ı���3
	unsigned int    objBoxInfo4;				//--�����ı���4
												//����������� serialNum(4), body(x,y,w,h)(12), face(x,y,w,h)(12)
	unsigned int    faceBoxInfo1;				//--����������еĵڼ���
	unsigned int    faceBoxInfo2;				//--�������ͼƬ�е�x,yλ��
	unsigned int    faceBoxInfo3;				//--�������ͼƬ�е�width,height��С
	unsigned int    faceBoxInfo4;				//--��������������x,yλ��
	unsigned int    faceBoxInfo5;				//--��������������width,height��С
	//-action
}SectionDatabase;


//��ȡ��������
class ExtractFeature
{
public:
	ExtractFeature();
	~ExtractFeature();
	int Init(const char *fileName,											//��ʼ�������ø������ò������������ݿ�
		DlbkPara &dlbkPara = DLBK_DEFAULT_PARA,								//��ȡ������Ƶ����1����ʼ���켣����ʧ�ܷ���2���������ݿ�ʧ�ܷ���3
		VideoAnalysisPara &trackPara = VIDEOANALYSIS_DEFAULT_PARA, 
		ExtractFeaturePara &featurePara = EXTRACTFEATURE_DEFAULT_PARA);
	void Process(const IplImage *pCaptureFrame);							//����
	void Store();															//�������ݵ����ݿ�

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
	void Tm2DateTime(struct tm &oTime, otl_datetime &dTime);	//tmת��Ϊotl_datatime

protected:	
	//trackobj
	IVideoAnalysis *m_videoAnalysisObj;			//��ȡ�켣�����ָ��
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


//�����������Ϣ
//����ͷ�ţ���ʼ��ֹʱ�䣬ͼƬ
class AddBasicInfo : public TrackObserver
{
public:
	explicit AddBasicInfo(ExtractFeature *obj){m_obj = obj;};
	~AddBasicInfo(){}
	void OneLineComplete(const TrackLine & trackLine);
protected:
	ExtractFeature *m_obj;
};


//��ȡ��ɫ
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



//����ͷ��Ŀ��
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