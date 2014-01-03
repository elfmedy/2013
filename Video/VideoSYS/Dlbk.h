#ifndef _DLBK_H_
#define _DLBK_H_

#include "IBackModel.h"
#include "cxcore.h"
#include "cv.h"

//dlbk����ģ�Ͳ���
struct DlbkPara
{
	int           videoWidth;
	int           videoHeight;
	int           w;//3		//&ͨ��
	int           x;//40	//-����ʱ�ļ���ֵ(���ٸ��£�֡�����С��aʱx�ݼ�����0���¡���ͨ���£��ж�֡��Ϊ0֮��x�ݼ�����0�͸���)
	int           y;//5		//-֡�����ڴ�ֵ��ʱ��ᱻ������fk�˵�Ϊ1������Ϊ0
	int           z;//25	//*������ֵ�����ڴ���ֵ����Ϊǰ��
	double        a;//0		//-���ٸ��£����֡��Ϊ1�ĵ����ĿС�����ֵ�Ļ�����ô�ӿ��ٶȸ��±�����(Ҫ����x֮֡��Ż�)
	double        b;//0.08	//-���ٸ��µ�����
	int			  filter;	//$0������ʽʱ���˲��Ĵ�С ��ֵ�˲�ʱ�Ĳ����� (2*(sqrt(filter)+1)^2�ķ��顣�������ȼ�
	int           isShadow;	//#�ǲ���ȥ��Ӱ�ӣ�1ȥ����0��ȥ��
	int           ft;		//$���ȥ��С��������˲���ʽ��ѡ0��ʾ����������ѡ1��ʾ������ֵ
};

//Ĭ�ϲ���
extern struct DlbkPara DLBK_DEFAULT_PARA;

//dlbk����ģ����
class Dlbk : public IBackModel
{
public:
	Dlbk();
	~Dlbk();
    int Init(void *para);
    void Process(unsigned char *pInput);
	const unsigned char *GetBack() { return pBK; }
	const unsigned char *GetMask() { return pSeg; }

private:
	void OInit(int mWid, int mHei, unsigned char* pInput);
	int	 UpdateBK(int mWid, int mHei, unsigned char* pInput);
	void UpdateSeg(int mWid, int mHei, unsigned char* pInput);

private:
	int    SRC_CHANNEL;		//channel number
	int    T_STABLE;		//stable frames
	double QUICK_UPDATE;        
	double ALPHA;			//learning ratio
	int    FRAME_THRESHOLD;	//threshold for frame level
	int    BK_THRESHOLD;	//threshold for pixel level
	int    FILTER_SIZE;
	int    IS_SHADOW;
	int    FT;

	int videoWidth;;
	int videoHeight;

private:
	void ShadowSub(unsigned char* pIn, unsigned char* pBK, unsigned char* pSeg, int w, int h);
	int  DispImg(unsigned char* img, int w, int h, int channel, const char* name, int pos);
	void Connect_Filter(int mWid, int mHei, unsigned char* pInput);
	void SILTP_Filter(int mWid, int mHei, unsigned char *pSeg, unsigned char * fg, unsigned char * bk);

private:
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
};

#endif
