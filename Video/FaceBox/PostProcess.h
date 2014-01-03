
/*************************************************************************\
Module
	Face item merger
	
Abstract

	(Original version: FaceCluster.h)
	8/18/2004
	Usage:
	I have replaced the structs SFaceItemMV and FaceItemsMV. The interface
	of basic functions have also been changed, but not significantly.
	To use the merging routine, define a CFaceCluster varible.

	1) To clear the face item list, call Initialize(...);
	2) To add a face item into the list, call AddFace(...);
	3) To merge the face items, call Merge();
	4) For user's convenience, two members of CFaceCluster class are made 
	public: 
	m_rgFaceItem, the original face item list;
	m_rgResultFaceItem, the merge face item list.
	Both are of the type FaceItemsMV. To decide which to use, call IsMerged()
	to see whether the result face list has been generated.
	  
	5) Other utilities:
	SetMaxNumOfFaces(...) allows you to set the max number of face items to be
	stored.
	Other params concerning the strategy of face merging can be accessed 
	thought a series of set/get functions.


	Notes:
	It has been tested that the new merging routine will not affect the 
	detection speed.

	9/12/2004
	CalculateDistance() modified.



History
	Created by sticky, 11/20/2004
	Last modified by sticky, 11/26/2004

\*************************************************************************/

#ifndef __POSTPROC_H__
#define __POSTPROC_H__

#include <windows.h>
#include "Common.h"
#include "FaceItem.h"
#include <math.h>
#include <vector>

struct SClusterInfo
{
	double cx, cy, r;
	double InPlaneAngle;	//in plane rotation angle (0-359)
	double YawExtent;		//out of plane rotation extent
	int nYawExtent;			//used for clustering
	double confidence;		//confidence value

	int nNumOfFaces;

	//FACE_VECTOR meanVector;

	bool eliminated;

	int id; 

};

//typedef std::list<FACE_ITEM> FACE_ITEM_LIST;  // A list keeping track of all face items.
//typedef FACE_ITEM_LIST FaceItemsMV;

typedef std::vector<SClusterInfo*> SFaceClusterList; // A list keeping track of the mean vector of each cluster.
typedef std::vector<int> SClusterIndex;

class SMergeConfig
{
public:
	SMergeConfig(
					IN double position = 1,
					IN double size = 1,
					IN double angle = 1,
					IN double disThreshold = 0.8,

					IN double overlapThreshold = 0.6,
					IN int minNumThreshold = 2
					)
	{
		m_overlapThreshold = overlapThreshold;
		m_minNumThreshold = minNumThreshold;
		m_position = position;
		m_size = size;
		m_angle = angle;

		m_disThreshold = disThreshold;

	}
	double m_position, m_size, m_angle;
	double m_disThreshold;	// A vector with larger distance to other clusters' mean vectors
	                        // is recognized as a new cluster.
	double m_overlapThreshold;
	int m_minNumThreshold;	// A cluster with less number of face items will be eliminated.

};
 

class CMerger
{
	//
	// Data
	//
private:	
	SFaceItems* m_rgFaceItem;				// Original face items
	
	SFaceClusterList m_rgInCluster;			// Which cluster does the face item belong to
	SClusterIndex* m_rgInClusterIdx;

	SFaceItems* m_rgResultFaceItem;
	SFaceClusterList m_rgCluster;

private:

//	long m_lMaxNumOfFaces;
//	bool m_fMerged;
//	bool m_fClustered;

	//
	// Param
	//
	int m_nImageW, m_nImageH;

	//
	// Weight & others
	//
	SMergeConfig m_Config;

	double m_squredDisThre; 


	double CalculateDistance(
					IN SClusterInfo& u, 
					IN SFaceItem& v
					);

	bool Overlapped(
					IN SClusterInfo* c1, 
					IN SClusterInfo* c2
					);


public:
	CMerger();
	
	~CMerger();

	// Initialize:
	void Initialize(
					IN int w,
					IN int h
					);
	
	// Merging:
	void Merge(
	  					IN SFaceItems* inputFaces,
						OUT SFaceItems* outputFaces,
						IN SMergeConfig& config,
						OUT SClusterIndex* clusters = NULL
						);

private:
	// For Clustering:
	void AddOne2Cluster(IN int faceIndex);
	void AddAll2Cluster();	

	// Eliminating (overlapped) faces:
	void EliminateOverlapped();

	// Genterate the merged face list:
	void GenResultList();

	void DeleteClusters();

};



#endif