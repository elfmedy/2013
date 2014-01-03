#ifndef _COMPACT_WEAK_CLASSIFIER
#define _COMPACT_WEAK_CLASSIFIER

#include "Common.h"
#include "Mapping.h"
#include "SparseFeature.h"
#define MAX_PIXELS_FOR_SPARSE_FEATURE 8

//////////////////////////////////////////////////////////////////////////
//				Compact Weak Classifier structure						//
//////////////////////////////////////////////////////////////////////////

class SCompactWeakClassifier
{
public:
	//for sparse feature
	int m_iPosPixelNum;		//number of positive pixels
	int m_iNegPixelNum;		//number of negative pixels
	int m_pOffsets[MAX_PIXELS_FOR_SPARSE_FEATURE];	//theirs offsets
	
	//for lut mapping
	double m_dLowerBound;
	double m_dScaleRatio;
	int m_iLutSize;
	int m_iDimOutput;
	float *m_pTable;
	
	void Construct(SSparseFeature &s_f, SLutMapping &lut_mapping)
	{
		m_iPosPixelNum = m_iNegPixelNum = 0;
		for(int i=0; i<s_f.m_vPixels.size(); i++){
			if(s_f.m_vAlphas[i]>0)
				m_iPosPixelNum++;
			else
				m_iNegPixelNum++;
		}
		
		m_dLowerBound = lut_mapping.m_dLowerBound;
		m_dScaleRatio = lut_mapping.m_dScaleRatio;
		m_iLutSize = lut_mapping.m_iLutSize;
		m_iDimOutput = lut_mapping.m_iDimOutput;
		m_pTable = lut_mapping.m_pTable;
	};		
};


#endif
