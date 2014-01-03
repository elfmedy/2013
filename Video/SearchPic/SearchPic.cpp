#include "SearchPic.h"
#include "stdio.h"
#include <fstream>

using namespace std;

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


//pictureFeat struct
struct PictureFeatStruct
{
	int count;
	unsigned char originalArr[14];
	int idArr[100];
};



static void FeatToData(unsigned char *feat, const char *firstDataFile, int *idArr, int &hashValue, int &idCount)
{
	//cal the hash value to feat array
	unsigned int seed = 131;	//31 131 1313 131313 etc..  (bkdr hash seeds)
	unsigned int tmpHashValue = 0;
	for (int i = 0; i < 14; i++)
	{
		tmpHashValue = tmpHashValue * seed + feat[i];
	}
	tmpHashValue = (tmpHashValue & 0x7FFFFFFF) % 100003;	

	fstream fileFirst(firstDataFile, ios_base::in | ios_base::binary);
	struct PictureFeatStruct tmp;
	fileFirst.seekg(tmpHashValue * sizeof(struct PictureFeatStruct));
	fileFirst.read((char *)(&tmp),sizeof(struct PictureFeatStruct));

	memcpy(idArr, tmp.idArr, 100);
	hashValue = tmpHashValue;
	idCount   = tmp.count;

	fileFirst.close();
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


void _stdcall SearchPic(const char *picFile, const char *firstDataFile, int *idArr, int &hashValue, int &idCount)
{
	IplImage *img = cvLoadImage(picFile);

	unsigned char *rectImage = (unsigned char *)(img->imageData);
	int width  = img->width;
	int height = img->height;
	int step   = img->widthStep;

	//classify image color, add colors in each bin
	int hsvClassify[8*3+1+1+1] = {0};	//8 colors * 3 level + black(24) + white(25) + grey(26) = 27 bins
	int sumPix = 0;
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			double h, s, v;
			pixRGBtoHSVCommon( rectImage[i*step+3*j+2], rectImage[i*step+3*j+1], rectImage[i*step+3*j+0], h, s, v, 1);
			int tmpColor = table[(int)(v/25.5)][(int)(s/10)];
			if (tmpColor < 3) hsvClassify[classifyTable[tmpColor]]++;
			else hsvClassify[colorTable[(int)(h/10)]+classifyTable[tmpColor]]++;
			sumPix++;
		}
	}

	//hsvClassify[27] -> feat[14]
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

	//read picture feat file
	FeatToData(feat, firstDataFile, idArr, hashValue, idCount);
	delete [] feat;
}


void _stdcall SearchVal(const char *dataFilePath, int fileSerialNum, int hashValue, int *idArr)
{
	char tmpStr[256];
	sprintf(tmpStr, "%s_03d", dataFilePath, fileSerialNum);

	fstream fin(tmpStr, ios_base::in | ios_base::binary);
	struct PictureFeatStruct tmp;
	fin.seekg(hashValue * sizeof(struct PictureFeatStruct));
	fin.read((char *)(&tmp),sizeof(struct PictureFeatStruct));
	memcpy(idArr, tmp.idArr, 100);

	fin.close();
}