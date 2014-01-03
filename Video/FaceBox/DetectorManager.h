#ifndef _DETECTOR_MANAGER_H_
#define _DETECTOR_MANAGER_H_

#include "TreeClassifier.h"
#include "Common.h"
#include "Detector.h"

//////////////////////////////////////////////////////////////////////////
//			Used to load, clone and prepare tree detectors				//
//////////////////////////////////////////////////////////////////////////

class CDetectorManager
{
	//here is the individual-cascade based omni FD
public:
	STreeClassifier m_TreeF0;			//view 24
	STreeClassifier m_TreeF30;			//view 25
	STreeClassifier m_TreeHP0;			//view 12
	STreeClassifier m_TreeHP30;			//view 13
	STreeClassifier m_TreeHP_30;		//view 23
	STreeClassifier m_TreeFP0;			//view 0
	STreeClassifier m_TreeFP30;			//view 1
	STreeClassifier m_TreeFP_30;		//view 11

	STreeClassifier m_WFSTreeMVFD;		//wfs tree that covers 15 views

	vector<BOOL> m_vIsClone;			
	vector<STreeClassifier*> m_vAllTree;
	vector<STreeClassifier*> m_vDetectingTree;

public:
	CDetectorManager();
	~CDetectorManager();
	void Load(const char* path, char *load_log);
	void Release();
	void InitialTreeConfig(STreeConfig &tree_config);
	BOOL PrepareTrees(STreeConfig &tree_config);

	vector<STreeClassifier*>& GetDetectingTrees();

};





#endif
