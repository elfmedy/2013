//==============================================================================
//Author:
//    Michael.Z . 2010-10 . IVA
//Description:
//    Double level background modeling[1];
//    HSV[2] & SILTP[3] shadow subtraction;
//    Fast slip connect-filter    
//Reference:
//    [1]Tao Yang, Stan Z. Li, Quan Pan, Jing Li, "Real-Time Multiple Objects Tracking 
//    with Occlusion Handling in Dynamic Scenes," cvpr, vol. 1, pp.970-975, 
//    2005 IEEE Computer Society Conference on Computer Vision and Pattern Recognition 
//    (CVPR'05) - Volume 1, 2005
//
//    [2]Rita Cucchiara, Costantino Grana, Massimo Piccardi, Andrea Prati, 
//    "Detecting Moving Objects, Ghosts, and Shadows in Video Streams,"IEEE Transactions 
//    on Pattern Analysis and Machine Intelligence, vol. 25, no. 10, pp. 1337-1342, 
//    Oct. 2003, doi:10.1109/TPAMI.2003.1233909 
//
//    [3]Shengcai Liao, Guoying Zhao, Stan Z. Li, Vili Kellokumpu and Matti Pietik¨ainen. 
//    "Modeling Pixel Process with Scale Invariant Local Patterns for Background 
//    Subtraction in Complex Scenes", In Proceedings of IEEE International Conference on 
//    Computer Vision and Pattern Recognition, (CVPR2010), pp.1301-1306,2010. 
//

//-------------------------------------------------------------------------------
// 2011.12.26 不是苹果 
// 修改：替换掉滤波，加入去除一定像素轮廓
//-------------------------------------------------------------------------------

#include "dlbk.h"
#include "math.h"

DlbkPara DLBK_DEFAULT_PARA = {
	3,			//3通道
	40,			//stable 40
	5,			//帧差5
	25,			//阈值25
	0,
	0.08,
	9,
	0,			//不处理阴影
	1			//采用中值滤波
};

//==============================================================================
Cdlbk::Cdlbk(DlbkPara &para)
{
	SRC_CHANNEL = para.w; 
	T_STABLE = para.x;
	FRAME_THRESHOLD = para.y;
	BK_THRESHOLD = para.z;
	QUICK_UPDATE = para.a;
	ALPHA = para.b;
	FILTER_SIZE = para.filter;
	IS_SHADOW = para.isShadow;
	FT = para.ft;

	pBK = NULL;
	pSeg = NULL;
	pDK = NULL;
	pFK = NULL;
	pLIN = NULL;
	ptembk = NULL;
	ptemin = NULL;
	ptemhsv = NULL;
	pVin = NULL;
	pVbk = NULL;
	//pimgseg = NULL;
}


//==============================================================================
//Initiate memory space
void Cdlbk::Init(int mWid, int mHei, unsigned char* pInput)
{	
	pBK  = new unsigned char[mWid*mHei*SRC_CHANNEL];
	pSeg = new unsigned char[mWid*mHei];
	pDK  = new unsigned char[mWid*mHei];
	pFK  = new unsigned char[mWid*mHei];
	pLIN = new unsigned char[mWid*mHei*SRC_CHANNEL];

    for(int i=0;i<mWid*mHei*SRC_CHANNEL;i++) 
	{
		pBK[i] = pInput[i];//initialize background		
		pLIN[i] = pInput[i];
	}

	memset(pSeg,0,sizeof(unsigned char)*mWid*mHei);
	memset(pDK,T_STABLE,sizeof(unsigned char)*mWid*mHei);
	memset(pFK,0,sizeof(unsigned char)*mWid*mHei);

	//----------------------------------------
	//for shadow sub
	ptemin = cvCreateImage(cvSize(mWid,mHei), 8, 3 );	
	ptembk = cvCreateImage(cvSize(mWid,mHei), 8, 3 );	
	ptemhsv = cvCreateImage(cvSize(mWid,mHei), 8, 3 );
	pVin = cvCreateImage(cvSize(mWid,mHei), 8, 1 );
	pVbk = cvCreateImage(cvSize(mWid,mHei), 8, 1 );

	//----------------------------------------
	//for cal contour
	//pimgseg = cvCreateImage( cvSize(mWid,mHei), 8, 1 );
	//pimgseg->origin = 1;
}

