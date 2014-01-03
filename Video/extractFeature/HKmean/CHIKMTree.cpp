// CHIKMTree.cpp: implementation of the CHIKMTree class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CHIKMTree.h"
#include <assert.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include "string.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

#define MAX_PATH 100

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CHIKMTree::CHIKMTree()
{
	m_VlHIKMTree	= NULL;
}

CHIKMTree::~CHIKMTree()
{
	vl_hikm_free_VlHIKMTree();
}

/************************************************************************/
/*  free the elements                                                   */
/************************************************************************/
static void xdelete (VlHIKMNode *node)    
{
	if(node) {
		if (node->children) {
			for(vl_uint32 k = 0 ; k < node->filter->vl_ikm_get_k() ; k++)
				xdelete (node->children[k]) ;
			free (node->children) ;
		}
		if (node->filter)
			node->filter->vl_ikm_delete_tree();
			//delete(node->filter);
		free(node);
	}	
}

void CHIKMTree::vl_hikm_free_VlHIKMTree()
{
	if (m_VlHIKMTree) {
		if (m_VlHIKMTree->root)  xdelete (m_VlHIKMTree -> root) ;
		free (m_VlHIKMTree) ;
		m_VlHIKMTree = NULL;
	}	
}

/************************************************************************/
/*  new vlHIKMTree and initialization                                   */
/************************************************************************/
void CHIKMTree::vl_hikm_new_VlHIKMTree()
{
	if (m_VlHIKMTree) {
		vl_hikm_free_VlHIKMTree();
	}
	m_VlHIKMTree = (VlHIKMTree*)malloc (sizeof(VlHIKMTree)) ;
	m_VlHIKMTree -> M          = 0 ;
	m_VlHIKMTree -> K          = 0 ;
	m_VlHIKMTree -> depth      = 0 ;
	m_VlHIKMTree -> max_niters = 200 ;
	m_VlHIKMTree -> verb       = 0 ;
	m_VlHIKMTree -> root       = NULL ;
}

void CHIKMTree::vl_hikm_init_VlHIKMTree(vl_uint32 M, vl_uint32 K, vl_uint32 depth)
{
	if (!m_VlHIKMTree) {
		vl_hikm_new_VlHIKMTree();
	}
	
	xdelete (m_VlHIKMTree -> root) ;
	m_VlHIKMTree -> root = NULL;
	
	m_VlHIKMTree -> M = M ;
	m_VlHIKMTree -> K = K ;
	m_VlHIKMTree -> depth = depth ;
}
/************************************************************************/
/* train the vlHIKMTree                                                 */
/************************************************************************/
/* ------------------------------------------------------------------
 brief Copy a subset of the data to a buffer
 
 param data  Data
 param ids   Data labels
 param N     Number of indices
 param M     Data dimensionality 
 param id    Label of data to copy
 param N2    Number of data copied (out)
 
 return a new buffer with a copy of the selected data.
 */

static vl_ikm_data* vl_hikm_copy_subset (vl_ikm_data const * data, 
							   vl_uint32 * ids, 
                               vl_uint32 N, vl_uint32 M, 
                               vl_uint32 id, vl_uint32 *N2)
{
  vl_uint32 i ;
  vl_uint32 count = 0;

  /* count how many data points with this label there are */
  for (i = 0 ; i < N ; i++)
    if (ids[i] == id)
      count ++ ;
  *N2 = count ;
  
  /* copy each datum to the buffer */
  {
    vl_ikm_data * new_data = (vl_ikm_data*)malloc (sizeof(vl_ikm_data) * M * count);    
    count = 0;
    for (i = 0 ; i < N ; i ++)
      if (ids[i] == id)
        {
          memcpy(new_data + count * M, 
                 data     + i     * M, 
                 sizeof(vl_ikm_data) * M);
          count ++ ;
        }    
    *N2 = count ;
    return new_data ;
  }  
}

/** ------------------------------------------------------------------
 ** @brief Compute HIKM clustering.
 **
 ** @param tree   HIKM tree to initialize.
 ** @param data   Data to cluster.
 ** @param N      Number of data points.
 ** @param K      Number of clusters for this node.
 ** @param height Tree height.
 **
 ** @remark height cannot be smaller than 1.
 **
 ** @return a new HIKM node representing a sub-clustering.
 **/
