#ifndef _SPARSE_FEATURE_
#define _SPARSE_FEATURE_

#include <fstream>
#include <vector>
#include <assert.h>
#include "NewIntegralImage.h"

#define MIRROR_HORIZONTAL	0
#define MIRROR_VERTICAL		1
#define ROTATE_90			2

using namespace std;

class SSinglePixelFeature
{
public:
	int m_iBandWidth;   //bandwidth of the pixel. the actual width of the pixel is 1<<bandwidth
	int m_iX, m_iY;     //offset of the pixel
	int m_iOffset;

	SSinglePixelFeature();
	SSinglePixelFeature(int x, int y, int bandwidth);

	int GetArea();
	void ScaleImage(CNewIntegralImage &iimg);
	unsigned char GetGrayValue(CNewIntegralImage &iimg);

	//I/O
	BOOL Load(const char* xml_begin, const char* xml_end); 
 	void Save(ofstream &os);

	//operator
	BOOL operator == (SSinglePixelFeature &f1);
	BOOL operator != (SSinglePixelFeature &f1);
	SSinglePixelFeature& operator = (SSinglePixelFeature f1);         
}; 

class SSparseFeature
{
public:
	vector<SSinglePixelFeature> m_vPixels;  //pixels
	vector<int> m_vPosOffset, m_vNegOffset;
	vector<int> m_vAlphas;  //coefficients
	int m_iSumArea;     //sum of covered area. use to balance so that EX = 0

	SSparseFeature();
	~SSparseFeature();

	void Clear();
	int IsIn(SSinglePixelFeature &single_pixel);		//return the index of single pixel, if not return -1
	BOOL Add(SSinglePixelFeature &single_pixel, int alpha);		//FALSE if existed, TRUE if not
	BOOL Remove(SSinglePixelFeature &single_pixel);		//TRUE if existed, FALSE if not
	void Decompose();	//decompose the pixel 

	BOOL operator == (SSparseFeature &f1);
	int GetPixelNum() {return m_vPixels.size();}
	// Calculate feature value
	void ScaleImage(CNewIntegralImage &iimg);
	double CalcFeatureValue(CNewIntegralImage &iimg);
	double CalcFeatureValueFast(CNewIntegralImage &iimg);

	// Load from file
	BOOL Load(const char* xml_begin, const char* xml_end); 
	// Save to file
 	void Save(ofstream &os);

	// Clone a feature
	void Clone(SSparseFeature &new_feature);
	// balance
	void Balance();
	// geometric transformation
	void GeometricTransfer(int op_type, int ref_w, int ref_h);

	void OrderPixels();
	void Mirror_Horizontal(int ref_w, int ref_h, SSinglePixelFeature ori_pixel, SSinglePixelFeature &cur_pixel);
	void Mirror_Vertical(int ref_w, int ref_h, SSinglePixelFeature ori_pixel, SSinglePixelFeature &cur_pixel);
	void Rotate_90(int ref_w, int ref_h, SSinglePixelFeature ori_pixel, SSinglePixelFeature &cur_pixel);

	SSparseFeature& operator = (const SSparseFeature &f1);         
	friend SSparseFeature operator + (SSparseFeature &f1, SSparseFeature &f2);
	friend SSparseFeature operator - (SSparseFeature &f1, SSparseFeature &f2);
	friend SSparseFeature operator * (SSparseFeature &f, int i);
	friend SSparseFeature operator / (SSparseFeature &f, int i);
};

#endif
