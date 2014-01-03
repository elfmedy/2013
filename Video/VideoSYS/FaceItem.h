/************************************************************************\
Module:
	Basic data structure of face item
	
Abstract:

Notes:
	
History:
	Created by sticky, 11/05/2004
	
\************************************************************************/

#ifndef _FACE_ITEM_H_
#define _FACE_ITEM_H_

#include <vector>
#include "windows.h"

struct CIRCLE
{
	POINT center;
	int r;
};

// To store the temporary detection resuls before other post processing
struct SFaceItem	
{
	// Face area as a circle
	CIRCLE c;	
	// Face area as a rect
	RECT rect;

	// Tilt (In-plane rotation angle 0-359)
	int	nInPlaneAngle;
	
	// Yaw
	int nYawExtent;		
	
		
	// Confidence value
	double confidence;  

	//PCLUSTER_INFO pCluster; // A pointer to the cluster it belongs to.
	//SFaceItemMV(){ confidence = 0.0; }

}; 

// A list keeping track of all face items.
typedef std::vector<SFaceItem> SFaceItems;  

#endif