//
//static VlHIKMNode * xmeans (VlHIKMTree *tree, const vl_ikm_data *data, vl_uint32 N, vl_uint32 K, vl_uint32 height, FILE* file, VlMatchList** listMatch)
//{
//  VlHIKMNode *node = (VlHIKMNode*)malloc (sizeof(VlHIKMNode)) ;
//  vl_uint32  *ids  = (vl_uint32*) malloc (sizeof(vl_uint32) * N);
//
//  node-> filter   =  new CIKMTree(); 
//  node-> children = (height == 1) ? NULL : (VlHIKMNode**)malloc(sizeof(VlHIKMNode*) * K) ;
//
//  node->filter->vl_ikm_new_tree();
//  node->filter->vl_ikm_set_max_niters(tree->max_niters);
//  node->filter->vl_ikm_set_verbosity (1) ;
//  node->filter->vl_ikm_init_rand_data(data, tree->M, N, K);
//  node->filter->vl_ikm_train(data, N, file, listMatch) ;
//  node->filter->vl_ikm_push (ids, data, N) ;
//  
//  /* recurse for each child */
//  if (height > 1) {
//
//	  for (vl_uint32 k = 0 ; k < K ; k ++) {
//		  vl_uint32		partition_N ;
//		  vl_uint32		partition_K ;
//		  vl_ikm_data *	partition ;
//
//		  partition = vl_hikm_copy_subset(data, ids, N, tree->M, k, &partition_N) ;
//
//		  int* indPointID_sub = (int*)malloc( sizeof(int) * partition_N );
//		  int flag = 0;
//		  for ( vl_uint32 i = 0; i < N; i++ )
//		  {
//			  if (ids[i] == k){
//				  indPointID_sub[flag] = i;
//				  flag ++;
//			  }
//		  }
//
//		  VlMatchList** listMatchSub = (VlMatchList**)malloc( sizeof(VlMatchList*) * partition_N );
//		  for ( vl_uint32 i = 0 ; i < partition_N; i++ )
//		  {
//				listMatchSub[i] = listMatch[ indPointID_sub[i] ];
//
//				if ( ! listMatchSub[i] )
//					continue;
//
//				VlMatchList * item = listMatchSub[i];
//				VlMatchList ** item_father = &(listMatchSub[i]);
//				while( item ){
//					vl_bool exist = 0;
//					for ( vl_uint32 j = 0; j < partition_N; j++ )
//					{
//						if ( indPointID_sub[j] == item->ptID )
//						{
//							item->ptID = j;
//							exist = 1;
//							break;
//						}
//					}
//					
//					if ( ! exist )
//					{
//						*item_father = item->next;
//						item = item->next;
//					}else{
//						item_father = &(item->next);
//						item = item->next;
//					}
//				}
//		  }
//
//
//		  partition_K = VL_MIN (K, partition_N);
//
//		  node->children [k] = xmeans(tree, partition, partition_N, partition_K, height - 1, file, listMatchSub) ;
//		  free (partition) ;
//		  free (indPointID_sub);
//		  free (listMatchSub);
//
//		  /*  print some information */
//		  if (tree->verb > (int)tree->depth - (int)height && file) {
//			  fprintf(file, "hikmeans: branch at depth %d: %6.1f %% completed\n", 
//				  tree->depth - height, (double) (k+1) / K * 100) ;
//			  fflush(file);
//		  }
//	  }
//  }
//  free (ids) ;
//  return node ;
//}

static VlHIKMNode * xmeans (VlHIKMTree *tree, const vl_ikm_data *data, vl_uint32 N, vl_uint32 K, vl_uint32 height, FILE* file)
{
	VlHIKMNode *node = (VlHIKMNode*)malloc (sizeof(VlHIKMNode)) ;
	vl_uint32  *ids  = (vl_uint32*) malloc (sizeof(vl_uint32) * N);

	node-> filter   =  new CIKMTree(); 
	node-> children = (height == 1) ? NULL : (VlHIKMNode**)malloc(sizeof(VlHIKMNode*) * K) ;

	node->filter->vl_ikm_new_tree();
	node->filter->vl_ikm_set_max_niters(tree->max_niters);
	node->filter->vl_ikm_set_verbosity (1) ;
	node->filter->vl_ikm_init_rand_data(data, tree->M, N, K);
	node->filter->vl_ikm_train(data, N, file) ;
	node->filter->vl_ikm_push (ids, data, N) ;

	/* recurse for each child */
	if (height > 1) {

		for (vl_uint32 k = 0 ; k < K ; k ++) {
			vl_uint32		partition_N ;
			vl_uint32		partition_K ;
			vl_ikm_data *	partition ;

			partition = vl_hikm_copy_subset(data, ids, N, tree->M, k, &partition_N) ;

			partition_K = VL_MIN (K, partition_N);

			node->children [k] = xmeans(tree, partition, partition_N, partition_K, height - 1, file) ;

			free (partition) ;

			/*  print some information */
			if (tree->verb > (int)tree->depth - (int)height && file) {
				fprintf(file, "hikmeans: branch at depth %d: %6.1f %% completed\n", 
					tree->depth - height, (double) (k+1) / K * 100) ;
				fflush(file);
			}
		}
	}
	free (ids) ;
	return node ;
}

