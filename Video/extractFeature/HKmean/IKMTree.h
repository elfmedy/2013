// IKMTree.h: interface for the CIKMTree class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IKMTREE_H__C36F54FF_24AD_4AE9_B416_511BB50A5849__INCLUDED_)
#define AFX_IKMTREE_H__C36F54FF_24AD_4AE9_B416_511BB50A5849__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "../atom.h" 


#ifdef KD_TREE
typedef struct _paramKD_KNN
{
	struct feature* feat;
	struct kd_node* kd_root;
	struct feature* centers_kd;
	vl_uint32		K;
	double		  * dist;
	int			  *	index_knn;
}paramKD_KNN;
#endif
/** ------------------------------------------------------------------
 ** @brief IKM quantizer
 **/
typedef vl_uint32 vl_ikm_acc;	// type of the distance between point

typedef vl_uint8 vl_ikm_data;	// type of each dimension of one point 

#define CLUSTER_READ_CONTROL  "%d"
#define CLUSTER_WRITE_CONTROL  "%3d "

//typedef double vl_ikm_acc;	// type of the distance between point
//
//typedef float vl_ikm_data;	// type of each dimension of one point 
//
//#define CLUSTER_READ_CONTROL  "%f"
//#define CLUSTER_WRITE_CONTROL  "%2.4f "
//#define ACC_WRITE_CONTROL  "%15.2lf"

typedef struct _VlIKMFilt
{
  vl_uint32		M ;                 /**< data dimensionality */
  vl_uint32		K ;                 /**< number of centers   */
  int			max_niters ;		/**< Lloyd: maximum number of iterations */
  int			verb ;				/**< verbosity level */
  vl_ikm_data *	centers ;			/**< centers */
} VlIKMFilt ;

/* pairs are used to generate random permutations of data */
typedef struct _pair_t
{
	vl_int32 w;
	vl_int32 j;
} pair_t;

typedef struct _VlMatch
{
	int n1;
	int n2;
}VlMatch;

typedef struct _VlMatchList
{
	int ptID;
	_VlMatchList * next;
}VlMatchList;

class CIKMTree  
{
	/************************************************************************/
	/*  construction and destruction                                        */
	/************************************************************************/
public:
	CIKMTree();
	virtual ~CIKMTree();
	/************************************************************************/
	/*  retrieve and set parameters                                         */
	/************************************************************************/	
public:
	
	inline vl_uint32         vl_ikm_get_ndims()		{   return m_pVlIKMFilt->M ;          }
	inline vl_uint32         vl_ikm_get_k()			{	return m_pVlIKMFilt->K ;		  }
	inline int               vl_ikm_get_max_niters(){	return m_pVlIKMFilt->max_niters ; }
	inline int               vl_ikm_get_verbosity()	{	return m_pVlIKMFilt->verb;        }
	inline vl_ikm_data const* vl_ikm_get_centers()	{	return m_pVlIKMFilt->centers;	  }
	inline void vl_ikm_set_verbosity(int verb)       { m_pVlIKMFilt->verb       = verb;      }
	inline void vl_ikm_set_max_niters(int max_niters){ m_pVlIKMFilt->max_niters = max_niters;}
	inline void vl_ikm_set_M(vl_uint32 M){ m_pVlIKMFilt->M = M;}
	inline void vl_ikm_set_K(vl_uint32 K){ m_pVlIKMFilt->K = K;}
	inline void vl_ikm_set_centers(vl_ikm_data* centers){ m_pVlIKMFilt->centers = centers;}
	/************************************************************************/
	/* Train the VlHIKMTree                                                 */
	/************************************************************************/
public:
	void vl_ikm_init_rand(vl_uint32 M, vl_uint32 K);
	void vl_ikm_init_rand_data(vl_ikm_data const* data, vl_uint32 M, vl_uint32 N, vl_uint32 K);
	//vl_bool CIKMTree::vl_ikm_train(const vl_ikm_data *data, vl_uint32 N, FILE* file, VlMatchList** listMatch);
	vl_bool vl_ikm_train(vl_ikm_data const *data, vl_uint32 N, FILE* file);
private:
	vl_ikm_acc vl_ikm_computer_distance(const vl_ikm_data* data1, const vl_ikm_data* data2);
	/************************************************************************/
	/* Project the date				                                        */
	/************************************************************************/
public:
	vl_uint32  vl_ikm_push_one(vl_ikm_data const *data);  
	vl_uint32* vl_ikm_push_one_soft( vl_ikm_data* data, int N );
	void       vl_ikm_push(vl_uint32 *asgn, vl_ikm_data const *data, vl_uint32 N);
#ifdef KD_TREE
private:
	void	   vl_ikm_push_kd(vl_uint32 *asgn, const vl_ikm_data *data, vl_uint32 N, 
							struct kd_node* kd_root, struct feature* centers_kd);
#endif
public:
	vl_uint32* vl_ikm_assign_to_hist(vl_uint32* assign, vl_uint32 n);
	/************************************************************************/
	/*  new and free element                                                */
	/************************************************************************/
public:
	void vl_ikm_new_tree();
	void vl_ikm_delete_tree();
	/************************************************************************/
	/* Read and Write                                                       */
	/************************************************************************/
public:
	vl_bool vl_ikm_read_tree(const char* cluster_file_name);
	vl_bool	vl_ikm_write_tree(const char* cluster_file_name);
private:
	VlIKMFilt* m_pVlIKMFilt;
};

#endif // !defined(AFX_IKMTREE_H__C36F54FF_24AD_4AE9_B416_511BB50A5849__INCLUDED_)
