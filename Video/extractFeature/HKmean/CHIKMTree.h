// CHIKMTree.h: interface for the CHIKMTree class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CHIKMTREE_H__BBA4DD9D_D17E_41BA_AABD_E24A16D98FAD__INCLUDED_)
#define AFX_CHIKMTREE_H__BBA4DD9D_D17E_41BA_AABD_E24A16D98FAD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IKMTree.h"
#include "../atom.h" 



/** ------------------------------------------------------------------
/** @brief HIKM tree node 
 **
 **/

typedef struct _VlHIKMNode
{
  CIKMTree* filter ;              /**< IKM filter for this node*/
  struct _VlHIKMNode **children ; /**< Node children (if any) */
} VlHIKMNode ;

/** @brief HIKM tree */
typedef struct _VlHIKMTree {
  vl_uint32		M ;				/**< IKM: data dimensionality */
  vl_uint32		K ;				/**< IKM: K */
  int			max_niters ;	/**< IKM: maximum # of iterations */
  int			verb ;			/**< Verbosity level */
  vl_uint32		depth ;			/**< Depth of the tree */
  VlHIKMNode *	root;			/**< Tree root node */
} VlHIKMTree;

class CHIKMTree  
{
public:
	/************************************************************************/
	/*  construction and destruction                                        */
	/************************************************************************/
	CHIKMTree();
	virtual ~CHIKMTree();
	/************************************************************************/
	/*  retrieve and set parameters                                         */
	/************************************************************************/
	inline vl_uint32 vl_hikm_get_ndims      () { return m_VlHIKMTree->M;		  };
	inline vl_uint32 vl_hikm_get_K          () { return m_VlHIKMTree->K;		  };
	inline vl_uint32 vl_hikm_get_depth      () { return m_VlHIKMTree->depth;	  };
	inline void vl_hikm_set_max_niters(int max_niters){ m_VlHIKMTree->max_niters = max_niters; }
	/************************************************************************/
	/* Train the VlHIKMTree                                                 */
	/************************************************************************/
public:
	void vl_hikm_init_VlHIKMTree(vl_uint32 M, vl_uint32 K, vl_uint32 depth);
	vl_bool vl_hikm_train(const vl_ikm_data *data, vl_uint32 N);

	void vl_hikm_push (vl_uint32 *asgn, vl_ikm_data const *data, int N);
	vl_uint32* vl_hikm_assignment_normalization(vl_uint32* asgn, int N);
	/************************************************************************/
	/*  new and free element                                                */
	/************************************************************************/	
public:
	void vl_hikm_new_VlHIKMTree();
	void vl_hikm_free_VlHIKMTree();
/************************************************************************/
/* read and write                                                       */
/************************************************************************/
public:
	vl_bool vl_hikm_write_tree(const char* strSavePath);
	vl_bool	vl_hikm_read_tree(const char* strSavePath);
/************************************************************************/
/* data                                                                 */
/************************************************************************/	
private:
	VlHIKMTree	 *  m_VlHIKMTree;       // tree structure
};

#endif // !defined(AFX_CHIKMTREE_H__BBA4DD9D_D17E_41BA_AABD_E24A16D98FAD__INCLUDED_)
