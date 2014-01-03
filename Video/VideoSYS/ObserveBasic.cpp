#include "ObserveBasic.h"
#include "NormalTrack.h"

using namespace std;

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
static void pixRGBtoHSVCommon(const double R, const double G, const double B, double& H, double& S, double& V, const bool NORM)
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


//seg h-s-v s and v to 10. eg [0, 10). the end is 100 single.
//num is color binary value. -MSB color grey white black LSB-
//eg. black is 0x01, white is 0x02, grey is 0x04, color is 0x08,
//black or grey is 0x01 | 0x04 = 0x05 means this num can be black or grey
static const unsigned int table[11][11] =	{			//color | grey | white | black
	{2, 10,  8, 8, 8, 8, 8, 8, 8, 8, 8},
	{2, 10,  8, 8, 8, 8, 8, 8, 8, 8, 8},
	{2, 10,  8, 8, 8, 8, 8, 8, 8, 8, 8},
	{6, 12,  8, 8, 8, 8, 8, 8, 8, 8, 8},
	{6, 12,  8, 8, 8, 8, 8, 8, 8, 8, 8},
	{4,  4, 12, 8, 8, 8, 8, 8, 8, 8, 8},
	{5,  5, 12, 8, 8, 8, 8, 8, 8, 8, 8},
	{1,  1, 12, 8, 8, 8, 8, 8, 8, 8, 8},
	{1,  1,  9, 8, 8, 8, 8, 8, 8, 8, 8},
	{1,  1,  1, 9, 9, 9, 9, 9, 9, 9, 9},
	{1,  1,  1, 1, 1, 1, 1, 1, 1, 1, 1}
};


//37 element, last is 360 single
//same as above, num is the color binary value
//eg. red=0x01, blue=0x02, green=0x04, yellow=0x08
//	  red or blue=0x01 | 0x02 = 0x03 means this num can be red or blue
static const unsigned int colorTable[37] = {			// red | yellow | green | blue | red
	1, 3,	
	3, 2, 2, 2, 6,	
	4, 4, 4, 4, 4, 4, 4, 4, 12, 12,	
	12, 12, 8, 8, 8, 8, 8, 8, 8, 8, 8, 9,
	9, 1, 1, 1, 1, 1, 1, 1	
};


void ObserveBasic::Update(void *para)
{
	NTTrackLine *p = (NTTrackLine *)(para);

	//time period
	int startFC = p->line.front().frameCount;
	int endFC = p->line.back().frameCount;
	database->SetTime(p->startMs, videoFps, startFC, endFC);

	//overallimage. draw box rectangle and track line(left-bottom point) 
	IplImage *tmpImage = cvCreateImageHeader(cvSize(videoWidth, videoHeight), IPL_DEPTH_8U, 3);
	cvSetImageData(tmpImage, p->firstFrame, videoWidth * 3);
	cvRectangle(tmpImage, cvPoint(p->line.front().x, p->line.front().y), cvPoint(p->line.front().x + p->line.front().width, p->line.front().y + p->line.front().height), cvScalar(255,0,0), 1);
	for (vector<NTObjBox>::iterator iterBox = p->line.begin() + 1; iterBox != p->line.end(); iterBox++)
	{
		cvLine(tmpImage, cvPoint((iterBox-1)->x, (iterBox-1)->y + (iterBox-1)->height), cvPoint(iterBox->x, iterBox->y + iterBox->height), cvScalar(0, 0, 255));
	}
	database->SetOverallImage((unsigned char *)(tmpImage->imageData), videoWidth, videoHeight);
	cvReleaseImageHeader(&tmpImage);

	//basic color. choose 5 images in track, average their basic color
	unsigned int color = 0x00;

	int sample[5] = {0, 0, 0, 0, 0};
	if ((int)(p->line.size()) > 5)
		for (int i = 0; i < 5; i++)
			sample[i] = (int)(p->line.size()) / 5 * i;

	int hsvClassify[4+1+1+1] = {0};		//red | yellow | green | blue | black | white | grey = 7 bins, the value of each bin is color count
	int sumPixs = 0;

	for (int i = 0; i < 5; i++)
	{
		int width  = (p->line)[sample[i]].width;
		int height = (p->line)[sample[i]].height;
		int step0  = (width + 3) / 4 * 4;
		int step   = (width * 3 + 3) / 4 * 4;
		for (int j = 0; j < height; j++)
		{
			for (int k = 0; k < width; k++)
			{
				if ((p->line)[sample[i]].maskImage[j*step0+k] != 0)
				{
					double h, s, v;
					pixRGBtoHSVCommon( (p->line)[sample[i]].rectImage[j*step+3*k+2], (p->line)[sample[i]].rectImage[j*step+3*k+1], (p->line)[sample[i]].rectImage[j*step+3*k+0], h, s, v, 0);
					int tmpColor = table[10-(int)(v * 10)][(int)(s * 10)];
					if (tmpColor & 0x01) hsvClassify[4]++;
					if (tmpColor & 0x02) hsvClassify[5]++;
					if (tmpColor & 0x04) hsvClassify[6]++;
					if (tmpColor & 0x08)
					{
						int tmpColor2 = colorTable[(int)(h/10)];
						if (tmpColor2 & 0x01) hsvClassify[0]++;
						if (tmpColor2 & 0x02) hsvClassify[1]++;
						if (tmpColor2 & 0x04) hsvClassify[2]++;
						if (tmpColor2 & 0x08) hsvClassify[3]++;
					}
					sumPixs++;
				}
			}
		}
	}
	sumPixs = (int)(sumPixs * 0.3);	//more than 30% regard as a basic color
	for (int u = 0; u < 7; u++)
	{
		if (hsvClassify[u] > sumPixs) 
		{
			color |=  (0x01 << u);	//binary: MSB grey white black blue green yellow red LSB
		}
	}
	database->SetBasicColor(color);
}
