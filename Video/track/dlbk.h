#ifndef _DLBKDLL_H_
#define _DLBKDLL_H_

/*
---example----
pCapture = cvCaptureFromFile("highwayI_raw.AVI");
Cdlbk testModule(3, 40, 5, 25, 0, 0.08, 15);	//1.����ģ�͵Ķ���

while(pFrame = cvQueryFrame( pCapture ))
{
	testModule.Process(pFrame->width, pFrame->height, (unsigned char *)pFrame->imageData);	//2.����ģ��
	//testModule.pimgseg  �����ǰ����image
	cvWaitKey(10);
}
*/

#include "highgui.h"
#include "cxcore.h"
#include "cv.h"
#include <memory.h>
#include "math.h"

//#define DISP				//debugģʽ�¿��Կ���������


//����
typedef struct cdlbk_para_struct
{
	int           w;//3		//&ͨ��
	int           x;//40	//-����ʱ�ļ���ֵ(���ٸ��£�֡�����С��aʱx�ݼ�����0���¡���ͨ���£��ж�֡��Ϊ0֮��x�ݼ�����0�͸���)
	int           y;//5		//-֡�����ڴ�ֵ��ʱ��ᱻ������fk�˵�Ϊ1������Ϊ0
	int           z;//25	//*������ֵ�����ڴ���ֵ����Ϊǰ��
	double        a;//0		//-���ٸ��£����֡��Ϊ1�ĵ����ĿС�����ֵ�Ļ�����ô�ӿ��ٶȸ��±�����(Ҫ����x֮֡��Ż�)
	double        b;//0.08	//-���ٸ��µ�����
	int			  filter;	//$0������ʽʱ���˲��Ĵ�С ��ֵ�˲�ʱ�Ĳ����� (2*(sqrt(filter)+1)^2�ķ��顣�������ȼ�
	int           isShadow;	//#�ǲ���ȥ��Ӱ�ӣ�1ȥ����0��ȥ��
	int           ft;		//$���ȥ��С��������˲���ʽ��ѡ0��ʾ����������ѡ1��ʾ������ֵ
}DlbkPara;

extern DlbkPara DLBK_DEFAULT_PARA;

class Cdlbk
{
private:
	void Init(int mWid, int mHei, unsigned char* pInput);
	int	 UpdateBK(int mWid, int mHei, unsigned char* pInput);
	void UpdateSeg(int mWid, int mHei, unsigned char* pInput);

protected:
	int    SRC_CHANNEL;		//channel number
	int    T_STABLE;		//stable frames
	double QUICK_UPDATE;        
	double ALPHA;			//learning ratio
	int    FRAME_THRESHOLD;	//threshold for frame level
	int    BK_THRESHOLD;	//threshold for pixel level
	int    FILTER_SIZE;
	int    IS_SHADOW;
	int    FT;

protected:
	void ShadowSub(unsigned char* pIn, unsigned char* pBK, unsigned char* pSeg, int w, int h);
	int  DispImg(unsigned char* img, int w, int h, int channel, const char* name, int pos);
	void Connect_Filter(int mWid, int mHei, unsigned char* pInput);
	void SILTP_Filter(int mWid, int mHei, unsigned char *pSeg, unsigned char * fg, unsigned char * bk);

public:
	explicit Cdlbk(DlbkPara &para = DLBK_DEFAULT_PARA);
	~Cdlbk();

	void Process(int mWid, int mHei, unsigned char* pInput);
	unsigned char const *GetBack() const{return pBK;}
	unsigned char const *GetFore() const{return pSeg;}


public:
	unsigned char* pBK;//background
	unsigned char* pSeg;//segmentation
	unsigned char* pDK;//dynamic matrix
	unsigned char* pFK;//frame to frame difference
	unsigned char* pLIN;//frame last input
	
	//------------------------------
	//for shadow sub
	IplImage* ptemin ;	
	IplImage* ptembk ;	
	IplImage* ptemhsv;
	IplImage* pVin   ;
	IplImage* pVbk   ;

	//------------------------------
	//for cal contour
	//IplImage* pimgseg;
};

#endif