//==============================================================================
//release memory
Cdlbk::~Cdlbk()
{
	if(pBK != NULL)delete [] pBK;
	if(pSeg != NULL)delete [] pSeg;
	if(pDK != NULL)delete [] pDK;
	if(pFK != NULL)delete [] pFK;
	if(pLIN != NULL)delete [] pLIN;
	if(ptembk != NULL)cvReleaseImage(&ptembk);
	if(ptemin != NULL)cvReleaseImage(&ptemin);
	if(ptemhsv != NULL)cvReleaseImage(&ptemhsv);
	if(pVin != NULL)cvReleaseImage(&pVin);
	if(pVbk != NULL)cvReleaseImage(&pVbk);
	//if(pimgseg != NULL)cvReleaseImage(&pimgseg);
}

//==============================================================================
int Cdlbk::UpdateBK(int mWid, int mHei, unsigned char* pInput)
{
	unsigned int sum = 0;
	int i = 0;
	int j = 0;
	int tem_max = 0;
	int tem_data=0;
	//-----------------------------------------------------------
	//calculate FK
	for(i=0;i<mWid*mHei;i++) 
	{
		tem_max = 0;	
		for(j=0;j<SRC_CHANNEL;j++)
		{		
			tem_data = abs(pInput[SRC_CHANNEL*i+j] - pLIN[SRC_CHANNEL*i+j]);
			if(tem_data > tem_max)
				tem_max = tem_data;
			pFK[i]=tem_max;
			pLIN[SRC_CHANNEL*i+j] = pInput[SRC_CHANNEL*i+j];
		}			
		pFK[i] = (pFK[i]<=FRAME_THRESHOLD) ? 0 : 1;
		sum+=pFK[i];		
	}

	//-----------------------------------------------------------
	//calculate V to detect big-size-change of foreground
	//float v = (float)sum/(mWid*mHei);
	static int f_stable = T_STABLE;
	if(sum<QUICK_UPDATE)
	{
		if(f_stable != 0)
			f_stable--;	
		else
		{
			f_stable = T_STABLE;
			for(i=0;i<mWid*mHei*SRC_CHANNEL;i++) 
			{
				pBK[i]=(unsigned char)(0.08*pInput[i] + (1-0.08)*pBK[i]);//update background		
			}		
			return 0;
		}			
	}
	else
	{
		f_stable = T_STABLE;
	}

	//-----------------------------------------------------------
	//evaluate DK to update background
	
	for(i=0;i<mWid*mHei;i++) 
	{
		if(pFK[i]==0 && pDK[i]!=0)
		{
			pDK[i] -= 1;
			
		}
		else if(pFK[i]!=0)
		{
			pDK[i] = T_STABLE;
		}
		else if(pFK[i]==0 && pDK[i] == 0)
		{	
			for(j=0;j<SRC_CHANNEL;j++)
			{
				pBK[SRC_CHANNEL*i+j]=(unsigned char)(ALPHA*pInput[SRC_CHANNEL*i+j] + (1-ALPHA)*pBK[SRC_CHANNEL*i+j]);	
			}					
		}		
	}

	return 1;
}

