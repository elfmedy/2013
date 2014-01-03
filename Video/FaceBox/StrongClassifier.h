#ifndef __STRONG_CLASS_H__
#define __STRONG_CLASS_H__

#include "cxcore.h"
#include "highgui.h"

#include <vector>
#include <math.h>

#include "Common.h"
#include "Mapping.h"
#include "SparseFeature.h"
#include "CompactWeakClassifier.h"

using namespace std;

class SStrongClassifier
{
public:
	//
	// Variables
	//
	
	//dimension of strong classifier's output
	int m_iDimOutput;
	
	// sparse features
	vector<SSparseFeature> m_vSparseFeature;
	//weak hypothesis, mapping from feature value to output
	vector<SLutMapping> m_vWeakMapping;

	// below is some variables existing only in detection program
	//for compact weak classifiers
	vector<SCompactWeakClassifier> m_vCompactWCs;

	//connect all LUTs end to end so as to make sure the continuity of memory access
	float *m_pAllTables;						//rally point of all LUTs
	BOOL m_bAllTablesAllocate;							//it is true if m_pAllTables is created by this strong classifier, 
														//or it is false if m_pAllTables is shared from other strong classifer
	//
	// Methods
	//
	// Constructor
	SStrongClassifier();

	SStrongClassifier(int dim_output);

	// Destructor
	~SStrongClassifier();

	// Set output dimention
	void ResizeOutputDim(int dim_output);

	// Save & Load
	BOOL Load(const char* node_start, const char* node_end);
	
	void Save(ofstream &os);

	void CloneFrom(const SStrongClassifier &sc);

	void CalcOutput(CNewIntegralImage &iimg, float *output);
	void CalcOutputFast(CNewIntegralImage &iimg, float *output);
	
	// Prepare
	void ScaleImage(CNewIntegralImage &iimg);

	//rotate or mirror the sparse features in this strong classifier
	void GeometricTransfer(int op_type, int ref_w, int ref_h);

	//order pixels in sparse features by their memory address
	void OrderPixels();

	//functions below only exists in detection program
	void RallyLUTs();				//rally all LUTs
	void ConstructCompactWCs();		//construct compact weak classifier 
	void ScaleImageWithCompactWC(CNewIntegralImage &iimg);		//prepare offsets of pixels
	//calculating the output of strong classifier with compact weak classifiers
	void CalcOutputFastWithCompactWC(CNewIntegralImage &iimg, float *output);

	
};

#endif