vl_bool CHIKMTree::vl_hikm_train(const vl_ikm_data *data, vl_uint32 N)
{
	//char fileName[MAX_PATH];
	//sprintf(fileName, "%s\\index\\info_train.txt", m_WorkSpace);
	//FILE* file = fopen(fileName, "wb");
	//if (!file) {
	//	return 0;
	//}

	m_VlHIKMTree->root = xmeans (m_VlHIKMTree, data, N, VL_MIN(m_VlHIKMTree->K, N), m_VlHIKMTree->depth, NULL);
	if (!m_VlHIKMTree->root) {
		return FALSE;
	}
	//fclose(file);

	return TRUE;
}

//vl_bool CHIKMTree::vl_hikm_train(const vl_ikm_data *data, vl_uint32 N)
//{
//	//char fileName[MAX_PATH];
//	//sprintf(fileName, "%s\\index\\info_train.txt", m_WorkSpace);
//	//FILE* file = fopen(fileName, "wb");
//	//if (!file) {
//	//	return 0;
//	//}
//	CString strMatchFile = "F:\\Kentucky\\index\\match\\feature_match_info.txt";
//	FILE* file = fopen( strMatchFile, "rb" );
//	int nmatch;
//	fscanf( file, "%d", &nmatch);
//	VlMatch* pairMatch = (VlMatch*)malloc( sizeof(VlMatch) * nmatch );
//	int i;
//	for ( i=0; i<nmatch; i++)
//	{
//		fscanf( file, "%d %d", &(pairMatch[i].n1), &(pairMatch[i].n2)); 
//	}
//	fclose(file);
//
//	VlMatchList** listMatch = (VlMatchList**)malloc( sizeof(VlMatchList*) * N );
//	memset( listMatch, 0, sizeof(VlMatchList*) * N );
//
//	for ( i = 0; i < nmatch; i++ )
//	{
//		VlMatchList* newItem = (VlMatchList*)malloc( sizeof(VlMatchList) );
//		newItem->ptID = pairMatch[i].n2;
//		newItem->next = listMatch[pairMatch[i].n1];
//		listMatch[pairMatch[i].n1] = newItem;
//	}
//
//	SAFE_FREE( pairMatch );
//
//	m_VlHIKMTree->root = xmeans (m_VlHIKMTree, data, N, VL_MIN(m_VlHIKMTree->K, N), m_VlHIKMTree->depth, NULL, listMatch);
//	if (!m_VlHIKMTree->root) {
//		return FALSE;
//	}
//	//fclose(file);
//	SAFE_FREE( listMatch );
//
//	return TRUE;
//}

void CHIKMTree::vl_hikm_push (vl_uint32 *asgn, vl_ikm_data const *data, int N)
{
  int i, d,
    M		= vl_hikm_get_ndims (),
    depth	= vl_hikm_get_depth ();
  
  /* for each datum */
  for(i = 0 ; i < N ; i++) {
    VlHIKMNode *node = m_VlHIKMTree->root ;
    d = 0 ;      
    while (node) {
      /*
      vl_uint best = 
        vl_ikm_push_one (vl_ikm_get_centers (node->filter),  
                         data + i * M,
                         M,
                         vl_ikm_get_K (node->filter)) ;
      */
      
      vl_uint32 best ;
      node->filter->vl_ikm_push (&best, data + i * M, 1) ;
      
      asgn [i*depth + d] = best ;
	  //cout<<"d = "<<d<<endl;
      ++ d ;
      
      if (!node->children) break ;
      node = node->children [best] ;
    }
  }
}