//==============================================================================
void Cdlbk::UpdateSeg(int mWid, int mHei, unsigned char* pInput)
{
	//-----------------------------------------------------------
	//update foreground
	int tem_data = 0;
	int tem_max = 0;
	memset(pSeg,0,sizeof(unsigned char)*mWid*mHei);
	for(int i=0;i<mWid*mHei;i++) 
	{
		tem_max = 0;
		for(int j=0;j<SRC_CHANNEL;j++)
		{		
			tem_data = abs(pLIN[SRC_CHANNEL*i+j] - pBK[SRC_CHANNEL*i+j]);
			if(tem_data > tem_max)
				tem_max = tem_data;
		}			
		pSeg[i] = (tem_max <= BK_THRESHOLD) ? 0 : 255;
	}
}

//==============================================================================
void Cdlbk::Process(int mWid, int mHei, unsigned char* pInput)
{
	static bool initflg = false;
	if (initflg==false)
	{
		Init(mWid, mHei, pInput);
		initflg = true;
	}
	else
	{
		UpdateBK(mWid,mHei,pInput);
		UpdateSeg(mWid,mHei,pInput);           
		#ifdef DISP
			DispImg(pInput, mWid, mHei, 3, "original", 0);
			DispImg(pSeg, mWid, mHei, 1, "mainseg", 1);
			DispImg(pBK, mWid, mHei, 3, "BK", 2);
		#endif	
		
		if (IS_SHADOW)
		{
			ShadowSub(pInput,pBK,pSeg,mWid,mHei);
			#ifdef DISP
				DispImg(pSeg, mWid, mHei, 1, "shadowsub", 3);
			#endif		
		}
		
		Connect_Filter(mWid,mHei,pSeg);
		#ifdef DISP
			DispImg(pSeg, mWid, mHei, 1, "median2", 4);		
		#endif
		//memcpy(pimgseg->imageData,pSeg,sizeof(uchar)*mWid*mHei);
	}
}

