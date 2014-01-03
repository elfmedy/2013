#ifndef __TREE_CLASS_H__
#define __TREE_CLASS_H__

#include "cxcore.h"
#include "highgui.h"

#include <vector>
#include <math.h>

#include "Common.h"
#include "Mapping.h"
#include "CompactWeakClassifier.h"
#include "SparseFeature.h"
#include "NodeClassifier.h"

using namespace std;

class STreeClassifier
{
public:
	//
	// Variables
	//
	// size of the reference window, in our system, this value is always(REF_W, REF_H)
	int m_iRefW, m_iRefH;	
	
	// Description
	char m_sDescription[256];

	// layers in which the coarse search strategy is adopted.
	int m_iCoarseLayerNum;

	// Root node of the tree
	SNodeClassifier* m_pRootNode;

	// For detecion:

	// Rotation angle (clockwise), can only be 0, 90, 180, 270.
	// And 90 is the upright classifier.
	//int m_iRotAng;	
	// For cloning. To be rotated by m_iToRotate degree clockwise.
	//int m_iToRotate;		

	//
	// Methods
	//

	// Constructor
	STreeClassifier();

	// Destructor
	~STreeClassifier();

	// release memory
	void Release();

	// Load tree classifier from file
	BOOL Load(const char *filename);

	// Save tree classifier to file
	BOOL Save(const char *filename);

	//clone 
	void CloneFrom(STreeClassifier &tree);

	// Get pointer to a node classifier according to the given path
	SNodeClassifier* GetNodeClassifier(vector<int> path);

	// Get pointer to a leaf node classifier according to the given path
	SNodeClassifier* GetLeafNodeClassifier(vector<int> path);

	// generate inner nodes that are symmetric to those existed ones (e.g. get right profile node from left profile node)
	BOOL GenerateMirroredNodes();	

	// Control tree path 
	void CloseAll();
	void OpenAll();

	BOOL OpenPath(vector<int> path, 
					int leaf_id);		//leaf_id must be larger than -1

	BOOL ExtractPathWithWildcard(vector<int> &path);
	virtual BOOL OpenView(int view_index, int direction, int level = -1);


	// Generate names of node (by recursion)
	void RefreshNameOfNode();

	void ScaleImage(CNewIntegralImage &iimg);			// Integral image step

	void GeometricTransfer(int op_type);

	void OrderPixels();

	void SetSpeedUpRatio(double speed_up_ratio);

};

#endif
