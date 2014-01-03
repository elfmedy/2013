// imgProperty.cpp : 定义 DLL 应用程序的导出函数。
//

#include "imgProperty.h"

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
void pixRGBtoHSVCommon(const double R, const double G, const double B, double& H, double& S, double& V, const bool NORM)
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

void HsvColor(IplImage* img, int* feat)
{
	const int hTable[19] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 17};
	const int sTable[4]  = {0, 1, 2, 2};
	const int vTable[4]  = {0, 1, 2, 2};
	for (int y = 0; y < img->height; y++)
	{
		for ( int x = 0; x < img->width; x++)
		{
			double h, s, v;
			pixRGBtoHSVCommon( (unsigned char)(img->imageData + img->widthStep*y)[x*3+2],
				(unsigned char)(img->imageData + img->widthStep*y)[x*3+1],
				(unsigned char)(img->imageData + img->widthStep*y)[x*3+0], h, s, v, 1);
			feat[hTable[(int)(h/20.0)]*3*3 + sTable[int(s/33.3)]*3 + vTable[(int)(v/85.0)]]++;
		}
	}
}

int * _stdcall GetImgProperty(const char *directory)
{
	IplImage *img = cvLoadImage(directory);
	int *feat = new int[18*3*3*2];
	memset(feat, 0, 18*3*3*2);
	HsvColor(img, feat);
	return feat;
}


void _stdcall ReleaseArray(int *arr)
{
	delete [] arr;
}


