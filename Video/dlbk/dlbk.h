#ifndef _DLBKDLL_H_
#define _DLBKDLL_H_

/*
---example----
pCapture = cvCaptureFromFile("highwayI_raw.AVI");
Cdlbk testModule(3, 40, 5, 25, 0, 0.08, 15);	//1.建立模型的对象

while(pFrame = cvQueryFrame( pCapture ))
{
	testModule.Process(pFrame->width, pFrame->height, (unsigned char *)pFrame->imageData);	//2.更新模型
	//testModule.pimgseg  这个是前景的image
	cvWaitKey(10);
}
*/

#include "highgui.h"
#include "cxcore.h"
#include "cv.h"

#include <memory.h>
#include "math.h"

//#define DISP				//debug模式下可以开启，调试

#ifdef DLBK_EXPORTS
#define DLBK_API __declspec(dllexport)
#else
#define DLBK_API __declspec(dllimport)
#endif

//参数
typedef struct cdlbk_para_struct
{
	unsigned char w;//3		//&通道
	unsigned char x;//40	//-更新时的计数值(快速更新：帧差点数小于a时x递减，到0更新。普通更新：判断帧差为0之后x递减，到0就更新)
	unsigned char y;//5		//-帧间差大于此值的时候会被判数组fk此点为1，否则为0
	unsigned char z;//25	//*背景阈值，大于此阈值被判为前景
	double        a;//0		//-快速更新，如果帧差为1的点的数目小于这个值的话，那么加快速度更新背景。(要经过x帧之后才会)
	double        b;//0.08	//-快速更新的速率
	unsigned char filter;	//$0轮廓方式时的滤波的大小 中值滤波时的参数是 (2*(sqrt(filter)+1)^2的方块。参数不等价
	unsigned char isShadow;	//#是不是去除影子，1去除，0不去除
	unsigned char ft;		//$最后去除小块区域的滤波方式，选0表示采用轮廓，选1表示采用中值
}DlbkPara;

extern DLBK_API DlbkPara DLBK_DEFAULT_PARA;

class DLBK_API Cdlbk
{
private:
	void Init(int mWid, int mHei, unsigned char* pInput);
	int	 UpdateBK(int mWid, int mHei, unsigned char* pInput);
	void UpdateSeg(int mWid, int mHei, unsigned char* pInput);

protected:
	unsigned char SRC_CHANNEL;  //channel number
	unsigned char T_STABLE;     //stable frames
	double QUICK_UPDATE;        
	double ALPHA;				//learning ratio
	unsigned char FRAME_THRESHOLD;//threshold for frame level
	unsigned char BK_THRESHOLD;   //threshold for pixel level
	unsigned char FILTER_SIZE;
	unsigned char IS_SHADOW;
	unsigned char FT;

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

