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

#include "Dlbk.h"
#include "math.h"

struct DlbkPara DLBK_DEFAULT_PARA = {
	0,			//width
	0,			//height
	3,			//3通道
	40,			//stable 40
	5,			//帧差5
	25,			//阈值25
	0,
	0.08,
	9,			//filter size
	0,			//不处理阴影
	1			//采用中值滤波
};

//==============================================================================
Dlbk::Dlbk()
{
	videoWidth		= 0;
	videoHeight		= 0;
	SRC_CHANNEL		= 0; 
	T_STABLE		= 0;
	FRAME_THRESHOLD = 0;
	BK_THRESHOLD	= 0;
	QUICK_UPDATE	= 0.0;
	ALPHA			= 0.0;
	FILTER_SIZE		= 0;
	IS_SHADOW		= 0;
	FT				= 0;

	pBK		= NULL;
	pSeg	= NULL;
	pDK		= NULL;
	pFK		= NULL;
	pLIN	= NULL;
	ptembk	= NULL;
	ptemin	= NULL;
	ptemhsv = NULL;
	pVin	= NULL;
	pVbk	= NULL;
}


//==============================================================================
//Initiate memory space
int Dlbk::Init(void *para)
{	
	struct DlbkPara *p = (struct DlbkPara *)(para);

	SRC_CHANNEL		= p->w; 
	T_STABLE		= p->x;
	FRAME_THRESHOLD = p->y;
	BK_THRESHOLD	= p->z;
	QUICK_UPDATE	= p->a;
	ALPHA			= p->b;
	FILTER_SIZE		= p->filter;
	IS_SHADOW		= p->isShadow;
	FT				= p->ft;
	videoWidth		= p->videoWidth;
	videoHeight		= p->videoHeight;

	pBK  = new unsigned char[videoWidth * videoHeight * SRC_CHANNEL];
	pSeg = new unsigned char[videoWidth * videoHeight];
	pDK  = new unsigned char[videoWidth * videoHeight];
	pFK  = new unsigned char[videoWidth * videoHeight];
	pLIN = new unsigned char[videoWidth * videoHeight * SRC_CHANNEL];

	memset(pBK, 0, videoWidth * videoHeight * SRC_CHANNEL);
	memset(pSeg, 0, videoWidth * videoHeight);
	memset(pDK, 0, videoWidth * videoHeight);
	memset(pFK, 0, videoWidth * videoHeight);
	memset(pLIN, 0, videoWidth * videoHeight * SRC_CHANNEL);

	//----------------------------------------
	//for shadow sub
	ptemin = cvCreateImage(cvSize(videoWidth, videoHeight), 8, 3 );	
	ptembk = cvCreateImage(cvSize(videoWidth, videoHeight), 8, 3 );	
	ptemhsv = cvCreateImage(cvSize(videoWidth, videoHeight), 8, 3 );
	pVin = cvCreateImage(cvSize(videoWidth, videoHeight), 8, 1 );
	pVbk = cvCreateImage(cvSize(videoWidth, videoHeight), 8, 1 );

	//----------------------------------------
	//for cal contour
	return 0;
}

//==============================================================================
//release memory
Dlbk::~Dlbk()
{
	if(pBK != NULL) delete [] pBK;
	if(pSeg != NULL) delete [] pSeg;
	if(pDK != NULL) delete [] pDK;
	if(pFK != NULL) delete [] pFK;
	if(pLIN != NULL) delete [] pLIN;
	if(ptembk != NULL) cvReleaseImage(&ptembk);
	if(ptemin != NULL) cvReleaseImage(&ptemin);
	if(ptemhsv != NULL) cvReleaseImage(&ptemhsv);
	if(pVin != NULL) cvReleaseImage(&pVin);
	if(pVbk != NULL) cvReleaseImage(&pVbk);
}

void Dlbk::OInit(int mWid, int mHei, unsigned char* pInput)
{	
    for(int i=0;i<mWid*mHei*SRC_CHANNEL;i++) 
	{
		pBK[i] = pInput[i];//initialize background		
		pLIN[i] = pInput[i];
	}
}

//==============================================================================
int Dlbk::UpdateBK(int mWid, int mHei, unsigned char* pInput)
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
void Dlbk::UpdateSeg(int mWid, int mHei, unsigned char* pInput)
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
void Dlbk::Process(unsigned char* pInput)
{
	static bool initflg = false;
	if (initflg==false)
	{
		OInit(videoWidth, videoHeight, pInput);
		initflg = true;
	}
	else
	{
		UpdateBK(videoWidth, videoHeight, pInput);
		UpdateSeg(videoWidth, videoHeight, pInput);           
		
		if (IS_SHADOW)
		{
			ShadowSub(pInput, pBK, pSeg, videoWidth, videoHeight);
		}
		
		Connect_Filter(videoWidth, videoHeight, pSeg);
	}
}

//==============================================================================
//Binary image median filtering with a (2ts+1)^2 mask
void Dlbk::Connect_Filter(int mWid, int mHei, unsigned char* pInput)
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
void Dlbk::ShadowSub(unsigned char* pIn, unsigned char* pBK, unsigned char* pSeg, int w, int h)
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
	SILTP_Filter(w, h, pSeg, (uchar*)(pVin->imageData), (uchar*)(pVbk->imageData));
}

//==============================================================================
void Dlbk::SILTP_Filter(int mWid, int mHei, unsigned char *pSeg, unsigned char * fg, unsigned char * bk)
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