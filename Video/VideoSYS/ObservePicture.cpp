#include "ObservePicture.h"
#include "NormalTrack.h"

using namespace std;

// The boolean value NORM is used to decide whether to output traditional HSV values where 
// 0 <= S <= 100 and 0 <= V <= 255. 
// Else we keep the values at a norm where
// 0 <= S <= 1 and 0 <= V <= 1. 
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


//0-black  1-white  2-gray  3-pure color  4-light color  5-deep color
//seg h-s-v's s and v to 10 parts. eg. [0, 10) [10 20). the last one is 100 alone.
static const int table[11][11] = {
	{1, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3},
	{1, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3},
	{1, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3},
	{2, 2, 4, 4, 4, 3, 3, 3, 3, 3, 3},
	{2, 2, 4, 4, 4, 3, 3, 3, 3, 3, 3},
	{2, 2, 4, 4, 4, 3, 3, 3, 3, 3, 3},
	{2, 2, 5, 5, 5, 5, 5, 5, 5, 5, 5},
	{0, 0, 5, 5, 5, 5, 5, 5, 5, 5, 5},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};


//0-red 1-orange 2-yellow 3-kelly 4-green 5-sky blue 6-deep blue 7-purple  [0-360]
//37 element, the last one is num 360
static const int colorTable[37] = {	
	0, 0,			              				 
	1, 1,                        
	2, 2, 						   
	3, 3,                      
	4, 4, 4, 4, 4, 4, 4, 4,       
	5, 5, 5,                       
	6, 6, 6, 6, 6, 6, 6, 6,		   
	7, 7, 7,                       
	0, 0, 0, 0, 0, 0, 0			   
};


//a table used to locate array's element
static const int classifyTable[6] = {24, 25, 26, 0, 8, 16};	


void ObservePicture::Update(void *para)
{
	NTObjBox firstBox= ((NTTrackLine *)para)->line.front();
	int width  = firstBox.width;
	int height = firstBox.height;
	int step   = (width * 3 + 3) / 4 * 4;

	//classify image color, add colors in each bin
	int hsvClassify[8*3+1+1+1] = {0};	//8 colors * 3 level + black(24) + white(25) + grey(26) = 27 bins
	int sumPix = 0;
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			//pix rgb -> hsvClassify[x]'s count++
			double h, s, v;
			pixRGBtoHSVCommon( firstBox.rectImage[i*step+3*j+2], firstBox.rectImage[i*step+3*j+1], firstBox.rectImage[i*step+3*j+0], h, s, v, 1);
			int tmpColor = table[(int)(v/25.5)][(int)(s/10)];
			if (tmpColor < 3) hsvClassify[classifyTable[tmpColor]]++;
			else hsvClassify[colorTable[(int)(h/10)]+classifyTable[tmpColor]]++;
			sumPix++;
		}
	}

	//hsvClassify[27] -> feat[14]  ,histogram to vector
	unsigned char *feat = new unsigned char[14];
	memset(feat, 0, 14);
	int fraction20 = sumPix / 5;
	int fraction40 = sumPix * 2 / 5;
	int fraction60 = sumPix * 3 / 5;
	int fraction80 = sumPix * 4 / 5;
	for (int i = 0; i < 27; i++)
	{	
		if (hsvClassify[i] < fraction20)
		{
			continue;
		}
		else if (hsvClassify[i] < fraction40)
		{
			feat[i/2] |= 0x01 << 4 * (i % 2);
		}
		else if (hsvClassify[i] < fraction60)
		{
			feat[i/2] |= 0x02 << 4 * (i % 2);
		}
		else if (hsvClassify[i] < fraction80)
		{
			feat[i/2] |= 0x03 << 4 * (i % 2);
		}
		else
		{
			feat[i/2] |= 0x04 << 4 * (i % 2);
		}
	}

	//store feat
	database->SetPictureFeat(feat, 14);
	delete []feat;
}