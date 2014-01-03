#ifndef _DLBK_H_
#define _DLBK_H_

#include "IBackModel.h"
#include "cxcore.h"
#include "cv.h"

//dlbk背景模型参数
struct DlbkPara
{
	int           videoWidth;
	int           videoHeight;
	int           w;//3		//&通道
	int           x;//40	//-更新时的计数值(快速更新：帧差点数小于a时x递减，到0更新。普通更新：判断帧差为0之后x递减，到0就更新)
	int           y;//5		//-帧间差大于此值的时候会被判数组fk此点为1，否则为0
	int           z;//25	//*背景阈值，大于此阈值被判为前景
	double        a;//0		//-快速更新，如果帧差为1的点的数目小于这个值的话，那么加快速度更新背景。(要经过x帧之后才会)
	double        b;//0.08	//-快速更新的速率
	int			  filter;	//$0轮廓方式时的滤波的大小 中值滤波时的参数是 (2*(sqrt(filter)+1)^2的方块。参数不等价
	int           isShadow;	//#是不是去除影子，1去除，0不去除
	int           ft;		//$最后去除小块区域的滤波方式，选0表示采用轮廓，选1表示采用中值
};

//默认参数
extern struct DlbkPara DLBK_DEFAULT_PARA;

//dlbk背景模型类
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