vl_uint32* CHIKMTree::vl_hikm_assignment_normalization(vl_uint32* asgn, int N)
{
	if (!asgn)
		return NULL;

	vl_uint32* assign_norm = (vl_uint32*)malloc(sizeof(vl_uint32) * N);
	memset(assign_norm, 0, sizeof(vl_uint32) * N);

	int i, j;
	vl_uint32 depth = m_VlHIKMTree->depth;
	vl_uint32 K		= m_VlHIKMTree->K;
	for (i = 0; i < N; i++){
		for (j = 0; j < depth; j++){
			vl_uint32 this_asgn = asgn[i*depth + j];
			if (this_asgn < K){
				assign_norm[i] += this_asgn * pow((double)K, (double)depth-j-1);
			}else{
                //cout<<"ERROR"<<endl;
				return NULL;
			}
		}
	}

	return assign_norm;
}

//static vl_bool write_tree(VlHIKMNode* node, vl_uint32 height, CString strSavePath, CString strInd)
static vl_bool write_tree(VlHIKMNode* node, vl_uint32 height, const char* strSavePath, const char* strInd)
{	
	//CString strFileClusterName = strSavePath + "\\" + strInd + ".txt";
	char strFileClusterName[MAX_PATH];
	strcpy( strFileClusterName, strSavePath );
	strcat( strFileClusterName, "\\" );
	strcat( strFileClusterName, strInd );
	strcat( strFileClusterName, ".txt" );
	if(!node->filter->vl_ikm_write_tree(strFileClusterName)){
		return FALSE;
	}

	if (height>1) {
		vl_uint32 k;
		for (k=0; k<node->filter->vl_ikm_get_k(); k++) {
			//CString strIndChild;
			//strIndChild.Format( _T("%3d"), k );
			//strIndChild.Replace( _T(" "), _T("0") );
			//strIndChild = strInd + strIndChild;
			char strIndChild[MAX_PATH];
			sprintf( strIndChild, "%s%4d", strInd, k );
			for ( int c = 0; c < strlen(strIndChild); c++ )
			{
				if ( strIndChild[c] == ' ')
				{
					strIndChild[c] = '0';
				}
			}

			if(!write_tree(node->children[k], height-1, strSavePath, strIndChild)){
				return FALSE;
			}
		}
	}
	return TRUE;
}

//vl_bool CHIKMTree::vl_hikm_write_tree(CString strSavePath)
vl_bool CHIKMTree::vl_hikm_write_tree(const char* strSavePath)
{
	char strFileName[MAX_PATH];
	strcpy( strFileName, strSavePath );
	strcat( strFileName, "\\cluster.txt" );
	//CString strFileName = strSavePath + "\\cluster.txt";
	FILE * file;
	file = fopen( strFileName,"wb");
	if (!file) {

		return FALSE;
	}

	if(fprintf(file,"%u %u %u %d %d\n", m_VlHIKMTree->M, m_VlHIKMTree->K, m_VlHIKMTree->depth,
		m_VlHIKMTree->max_niters, m_VlHIKMTree->verb) < 0){
			return FALSE;
	}

	fclose(file);

	if(!write_tree(m_VlHIKMTree->root, m_VlHIKMTree->depth, strSavePath, "0")){
		return FALSE;
	}

	return TRUE;
}



static vl_bool read_tree(VlHIKMNode*& node, vl_uint32 height, const char* strSavePath, const char* strInd)
{
	node = (VlHIKMNode*)malloc (sizeof(VlHIKMNode)) ;

	node-> filter   =  new CIKMTree(); 
	node-> children =  NULL ; 


	//CString strFileClusterName = strSavePath + "\\" + strInd + ".txt";
	char strFileClusterName[MAX_PATH];
	strcpy( strFileClusterName, strSavePath );
	strcat( strFileClusterName, "\\" );
	strcat( strFileClusterName, strInd );
	strcat( strFileClusterName, ".txt" );
	if(!node->filter->vl_ikm_read_tree( strFileClusterName )){
		return FALSE;
	}

	if (height > 1) {
		vl_uint32 K = node->filter->vl_ikm_get_k();
		node-> children = (VlHIKMNode**)malloc(sizeof(VlHIKMNode*) * K) ;

		for ( vl_uint32 k = 0; k < K; k++ ) {
			//CString strIndChild;
			//strIndChild.Format( _T("%3d"), k );
			//strIndChild.Replace( _T(" "), _T("0") );
			//strIndChild = strInd + strIndChild;
			char strIndChild[MAX_PATH];
			sprintf( strIndChild, "%s%4d", strInd, k );
			for ( int c = 0; c < strlen(strIndChild); c++ )
			{
				if ( strIndChild[c] == ' ')
				{
					strIndChild[c] = '0';
				}
			}

			if(!read_tree(node->children[k], height-1, strSavePath, strIndChild)){
				return FALSE;
			}
		}
	}
	return TRUE;
}