//==============================================================================
//Binary image median filtering with a (2ts+1)^2 mask
void Cdlbk::Connect_Filter(int mWid, int mHei, unsigned char* pInput)
{	
	
	if (FT)
	{
		float flg_f=float(0.3);
		int ts = (int)(sqrt((float)FILTER_SIZE));
		unsigned int mask=0;
		unsigned int tem_sum=0;
		int i,j,m,n,x1,x2,y1,y2;
		unsigned char* ptem = new unsigned char[mWid*mHei];
		memset(ptem,0,sizeof(unsigned char)*mWid*mHei);
		unsigned char* pslipmast = new unsigned char[2*ts+1];
		memset(pslipmast,0,sizeof(unsigned char)*(2*ts+1));

		for( i=0; i<mHei; i+=1)
		{
			for( j=0; j<mWid; j+=1)
			{
				x1 = ( (j-ts) < 0    ) ? 0    : (j-ts);
				x2 = ( (j+ts) > mWid ) ? mWid : (j+ts);
				y1 = ( (i-ts) < 0    ) ? 0    : (i-ts);
				y2 = ( (i+ts) > mHei ) ? mHei : (i+ts);
				mask = (x2-x1+1)*(y2-y1+1);
				tem_sum=0;

				//for (n=y1; n<=y2; n++)
				//for (m=x1; m<=x2; m++)
				//	tem_sum+=pInput[n*mWid+m]/255;					
				//
				//float rst = (float)tem_sum/mask;
				//if(rst>flg_f)
				//ptem[i*mWid+j]=255;	
				
				if( mask!=(ts+ts+1)*(ts+ts+1) )
				{
					for (n=y1; n<=y2; n++)
					for (m=x1; m<=x2; m++)
						tem_sum+=pInput[n*mWid+m]/255;					

					float rst = (float)tem_sum/mask;
					if(rst>flg_f)
					ptem[i*mWid+j]=255;	
				}
				else
				{
					if(x1==0)//new row
					{
						for (m=x1; m<=x2; m++)
						{
							pslipmast[m] = 0;						
							for (n=y1; n<=y2; n++)					//cal every pslipmast element
								pslipmast[m]+=pInput[n*mWid+m]/255;					
						}
						
						tem_sum = 0;								//cal rst
						for (int k=0; k<=x2-x1; k++)
							tem_sum += pslipmast[k];
						float rst = (float)tem_sum/mask;
						if(rst>flg_f)
							ptem[i*mWid+j]=255;	
					}
					else
					{
						for (int q=0; q<x2-x1; q++)					 //slip buffer 
							pslipmast[q] = pslipmast[q+1];

						m = x2;										//cal last element of the slip buffer
						pslipmast[x2-x1]=0;
						for (n=y1; n<=y2; n++)
							pslipmast[x2-x1] += pInput[n*mWid+m]/255;					
						
						tem_sum = 0;								//cal rst
						for (int k=0; k<=x2-x1; k++)
							tem_sum += pslipmast[k];
						float rst = (float)tem_sum/mask;
						if(rst>flg_f)
							ptem[i*mWid+j]=255;	
					}			
				}						
			}
		}

		//update input data
		memcpy(pInput,ptem,sizeof(unsigned char)*mWid*mHei);
		delete [] ptem;
		delete [] pslipmast;
	}
	else
	{
		IplImage* pFore = cvCreateImageHeader(cvSize(mWid,mHei), 8, 1);
		cvSetData(pFore, pInput, mWid);	//4倍宽度
		CvMemStorage* storage = cvCreateMemStorage(0);
		CvSeq *first_seq = NULL, *prev_seq = NULL, *seq = NULL;
		cvFindContours( pFore, storage, &first_seq, sizeof(CvContour), CV_RETR_LIST );
		for (seq = first_seq; seq; seq = seq->h_next)
		{
			CvContour* cnt = (CvContour*)seq;
			double area = cvContourArea( cnt, CV_WHOLE_SEQ );
			if (fabs(area) <= FILTER_SIZE)
			{
				prev_seq = seq->h_prev;
				if( prev_seq )
				{
					prev_seq->h_next = seq->h_next;
					if( seq->h_next ) seq->h_next->h_prev = prev_seq;
				}
				else
				{
					first_seq = seq->h_next;
					if( seq->h_next ) seq->h_next->h_prev = NULL;
				}
			}
		}
		cvZero(pFore);
		cvDrawContours(pFore, first_seq, CV_RGB(255, 255, 255), CV_RGB(255, 255, 255), 10, -1);
		cvReleaseImageHeader(&pFore);
		cvReleaseMemStorage(&storage);
	}
}

//==============================================================================
//for test
int Cdlbk::DispImg(unsigned char* img, int w, int h, int channel, const char* name, int pos)
{
	IplImage* pOutput = cvCreateImage( cvSize(w, h), IPL_DEPTH_8U, channel );
	memcpy(pOutput->imageData, img, sizeof(uchar)*w*h*channel);
	//pOutput->origin = 1;
	cvNamedWindow(name, 1);
	int x,y;
	x=(pos*w)%1280;
	y=(pos*w)/1280;
	//cvMoveWindow(name,x,pos*y);
	cvShowImage(name, pOutput);
	cvWaitKey(1);
	cvReleaseImage(&pOutput);
	return 1;
}

