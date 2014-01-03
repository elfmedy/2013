/************************************************************************\
Module
	Data structures of classifiers
	
Abstract

	
History
	Created by WU Bo, ?/?/?
	Modified by smooth, ?/?/?
	Modified by sticky, 8/18/2004
	Modified by sticky, 10/31/2004
	Modifier by smooth, 12/27/2004, for sparse feature
\************************************************************************/

#ifndef __MAPPING_H__
#define __MAPPING_H__

#include "cxcore.h"
#include "highgui.h"

#include "SparseFeature.h"
#include <vector>
#include <math.h>

using namespace std;

/*
	F is the original feature space, A is a Matrix
	AF = F'	
	F' is a new strong feature vector.

    Weak classifiers are based on F'.
*/

/************************************************************************\
				Mapping from feature value to confidence
\************************************************************************/
class SLutMapping
{
public:
	// Look up table
	float* m_pTable;
	// lower bound
	double m_dLowerBound;
	// scale ratio
	double m_dScaleRatio; 
	// Size of LUT
	int m_iLutSize;
	// is lut allocated
	BOOL m_bIsAllocated;
	// output dim
	int m_iDimOutput;

	//
	// Methods
	//
	SLutMapping();

	SLutMapping(int lut_size, int output_dim);
	SLutMapping(const SLutMapping &lut)
	{
		this->m_pTable = NULL;
		*this = lut;
	}

	~SLutMapping();



	// Get LUT value
	float* Mapping(double feature_value);

	// Set LUT number
	BOOL ResizeLutSize(int lut_size, int output_dim);
	
	// Save & Load
	BOOL Load(const char* map_start, const char* map_end);

	void Save(ofstream &os);

	void CloneFrom(const SLutMapping &lut);

	void GetLutElement(int lut_index, int dim_index, float &value) {value = m_pTable[lut_index*m_iDimOutput+dim_index]; }
	void SetLutElement(int lut_index, int dim_index, float value) {m_pTable[lut_index*m_iDimOutput+dim_index] = value; }
	SLutMapping& operator = (const SLutMapping &lut)
	{
		this->m_dLowerBound = lut.m_dLowerBound;
		this->m_dScaleRatio = lut.m_dScaleRatio;
		this->m_iLutSize = lut.m_iLutSize;
		this->m_iDimOutput = lut.m_iDimOutput;
		this->m_bIsAllocated = TRUE;
		ResizeLutSize(this->m_iLutSize, m_iDimOutput);
		memcpy(this->m_pTable, lut.m_pTable, sizeof(float)*m_iLutSize*m_iDimOutput);

		return *this;
	}

	
};

class SQuadraticMapping
{
public:
	//mapping parameters
	float m_dA, m_dB, m_dC;		//f(x) = a*x*x + b*x + c

	float Mapping(float feature_value)
	{
		return (m_dA*feature_value+m_dB)*feature_value+m_dC;
	}

	// Save & Load
	BOOL Load(const char* map_start, const char* map_end);

	void Save(ofstream &os);

	void Clone(SQuadraticMapping &q_m);
};

#endif
