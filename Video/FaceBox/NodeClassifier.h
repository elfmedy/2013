#ifndef __NODE_CLASS_H__
#define __NODE_CLASS_H__

#include "cxcore.h"
#include "highgui.h"

#include <vector>
#include <math.h>

#include "CompactWeakClassifier.h"
#include "StrongClassifier.h"
#include "Common.h"
#include "Mapping.h"
#include "SparseFeature.h"

using namespace std;

#define X2EXP_LOWER_BOUND -8
#define X2EXP_UPPER_BOUND 2
#define X2EXP_INTERPOLATENUM 1001
#define X2EXP_SCALE_RATIO 100

enum NodeType{
		NON_NODE = -1,
		SINGLE_CLASS_NODE = 0,
		MULTI_CLASS_NODE,
		BRANCHING_NODE
};

typedef vector<float> VEC_LUT;

union UNodeThreshold{
	double multi_class_threshold;
	double single_class_threshold;
	double branching_thresholds[MAX_NUM_LUT];
};

/************************************************************************\
					      Node Classifier	
\************************************************************************/
class SNodeClassifier
{
public:
	//
	// Variables
	//
	// Name of node
	char m_sNodeName[128];	
	
	// node type
	NodeType m_NodeType;
	
	// output dim
	int m_iDimOutput;
	
	// nested weak classifier
	vector<SQuadraticMapping> m_vNestedWeak;

	// strong classifier
	SStrongClassifier m_StrongClassifier;
	
	UNodeThreshold m_Threshold, m_Threshold1;
	UNodeThreshold m_OriThreshold, m_OriThreshold1;
	
	// Layer index of this node in the tree
	int m_iLayerIndex;
	
	// parent node
	SNodeClassifier *m_pParent;
	int m_iIndexSon;
	
	// Son nodes
	SNodeClassifier *m_pSons[MAX_NUM_LUT];
	
	// ID of son nodes
	int m_pID[MAX_NUM_LUT];
	// Accessibility of son nodes 
	BOOL m_bAvail[MAX_NUM_LUT];
	// If the son node is a leaf node
	BOOL m_bIsLeaf[MAX_NUM_LUT];
	
	
	//running time data
	BOOL m_pPassed[MAX_NUM_LUT];
	float m_pOutput[MAX_NUM_LUT];

	static double x2exp[X2EXP_INTERPOLATENUM];
	
	//
	// Methods
	//
	// Constructor
	SNodeClassifier();

	// Destructor
	~SNodeClassifier();

	static void Init();

	// Load node classifier from file
	BOOL Load(const char* node_name, int layer_index, const char* xml_begin, const char* xml_end);

	// Save node classifier to file
	void Save(ofstream &os);

	// clone a node classifier
	void SNodeClassifier::CloneFrom(SNodeClassifier& node);
		
	double CalExpByInterpolation(double x);
	
	// Control tree path 
	void CloseAll();
	void OpenAll();

	// Generate a vector containing all layer vectors (by recursion)
	//void RefreshAllLayerVector(vector<SLayerClassifierExt*> &allLayer);

	// Generate names of node (by recursion)
	void RefreshNameOfNode();

	void GeometricTransfer(int op_type, int ref_w, int ref_h);

	void OrderPixels();

	void SetSpeedUpRatio(double speed_up_ratio);
	void SwapThreshold();
		
	//prepare
	void ScaleImage(CNewIntegralImage &iimg);

	//run
	void RunFast(CNewIntegralImage &iimg);


};

#endif