//==============================================================================
void Cdlbk::ShadowSub(unsigned char* pIn, unsigned char* pBK, unsigned char* pSeg, int w, int h)
{	
	memcpy((unsigned char*)(ptemin->imageData),pIn,sizeof(unsigned char)*w*h*3);
	memcpy((unsigned char*)(ptembk->imageData),pBK,sizeof(unsigned char)*w*h*3);
	
	cvCvtColor( ptemin, ptemhsv, CV_RGB2HSV);//CV_BGR2HLS
	cvSplit( ptemhsv, 0, 0, pVin, 0 );
	pVin->origin =1;
	cvCvtColor( ptembk, ptemhsv, CV_RGB2HSV);//CV_BGR2HLS
	cvSplit( ptemhsv, 0, 0, pVbk, 0 );
	pVbk->origin =1;
		
	for(int i=0; i<h; i++)
	{
		for(int j=0; j<w; j++)
		{
			if( ( (float)(((unsigned char*)(pVin->imageData))[i*w+j])/((unsigned char*)(pVbk->imageData))[i*w+j]>=0.3) 
			  &&( (float)(((unsigned char*)(pVin->imageData))[i*w+j])/((unsigned char*)(pVbk->imageData))[i*w+j]<=1)
			  //&&((pHin[i*w+j]-pHbk[i*w+j])>0)
			  //&&((pSin[i*w+j]-pSbk[i*w+j])>0.1)
			  &&(pSeg[i*w+j]!=0) )
			{
				pSeg[i*w+j] = 128;
			}
		}
	}
#ifdef DISP
	DispImg(pSeg, w, h, 1, "hsvsub", 5); 
#endif
	SILTP_Filter(w, h, pSeg, (uchar*)(pVin->imageData), (uchar*)(pVbk->imageData));
}

//==============================================================================
void Cdlbk::SILTP_Filter(int mWid, int mHei, unsigned char *pSeg, unsigned char * fg, unsigned char * bk)
{	
	float T = (float)(0.1);	
	int ts = 3;//
	int differ=0;
	int feature=0;
    unsigned int mask=0;
	unsigned int border=0;
	int ltp_fg,ltp_bk;
	int i,j,m,n,x1,x2,y1,y2;
	unsigned char* ptem = new unsigned char[mWid*mHei];
	memset(ptem,0,sizeof(unsigned char)*mWid*mHei);
	
	for( i=0; i<mHei; i+=1)
	{
		for( j=0; j<mWid; j+=1)
		{
		    
			x1 = ( 0 > (j-ts) ) ? 0 : (j-ts);
			x2 = ( (j+ts) < mWid ) ? (j+ts) : mWid;
			y1 = ( 0 > (i-ts) ) ? 0 : (i-ts);
			y2 = ( (i+ts) < mHei ) ? (i+ts) : mHei;
							 
			//compute shadow area only
			if(128 == pSeg[i*mWid+j])
			{
				
				border = 0;
				mask = 0;
				for (n=y1; n<=y2; n++)
				{
					for (m=x1; m<=x2; m++)
					{
						mask++;
						if(pSeg[n*mWid+m]==0)
							border++;
					}
				}
				if (((float)border/mask)<0.1) //not shadow border  
				{
					differ = 0;		
					mask = 0;
					feature=0;
					for (n=y1; n<=y2; n++)
					{
						for (m=x1; m<=x2; m++)
						{
							//fg LTP
							if(fg[n*mWid+m] > (1+T)*fg[i*mWid+j])
								ltp_fg = 1;
							else if(fg[n*mWid+m] < (1-T)*fg[i*mWid+j])
								ltp_fg = -1;
							else
								ltp_fg = 0;
							
							//bk LTP
							if(bk[n*mWid+m] > (1+T)*bk[i*mWid+j])
								ltp_bk = 1;
							else if(bk[n*mWid+m] < (1-T)*bk[i*mWid+j])
								ltp_bk = -1;
							else
								ltp_bk = 0;
							
							differ += abs(ltp_bk-ltp_fg);
							feature += abs(ltp_fg);
							mask+=2;//max difference
						}//end for
					}//end for
					
					if(((float)feature/mask) > 0.3 &&   //high LTP，means not heavy shadow area
						 ((float)differ/mask) > 0.2)//
						ptem[i*mWid+j]=255;//recover shadow to foreground
						
					//else
					//ptem[i*mWid+j]=128;
				}
							
			}//end if	
			else if (255 == pSeg[i*mWid+j])//foreground unchanged
			{
				ptem[i*mWid+j]=255;
			}
		}//end for
	}//end for
	
	//update input data
	memcpy(pSeg,ptem,sizeof(unsigned char)*mWid*mHei);
	delete [] ptem;
}

//end of file
//==============================================================================