vl_bool CHIKMTree::vl_hikm_read_tree(const char* strSavePath)
{
	//CString strFileName = strSavePath + "\\cluster.txt";
	char strFileName[MAX_PATH];
	strcpy( strFileName, strSavePath );
	strcat( strFileName, "\\cluster.txt" );
	FILE * file = fopen( strFileName,"rb");
	if (!file) {
		return FALSE;
	}

	vl_hikm_new_VlHIKMTree();

	if(fscanf(file,"%u %u %u %d %d\n", &(m_VlHIKMTree->M), &(m_VlHIKMTree->K), &(m_VlHIKMTree->depth), &(m_VlHIKMTree->max_niters), &(m_VlHIKMTree->verb)) != 5){
		return FALSE;
	}
	fclose(file);

	if(!read_tree(m_VlHIKMTree->root, m_VlHIKMTree->depth, strSavePath, "0")){
		return FALSE;
	}

	return TRUE;
}


/************************************************************************/
/*   query                                                              */
/************************************************************************/
/*
typedef struct _paramQueryPush{
	VlHIKMNode		* node;
	VlQueryList		* pQueryList;
	VlForwardList	* pForwardList;
	vl_ikm_data		* data;
	vl_uint32		  N;
	vl_bool			* bUsed;
	vl_uint32		  height;
	vl_bool			  isDone;
	vl_bool			  isError;
}paramQueryPush;

static vl_bool query_push(	VlHIKMNode *  node, VlQueryList *  pQueryList, VlForwardList *  pForwardList,
			vl_ikm_data const*  data, vl_uint32	N, vl_bool	*  bUsed, vl_uint32 height)
{
	vl_uint32 * asgn = (vl_uint32*)malloc(sizeof(vl_uint32) * N);
	if (bUsed[height-1]) {
		if(!node->filter->vl_ikm_query_push(asgn, data, N, pQueryList, pForwardList)){
			return 0;
		}
		node->filter->vl_ikm_push(asgn, data, N);
	}else{
		node->filter->vl_ikm_push(asgn, data, N);
	}
	if (height > 1) {
		vl_uint32 k;
		for (k=0; k<node->filter->vl_ikm_get_k(); k++) {
			vl_uint32 partition_N ;
			vl_ikm_data *partition ;
			partition = vl_hikm_copy_subset(data, asgn, N, node->filter->vl_ikm_get_ndims(), k, &partition_N) ;
			
			if(!query_push(node->children[k], pQueryList, pForwardList, partition, partition_N, bUsed, height-1)){
				return 0;
			}
			free (partition) ;
			
		}
	}
	free(asgn);
	return 1;
}

DWORD WINAPI query_push_thread(void* lp)
{
	paramQueryPush* param = (paramQueryPush*)lp;

	VlHIKMNode		 *  node		= param->node;
	VlQueryList		 *  pQueryList	= param->pQueryList;
	VlForwardList	 *  pForwardList= param->pForwardList;
	vl_ikm_data		 *  data		= param->data;
	vl_uint32			N			= param->N;
	vl_bool			 *  bUsed		= param->bUsed;
 	vl_uint32			height		= param->height;

	if(!query_push(node, pQueryList, pForwardList, data, N, bUsed, height)){
		param->isError = 0;
	}else{
		param->isError = 1;
	}

	free(data);
	free(param);
	param->isDone = 1;
	return 1;
}

vl_bool CHIKMTree::vl_hikm_query_calc_score(vl_ikm_data const* data, vl_uint32 N)
{
	if (!m_VlHIKMTree || !m_VlHIKMTree->root) {
		return 0;
	}
	vl_hikm_free_query_list();
	m_pQueryList = (VlQueryList*)malloc(sizeof(VlQueryList)*m_nImage);
	memset(m_pQueryList, 0, sizeof(VlQueryList)*m_nImage);

	VlHIKMNode * node = m_VlHIKMTree->root;
	vl_uint32 * asgn = (vl_uint32*)malloc(sizeof(vl_uint32)*N);
	node->filter->vl_ikm_push(asgn,data,N);

	vl_uint32 k;
	vl_uint32 K = node->filter->vl_ikm_get_k();
	//paramQueryPush** pparam = (paramQueryPush**)malloc(sizeof(paramQueryPush*) * K);
	HANDLE*  pHandle = new HANDLE[K]; 
	for (k = 0; k < K; k++) {
		vl_uint32 partition_N ;
		vl_ikm_data *partition ;
		partition = vl_hikm_copy_subset(data, asgn, N, node->filter->vl_ikm_get_ndims(), k, &partition_N) ;
		
		//pparam[k] = (paramQueryPush*)malloc(sizeof(paramQueryPush));
		//paramQueryPush* param = pparam[k];
		paramQueryPush* param = (paramQueryPush*)malloc(sizeof(paramQueryPush));
		param->node = node->children[k];
		//param->pQueryList = (VlQueryList*)malloc(sizeof(VlQueryList)*m_nImage);//m_pQueryList;
		//memset(param->pQueryList, 0, sizeof(VlQueryList)*m_nImage);
		param->pQueryList = m_pQueryList;
		param->pForwardList = m_pForwardList;
		param->data = partition;
		param->N	= partition_N;
		param->bUsed = m_VlHIKMTree->bUsed;
		param->height = m_VlHIKMTree->depth - 1 ;
		param->isDone = 0; 
		
		DWORD dwThreadID;
		pHandle[k] = ::CreateThread(NULL,0,query_push_thread,param,CREATE_SUSPENDED,&dwThreadID);
		::SetThreadPriority(pHandle[k],THREAD_PRIORITY_NORMAL);
		::ResumeThread(pHandle[k]);
		//pHandle[k] = AfxBeginThread(query_push_thread, param);
		//_beginthread(query_push_thread, 0, param);		
	}
	
	WaitForMultipleObjects(K, pHandle, TRUE, INFINITE);
	for (k = 0; k<K; k++) {
		::CloseHandle(pHandle[k]);
	}

//	Sleep(150);
	vl_bool isErr = 1;
//	while (1) {
//		vl_bool isDone = 1;
//		for (k = 0; k < K; k++) {
//			isDone &= pparam[k]->isDone;
//		}
//		if (isDone) {
//			for (k = 0; k < K; k++) {
//				if (!pparam[k]->isError) {
//					isErr = 0;
//				}else{
//					vl_uint32 flag;
//					for (flag = 0; flag < m_nImage; flag++) {
//						m_pQueryList[flag].total_score += pparam[k]->pQueryList[flag].total_score;	
//					}
//				}
//				free(pparam[k]);
//			}
//			free(pparam);
//			
//			break;
//		}else{
//			Sleep(100);
//		}
//	}
	
	free(asgn);

	return isErr;
}

vl_uint32 CHIKMTree::vl_hikm_query_get_amount()
{
	if (!m_pQueryList) {
		return 0;
	}

	vl_uint32 n = 0;
	vl_uint32 flag = 0;
	while (flag < m_nImage) {
		if (m_pQueryList[flag].total_score > 0.0) {
			n ++;
		}
		flag ++;
	}
	return n;
}

VlQueryList* CHIKMTree::vl_hikm_query_pick_first(vl_uint32 * imageID)
{
	double best_score = -1;
	VlQueryList * bestQuery = NULL;
	vl_uint32 flag = 0;
	vl_uint32 ID = 0;

//	char fileName[MAX_PATH];
//	sprintf(fileName, "%s\\temp\\query.txt", m_WorkSpace);
//	FILE * file = fopen(fileName,"a+b");


	while(flag < m_nImage){
		// fprintf(file, "%8.6lf  ", m_pQueryList[flag].total_score);
		if (!m_pQueryList[flag].isPicked && m_pQueryList[flag].total_score > best_score) {
			best_score = m_pQueryList[flag].total_score;
			ID	= flag;
			bestQuery = m_pQueryList + flag;
		}
		flag ++;
	}
	bestQuery->isPicked = 1;

//	fprintf(file,"\n\n\n");
//	fclose(file);

	*imageID = ID;
	return bestQuery;
}

char * CHIKMTree::vl_hikm_get_image_name(vl_uint32 imageID)
{
	if (!m_pForwardList) {
		return NULL;
	}
	VlForwardList* iter = m_pForwardList + imageID;
	return iter->imageName;
}
*/
/************************************************************************/
/*  write the information                                               */
/************************************************************************/
/*vl_bool CHIKMTree::vl_hikm_write_one_query(const char * query_name, VlQueryList* item, char* image_name, const char* control)
{
	if (!item) {
		return 0;
	}

	char   fileName[MAX_PATH];
	sprintf(fileName, "%s\\query\\%s", m_WorkSpace, query_name);

	FILE * file;
	file = fopen(fileName, control);
	if (!file) {
		return 0;
	}
	
	fprintf(file, "%s %lf\n", image_name, item->total_score);

	fclose(file);
	return 1;
}*/
/*
static vl_bool write_tree(VlHIKMNode* node, vl_uint32 height)
{
	if(!node->filter->vl_ikm_write_tree()){
		return 0;
	}
	
	if (height>1) {
		vl_uint32 k;
		for (k=0; k<node->filter->vl_ikm_get_k(); k++) {
			if(!write_tree(node->children[k], height-1)){
				return 0;
			}
		}
	}
	return 1;
}

vl_bool CHIKMTree::vl_hikm_write_tree()
{
	char   fileName[MAX_PATH];
	sprintf(fileName, "%s\\index\\tree.txt", m_WorkSpace);

	FILE * file;
	file = fopen(fileName,"wb");
	if (!file) {
		return 0;
	}

	fprintf(file,"%u %u %u %d %d\n", m_VlHIKMTree->M, m_VlHIKMTree->K, m_VlHIKMTree->depth,
									m_VlHIKMTree->max_niters, m_VlHIKMTree->verb); 
	fclose(file);

	if(!write_tree(m_VlHIKMTree->root, m_VlHIKMTree->depth)){
		return 0;
	}

	return 1;
}

/************************************************************************/
/* read the information                                                 */
/************************************************************************/
/*
static vl_bool read_tree(VlHIKMNode* node, const char * directory_name, vl_uint32 height)
{
	node->filter = new CIKMTree();
	node->filter->vl_ikm_set_workspace(directory_name);
	if(!node->filter->vl_ikm_read_tree()){
		return 0;
	}
	if (height > 1) {
		vl_uint32 K = node->filter->vl_ikm_get_k();
		node->children = (VlHIKMNode**)malloc(sizeof(VlHIKMNode*)*K);
		vl_uint32 k;
		for (k=0; k<K; k++) {
			char  sub_folder_name[MAX_PATH];
			sprintf(sub_folder_name,"%s\\%u",directory_name, k);
			node->children[k] = (VlHIKMNode*)malloc(sizeof(VlHIKMNode));
			memset(node->children[k], 0, sizeof(VlHIKMNode));
			if(!read_tree(node->children[k], sub_folder_name, height-1)){
				return 0;
			}
		}
	}
	return 1;
}

vl_bool CHIKMTree::vl_hikm_read_tree()
{
	char   fileName[MAX_PATH];
	sprintf(fileName, "%s\\index\\tree.txt", m_WorkSpace);

	FILE * file;
	file = fopen(fileName,"rb");
	if (!file) {
		return 0;
	}

	int flag = 1;
	if(fscanf(file,"%d\n", &flag) != 1){
		return 0;
	}
	
	vl_hikm_new_VlHIKMTree();

	if(fscanf(file,"%u %u %u %d %d\n", &(m_VlHIKMTree->M), &(m_VlHIKMTree->K), &(m_VlHIKMTree->depth), &(m_VlHIKMTree->max_niters), &(m_VlHIKMTree->verb)) != 5){
		return 0;
	}

	if(flag){
		m_VlHIKMTree->bUsed = (vl_bool*)malloc(sizeof(vl_bool) * m_VlHIKMTree->K);
		vl_uint32 d;
		for (d=0; d<m_VlHIKMTree->depth; d++) {
			if(fscanf(file,"%d", &(m_VlHIKMTree->bUsed[d])) != 1){
				return 0;
			}
		}
	}

	fclose(file);

	char index_path[MAX_PATH];
	sprintf(index_path, "%s\\index\\root", m_WorkSpace);
	m_VlHIKMTree->root = (VlHIKMNode*)malloc(sizeof(VlHIKMNode));
	memset(m_VlHIKMTree->root, 0, sizeof(VlHIKMNode));
	if(!read_tree(m_VlHIKMTree->root, index_path, m_VlHIKMTree->depth)){
		return 0;
	}

	return 1;
}
*/