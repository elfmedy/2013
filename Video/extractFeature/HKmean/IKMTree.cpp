// IKMTree.cpp: implementation of the CIKMTree class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IKMTree.h"
#include "stdlib.h"
#include "memory.h"
#include "math.h"
#include "stdio.h"
#include "string.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
//#define new DEBUG_NEW
#endif


//#define KD_TREE

#define N_KD_TREE 8

/* the maximum number of keypoint NN candidates to check during BBF search */
#define KDTREE_BBF_MAX_NN_CHKS 200



//#define COSINE_DISTANCE
#define NORM 512



/* Period parameters */  
#define NN 624
#define MM 397
#define MATRIX_A   0x9908b0dfUL /* constant vector a */
#define UPPER_MASK 0x80000000UL /* most significant w-r bits */
#define LOWER_MASK 0x7fffffffUL /* least significant r bits */

static unsigned long mt[NN]; /* the array for the state vector  */
static int mti=NN+1; /* mti==N+1 means mt[N] is not initialized */

/** @brief Seed the state of the random number generator
**
** @param s seed.
**/
void vl_rand_seed (vl_uint32 s)
{
	mt[0]= s & 0xffffffffUL;
	for (mti=1; mti<NN; mti++) {
		mt[mti] = (1812433253UL * (mt[mti-1] ^ (mt[mti-1] >> 30)) + mti); 
		/* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
		/* In the previous versions, MSBs of the seed affect   */
		/* only MSBs of the array mt[].                        */
		/* 2002/01/09 modified by Makoto Matsumoto             */
		mt[mti] &= 0xffffffffUL;
		/* for >32 bit machines */
	}
}

/** @brief Generate a random UINT32 
** @return a random number in [0, 0xffffffff]. 
**/
vl_uint32 vl_rand_uint32 ()
{
	unsigned long y;
	static unsigned long mag01[2]={0x0UL, MATRIX_A};
	/* mag01[x] = x * MATRIX_A  for x=0,1 */
	
	if (mti >= NN) { /* generate N words at one time */
		int kk;
		
		if (mti == NN+1)   /* if init_genrand() has not been called, */
			vl_rand_seed (5489UL); /* a default initial seed is used */
		
		for (kk=0;kk<NN-MM;kk++) {
			y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
			mt[kk] = mt[kk+MM] ^ (y >> 1) ^ mag01[y & 0x1UL];
		}
		for (;kk<NN-1;kk++) {
			y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
			mt[kk] = mt[kk+(MM-NN)] ^ (y >> 1) ^ mag01[y & 0x1UL];
		}
		y = (mt[NN-1]&UPPER_MASK)|(mt[0]&LOWER_MASK);
		mt[NN-1] = mt[MM-1] ^ (y >> 1) ^ mag01[y & 0x1UL];
		
		mti = 0;
	}
	
	y = mt[mti++];
	
	/* Tempering */
	y ^= (y >> 11);
	y ^= (y << 7) & 0x9d2c5680UL;
	y ^= (y << 15) & 0xefc60000UL;
	y ^= (y >> 18);
	
	return y;
}


/************************************************************************/
/*  construction and destruction                                        */
/************************************************************************/

CIKMTree::CIKMTree()
{
	m_pVlIKMFilt = NULL;
}

CIKMTree::~CIKMTree()
{
	vl_ikm_delete_tree();
}

/************************************************************************/
/*  Train						                                        */
/************************************************************************/

void CIKMTree::vl_ikm_init_rand(vl_uint32 M, vl_uint32 K)
{
	vl_uint32 k;
	vl_uint32 i ;
	
	if (m_pVlIKMFilt-> centers) free(m_pVlIKMFilt->centers) ;
	m_pVlIKMFilt-> K = K ;
	m_pVlIKMFilt-> M = M ; 
	m_pVlIKMFilt-> centers = (vl_ikm_data*)malloc (sizeof(vl_ikm_data) * M * K) ;
	
	for (k = 0 ; k < K ; ++ k) {
		for (i = 0 ; i < M ; ++ i) {
			m_pVlIKMFilt-> centers [k * M + i] = (vl_ikm_data) (vl_rand_uint32 ()) ;
		}
	}
}


int cmp_pair (void const *a, void const *b)
{
	pair_t *pa = (pair_t *) a;
	pair_t *pb = (pair_t *) b;
	vl_int32 d = pa->w - pb->w ;
	if (d) return d ;
	/* break ties (qsort not stable) */
	return pa->j - pb->j;
}

// The number of training data N should be greater than the number of clusters K
void CIKMTree::vl_ikm_init_rand_data(const vl_ikm_data *data, vl_uint32 M, vl_uint32 N, vl_uint32 K)
{
	vl_uint32 i, j, k ;
	pair_t *pairs = (pair_t *)malloc (sizeof(pair_t) * N);
	
	if (m_pVlIKMFilt-> centers) free(m_pVlIKMFilt->centers) ;
	m_pVlIKMFilt-> K = K ;
	m_pVlIKMFilt-> M = M ; 
	m_pVlIKMFilt-> centers = (vl_ikm_data*)malloc (sizeof(vl_ikm_data) * M * K) ;
	
	/* permute the data randomly */
	for (j = 0 ; j < N ; ++j) {
		pairs[j].j = j ;
		pairs[j].w = ((vl_int32) vl_rand_uint32 ()) >> 2 ;
	}
	
	qsort (pairs, N, sizeof(pair_t), cmp_pair);
	
	/* initialize centers from random data points */
	for (k = 0 ; k < K ; ++ k) {
		for (i = 0 ; i < M ; ++ i) {
			m_pVlIKMFilt-> centers [k * M + i] = (vl_ikm_data)data[pairs[k].j * M + i] ;
		}
	}  
	
	free (pairs) ;
}

vl_ikm_acc CIKMTree::vl_ikm_computer_distance(const vl_ikm_data* data1, const vl_ikm_data* data2)
{
	vl_uint32 M = m_pVlIKMFilt->M;
	
	vl_uint32 i;
	vl_ikm_acc distance = 0;

#ifdef COSINE_DISTANCE

	for (i=0; i<M; i++)				// cosine
	{
		distance += data1[i] * data2[i];
	}
	distance = NORM * NORM - distance;

#else

	for (i = 0; i < M; ++i) {
		int delta = 1 - VL_MIN(data1[i],data2[i]);             // Hamming distance  我修改了
		distance += delta;
	}

#endif

	return distance;
}

vl_uint32* CIKMTree::vl_ikm_assign_to_hist(vl_uint32* assign, vl_uint32 n)
{
	if (!assign) {
		return NULL;
	}

	vl_uint32* count = (vl_uint32*)malloc(sizeof(vl_uint32) * m_pVlIKMFilt->K);
	memset(count, 0, sizeof(vl_uint32) *  m_pVlIKMFilt->K);

	vl_uint32 i;
	for (i = 0; i < n; i++) {
		vl_uint32 this_asgn = assign[i];
		++count[this_asgn];
	}

	return count;
}
#ifdef KD_TREE
DWORD WINAPI kd_knn_thread(void* lp)
{
	paramKD_KNN* param = (paramKD_KNN*)lp;
	struct feature* feat_data = param->feat;
	struct kd_node* kd_root	  = param->kd_root;
	vl_uint32 K = param->K;
	struct feature* centers_kd = param->centers_kd;

	//--------KD Tree算法-----------

	struct feature** nbrs;
	/*int knn = kdtree_bbf_knn( kd_root, feat_data, 1, &nbrs, KDTREE_BBF_MAX_NN_CHKS );*/
	/*int knn = kdtree_bbf_knn( kd_root, feat_data, 1, &nbrs, K );*/
	int knn = kdtree_bbf_knn( kd_root, feat_data, 1, &nbrs, K/8 );
	if (knn != 1)
	{
		fatal_error("Number of knn error, %s, line %d", __FILE__, __LINE__);
	}

	int index_knn = check_index(centers_kd, nbrs[0], K);
	if (index_knn < 0)
	{
		fatal_error("Index of knn error, %s, line %d", __FILE__, __LINE__);
	}
	double dist = descr_dist_sq( feat_data, nbrs[0] );

	SAFE_FREE(nbrs);

	*(param->dist) = dist;
	*(param->index_knn) = index_knn;
	
	return 1;
}
#endif



//vl_bool CIKMTree::vl_ikm_train(const vl_ikm_data *data, vl_uint32 N, FILE* file, VlMatchList** listMatch)
//{
//	vl_bool err = 1;		
//
//	if (m_pVlIKMFilt->verb) {
//		/* print information */
//		if (file) {
//			fprintf (file, "ikm: training with %d data\n",  N) ;
//			fprintf (file, "ikm: %d clusters\n",  m_pVlIKMFilt -> K) ;
//		}else{
//			printf("ikm: training with %d data\n",  N);
//			printf("ikm: %d clusters\n",  m_pVlIKMFilt -> K);
//		}
//	}
//	
//	int			iter;
//	vl_uint32	i, j, k, t;
//	vl_uint32	K = m_pVlIKMFilt-> K;
//	vl_uint32	M = m_pVlIKMFilt-> M;
//	vl_ikm_data * centers = m_pVlIKMFilt->centers ;  
//	vl_ikm_acc	* centers_sum = (vl_ikm_acc*)malloc(sizeof(vl_ikm_acc) * M * K);
//	vl_uint32  *asgn    = (vl_uint32*)malloc (sizeof(vl_uint32) * N) ;
//	vl_uint32  *counts  = (vl_uint32*)malloc (sizeof(vl_uint32) * K) ;
//
//	//vl_ikm_acc* dist_sum = (vl_ikm_acc*)malloc(sizeof(vl_ikm_acc) * m_pVlIKMFilt->max_niters);
//	//memset(dist_sum, 0, sizeof(vl_ikm_acc) * m_pVlIKMFilt->max_niters);
//	
//
//	vl_uint32* dist = (vl_uint32*)malloc( sizeof(vl_uint32) * N * K );
//
//	for ( iter = 0 ; 1 ; ++ iter ) {
//		vl_tic();
//		if (m_pVlIKMFilt->verb && !file) {
//			printf("ikm: the %dth iteration...\n", iter+1);
//		}
//
//		vl_bool done = 1 ;
//		
//		/* ---------------------------------------------------------------
//		*                                               Calc. assignments
//		* ------------------------------------------------------------ */
//
//		for (j = 0 ; j < N ; ++j) {
//			//---------找到最近邻-----------
//			//vl_uint32  best = (vl_uint32)-1 ;
//
//
//				//--------逐个比较法------------
//			//vl_ikm_acc best_dist = 0 ;
//			for (k = 0; k < K; ++k) {
//				//vl_ikm_acc dist = 0;
//
//				/* compute distance with this center */
//				dist[j*K + k] = vl_ikm_computer_distance(data+j*M , centers+k*M);
//
//				/* compare with current best */
//				//if (best == (vl_uint32)-1 || dist < best_dist) {
//				//	best = k ;
//				//	best_dist = dist ;
//				//}
//			}
//
//			//if (asgn [j] != best) {
//			//	asgn [j] = best ;
//			//	done = 0 ;
//			//}
//		}
//		
//		for ( i = 0; i < N; i++ )
//		{
//			vl_uint32* dist_one = (vl_uint32*)malloc( sizeof(vl_uint32) * K );
//			memcpy( dist_one, dist + i*K, sizeof(vl_uint32) * K );
//			
//			VlMatchList* item = listMatch[ i ];
//			while( item ){
//				for ( k = 0; k < K; ++k )
//				{
//					dist_one[ k ] += 0.2 * dist[ item->ptID * K + k];
//				}
//
//				item = item->next;
//			}
//
//			vl_uint32  best = (vl_uint32)-1 ;
//			vl_uint32  best_dist = 0 ;
//			for (k = 0; k < K; ++k) {
//				if (best == (vl_uint32)-1 || dist_one[k] < best_dist) {
//					best = k ;
//					best_dist = dist_one[k] ;
//				}
//			}
//
//			if (asgn [i] != best) {
//				asgn [i] = best ;
//				done = 0 ;
//			}
//
//			SAFE_FREE( dist_one );
//		}
//
//		//for ( i = 0; i < nmatch; i++ ){
//		//	int exist1 = -1, exist2 = -1;
//		//	for ( j = 0; j < N; j++ )
//		//	{
//		//		if ( indPointID[j] == pairMatch[i].n1 )
//		//		{
//		//			exist1 = j;
//		//		}else if ( indPointID[j] == pairMatch[i].n2 )
//		//		{
//		//			exist2 = j;
//		//		}
//
//		//		if ( exist1 >=0 && exist2 >=0 )
//		//		{
//		//			break;
//		//		}
//		//	}
//
//		//	if ( exist1 >= 0 && exist2 >= 0 )
//		//	{
//		//		for ( k = 0; k < K; ++k )
//		//		{
//		//			dist[exist1*K + k] = dist[exist1*K + k] + 0.3 * dist[exist2*K + k];
//		//		}
//		//	}
//
//		//}
//
//		//for (j = 0 ; j < N ; ++j) {
//		//	//---------找到最近邻-----------
//		//	vl_uint32  best = (vl_uint32)-1 ;
//
//
//		//	//--------逐个比较法------------
//		//	vl_uint32 best_dist = 0 ;
//		//	for (k = 0; k < K; ++k) {
//		//		//vl_ikm_acc dist = 0;
//
//		//		/* compute distance with this center */
//		//		//dist[j*K + k] = vl_ikm_computer_distance(data+j*M , centers+k*M);
//
//		//		/* compare with current best */
//		//		if (best == (vl_uint32)-1 || dist[j*K + k] < best_dist) {
//		//			best = k ;
//		//			best_dist = dist[j*K + k] ;
//		//		}
//		//	}
//
//		//	if (asgn [j] != best) {
//		//		asgn [j] = best ;
//		//		done = 0 ;
//		//	}
//		//}
//
//		/* stopping condition */
//		if (done || iter == m_pVlIKMFilt->max_niters)
//			break ;
//		
//		/* -----------------------------------------------------------------
//		*                                                     Calc. centers
//		* -------------------------------------------------------------- */
//		
//		/* re-compute centers */
//		memset (centers_sum, 0, sizeof (vl_ikm_acc) * M * K);
//		memset (counts,  0, sizeof (vl_uint32) * K);
//
//		for (j = 0; j < N; ++j) {
//			vl_uint32 this_center = asgn [j] ;      
//			++ counts [this_center] ;      
//			for (i = 0; i < M; ++i)
//				centers_sum[this_center * M + i] += (vl_ikm_acc)data[j * M + i];
//		}
//		
//		for (k = 0; k < K; ++k) {
//			vl_uint32 n = counts [k];
//			if (n > 0xffffff) {
//				err = 0 ;
//			}
//			if (n > 0) {
//
//				for (i = 0; i < M; ++i){
//					centers_sum[k * M + i] /= n;
//					centers[k * M + i] = (vl_ikm_data)centers_sum[k * M + i];
//				}
//
//			} else {
//			/* 
//			If no data are assigned to the center, it is not changed
//			with respect to the previous iteration, so we do not do anything. 
//				*/
//			}
//		}
//
//
//		vl_ikm_acc dist_sum = 0;
//		for (j=0; j<N; j++)
//		{
//			vl_uint32 this_center = asgn [j] ;
//			dist_sum += dist[j*K + this_center]; 
//		}
//
//		vl_ikm_acc dist_sum_match = 0;
//		vl_ikm_acc total_match = 0;
//		for ( i = 0; i < N; i++ ){
//			if ( ! listMatch[i] )
//				continue;
//			
//			VlMatchList* item = listMatch[ i ];
//			while( item ){
//				if ( asgn[i] != asgn[item->ptID] )
//					dist_sum_match ++;
//				
//				total_match++;
//				item = item->next;
//			}
//
//
//		}
//
//		printf("ikm: the sum of distance is %15llu// %6llu/%8llu (%lf secs).\n", dist_sum, dist_sum_match, total_match, vl_toc());
//		//printf("ikm: the sum of time is (%lf secs).\n\n", vl_toc());
//	}
//
//	if (dist)		free (dist);
//	if (centers_sum)free (centers_sum);
//	if (counts)		free (counts) ;
//	if (asgn)		free (asgn) ;
//
//	//if (m_pVlIKMFilt->verb) {
//	//	if (file) {
//	//		fprintf(file, "Cost %lf secs.\n", vl_toc());
//	//		fflush(file);
//	//	}else{
//	//		printf("Cost %lf secs.\n", vl_toc());
//	//	}
//	//}
//
//	return err;
//}
//

vl_bool CIKMTree::vl_ikm_train(const vl_ikm_data *data, vl_uint32 N, FILE* file)
{
	vl_bool err = 1;		

	if (m_pVlIKMFilt->verb) {
		/* print information */
		if (file) {
			fprintf (file, "ikm: training with %d data\n",  N) ;
			fprintf (file, "ikm: %d clusters\n",  m_pVlIKMFilt -> K) ;
		}else{
			printf("ikm: training with %d data\n",  N);
			printf("ikm: %d clusters\n",  m_pVlIKMFilt -> K);
		}
	}

	int			iter;
	vl_uint32	i, j, k;
	vl_uint32	K = m_pVlIKMFilt-> K;
	vl_uint32	M = m_pVlIKMFilt-> M;
	vl_ikm_data * centers = m_pVlIKMFilt->centers ;  
	vl_ikm_acc	* centers_sum = (vl_ikm_acc*)malloc(sizeof(vl_ikm_acc) * M * K);
	vl_uint32  *asgn    = (vl_uint32*)malloc (sizeof(vl_uint32) * N) ;
	vl_uint32  *counts  = (vl_uint32*)malloc (sizeof(vl_uint32) * K) ;

	//vl_ikm_acc* dist_sum = (vl_ikm_acc*)malloc(sizeof(vl_ikm_acc) * m_pVlIKMFilt->max_niters);
	//memset(dist_sum, 0, sizeof(vl_ikm_acc) * m_pVlIKMFilt->max_niters);

	for (iter = 0 ; 1 ; ++ iter) {
		vl_tic();
		if (m_pVlIKMFilt->verb && !file) {
			printf("ikm: the %dth iteration...\n", iter+1);
		}

		vl_bool done = 1 ;

		/* ---------------------------------------------------------------
		*                                               Calc. assignments
		* ------------------------------------------------------------ */
#ifdef KD_TREE
		//------------Build KD Tree-------------------------------
		struct feature* centers_kd	= sift_molding( centers, NULL, K, M );
		struct kd_node* kd_root[N_KD_TREE];	

		srand( (unsigned)time( NULL ) );
		for ( t=0; t<N_KD_TREE; t++ )
		{
			kd_root[t] = kdtree_build( centers_kd, K );
		}

#endif

		for (j = 0 ; j < N ; ++j) {
			//---------找到最近邻-----------
			vl_uint32  best = (vl_uint32)-1 ;

#ifdef KD_TREE

			double best_dist = 0;
			struct feature* feat_data = sift_molding( data+j*M, NULL, 1, M);
			double	dist[N_KD_TREE];	
			int index_knn[N_KD_TREE];
			HANDLE	pHandle[N_KD_TREE]; 
			paramKD_KNN param[N_KD_TREE] ;

			for ( t=0; t<N_KD_TREE; t++ )
			{
				param[t].feat = feat_data;
				param[t].kd_root = kd_root[t];
				param[t].centers_kd = centers_kd;
				param[t].K	= K;
				param[t].dist = dist + t;
				param[t].index_knn = index_knn + t;

				DWORD dwThreadID;
				pHandle[i] = ::CreateThread(NULL,0,kd_knn_thread,param + t,CREATE_SUSPENDED,&dwThreadID);
				::SetThreadPriority(pHandle[i],THREAD_PRIORITY_NORMAL);
				::ResumeThread(pHandle[i]);

			}

			WaitForMultipleObjects( N_KD_TREE, pHandle, TRUE, INFINITE );
			for ( t = 0; t<N_KD_TREE; t++) {
				::CloseHandle(pHandle[t]);
			}	

			for ( t=0; t<N_KD_TREE; t++ )
			{
				if ( dist[t] < best_dist || best == (vl_uint32)-1 )
				{
					best = (vl_uint32)index_knn[t];
					best_dist = dist[t];
				}

			}

			SAFE_FREE(feat_data);

#else
			//--------逐个比较法------------
			vl_ikm_acc best_dist = 0 ;
			for (k = 0; k < K; ++k) {
				vl_ikm_acc dist = 0;

				/* compute distance with this center */
				dist = vl_ikm_computer_distance(data+j*M , centers+k*M);

				/* compare with current best */
				if (best == (vl_uint32)-1 || dist < best_dist) {
					best = k ;
					best_dist = dist ;
				}
			}
#endif

			if (asgn [j] != best) {
				asgn [j] = best ;
				done = 0 ;
			}
		}

#ifdef KD_TREE
		for ( t=0; t<N_KD_TREE; t++)
		{
			kdtree_release( kd_root[t] );
		}

		SAFE_FREE(centers_kd);
#endif

		/* stopping condition */
		if (done || iter == m_pVlIKMFilt->max_niters)
			break ;

		/* -----------------------------------------------------------------
		*                                                     Calc. centers
		* -------------------------------------------------------------- */

		/* re-compute centers */
		memset (centers_sum, 0, sizeof (vl_ikm_acc) * M * K);
		memset (counts,  0, sizeof (vl_uint32) * K);

		for (j = 0; j < N; ++j) {
			vl_uint32 this_center = asgn [j] ;      
			++ counts [this_center] ;      
			for (i = 0; i < M; ++i)
				centers_sum[this_center * M + i] += (vl_ikm_acc)data[j * M + i];
		}

		for (k = 0; k < K; ++k) {
			vl_uint32 n = counts [k];
			if (n > 0xffffff) {
				err = 0 ;
			}
			if (n > 0) {
#ifdef COSINE_DISTANCE
				vl_ikm_acc norm = 0;
				for (i = 0; i < M; ++i){
					centers_sum[k * M + i] /= n;
					norm += centers_sum[k * M + i] * centers_sum[k * M + i];
				}
				double ratio = NORM / sqrt((double)norm);
				for (i = 0; i < M; ++i){
					centers[k * M + i] = (vl_ikm_data)( ratio * centers_sum[k * M + i] );
				}
#else
				for (i = 0; i < M; ++i){
					centers_sum[k * M + i] /= n;
					centers[k * M + i] = (vl_ikm_data)centers_sum[k * M + i];
				}
#endif
			} else {
				/* 
				If no data are assigned to the center, it is not changed
				with respect to the previous iteration, so we do not do anything. 
				*/
			}
		}
		vl_uint64 dist_sum = 0;
		for (j=0; j<N; j++)
		{
			vl_uint32 this_center = asgn [j] ;
			dist_sum += vl_ikm_computer_distance(data + j*M, centers + this_center*M);
		}
		printf("ikm: the sum of distance is %15llu (%lf secs).\n\n", dist_sum, vl_toc());
		//printf("ikm: the sum of time is (%lf secs).\n\n", vl_toc());
	}

	if (centers_sum)free (centers_sum);
	if (counts)		free (counts) ;
	if (asgn)		free (asgn) ;

	//if (m_pVlIKMFilt->verb) {
	//	if (file) {
	//		fprintf(file, "Cost %lf secs.\n", vl_toc());
	//		fflush(file);
	//	}else{
	//		printf("Cost %lf secs.\n", vl_toc());
	//	}
	//}

	return err;
}



/************************************************************************/
/*  Project the data		                                            */
/************************************************************************/
vl_uint32 CIKMTree::vl_ikm_push_one(const vl_ikm_data *data)
{
	vl_ikm_data const *centers = m_pVlIKMFilt->centers;
	vl_uint32 K = m_pVlIKMFilt->K;
	vl_uint32 M = m_pVlIKMFilt->M;
	
	vl_uint32 k;
	
	/* assign data to centers */
	vl_uint32		best      = (vl_uint32)-1 ;
	

#ifdef KD_TREE
	//------------Build KD Tree-------------------------------
	struct feature* centers_kd	= sift_molding( centers, NULL, K, M );
	struct kd_node* kd_root		= kdtree_build( centers_kd, K );

	//--------KD Tree算法-----------
	struct feature* feat_data = sift_molding( data, NULL, 1, M);
	struct feature** nbrs;
	int knn = kdtree_bbf_knn( kd_root, feat_data, 1, &nbrs, KDTREE_BBF_MAX_NN_CHKS );
	if (knn != 1)
	{
		fatal_error("Number of knn error, %s, line %d", __FILE__, __LINE__);
	}

	int index_knn = check_index(centers_kd, nbrs[0], K);
	if (index_knn < 0)
	{
		fatal_error("Index of knn error, %s, line %d", __FILE__, __LINE__);
	}

	best = (vl_uint32)index_knn;
	SAFE_FREE(feat_data);
	SAFE_FREE(nbrs[0]);
	SAFE_FREE(centers_kd);
	kdtree_release( kd_root );

#else
	
	vl_ikm_acc		best_dist = 0 ;
	for(k = 0 ; k < K ; ++k) {
		vl_ikm_acc dist = 0 ;
		
		/* compute distance with this center */
		dist = vl_ikm_computer_distance(data, centers + k*M);
		
		/* compare with current best */
		if (best == (vl_uint32)-1 || dist < best_dist) {
			best = k  ;
			best_dist = dist ;
		}
	}

#endif

	return best;
}

vl_uint32* CIKMTree::vl_ikm_push_one_soft( vl_ikm_data* data, int N )
{
	vl_ikm_data const *centers = m_pVlIKMFilt->centers;
	vl_uint32 K = m_pVlIKMFilt->K;
	vl_uint32 M = m_pVlIKMFilt->M;

	vl_uint32 * asgn = ( vl_uint32* )malloc( sizeof(vl_uint32) * N );
	memset(asgn, 127, N);

	int i, j;
	vl_uint32 k;
	for ( i=0; i<N; i++ )
	{
		/* assign data to centers */
		vl_uint32		best      = (vl_uint32)-1 ;
		vl_ikm_acc		best_dist = 0 ;

		for(k = 0 ; k < K ; ++k) {
			for ( j=0; j<i; j++ )
			{
				if ( asgn[j] == k)
					break;
			}
			
			if ( j < i)
				continue;

			vl_ikm_acc dist = 0 ;
			/* compute distance with this center */
			dist = vl_ikm_computer_distance(data, centers + k*M);

			/* compare with current best */
			if (best == (vl_uint32)-1 || dist < best_dist) {
				best = k  ;
				best_dist = dist ;
			}
		}

		asgn[i] = best;

	}

	return asgn;

}

void CIKMTree::vl_ikm_push(vl_uint32 *asgn, const vl_ikm_data *data, vl_uint32 N)
{
#ifdef KD_TREE

	vl_ikm_data const *centers = m_pVlIKMFilt->centers;
	vl_uint32 M = m_pVlIKMFilt->M;
	vl_uint32 K = m_pVlIKMFilt->K;
	//------------Build KD Tree-------------------------------
	struct feature* centers_kd	= sift_molding( centers, NULL, K, M );
	struct kd_node* kd_root		= kdtree_build( centers_kd, K );

	vl_ikm_push_kd(asgn, data, N, kd_root, centers_kd);
	
	SAFE_FREE(centers_kd);
	kdtree_release( kd_root );

#else
	for(vl_uint32 i=0 ; i < N ; ++i) {
		asgn[i] = vl_ikm_push_one (data + i * m_pVlIKMFilt->M);
	}
	
#endif

}

#ifdef KD_TREE
void CIKMTree::vl_ikm_push_kd(vl_uint32 *asgn, const vl_ikm_data *data, vl_uint32 N, 
							struct kd_node* kd_root, struct feature* centers_kd)
{	
	vl_uint32 i;
	vl_uint32 M = m_pVlIKMFilt->M;
	vl_uint32 K = m_pVlIKMFilt->K;

	for (i = 0; i < N; i++)
	{
		//--------KD Tree算法-----------
		struct feature* feat_data = sift_molding( data+i*M, NULL, 1, M);
		struct feature** nbrs;
		int knn = kdtree_bbf_knn( kd_root, feat_data, 1, &nbrs, KDTREE_BBF_MAX_NN_CHKS );
		if (knn != 1)
		{
			fatal_error("Number of knn error, %s, line %d", __FILE__, __LINE__);
		}

		int index_knn = check_index(centers_kd, nbrs[0], K);
		if (index_knn < 0)
		{
			fatal_error("Index of knn error, %s, line %d", __FILE__, __LINE__);
		}

		asgn[i] = (vl_uint32)index_knn;

		SAFE_FREE(feat_data);
		SAFE_FREE(nbrs[0]);
	}
	
}
#endif
/************************************************************************/
/*  new and delete the element                                          */
/************************************************************************/
void CIKMTree::vl_ikm_delete_tree()
{
	if (m_pVlIKMFilt) {
		if (m_pVlIKMFilt->centers)     free(m_pVlIKMFilt->centers) ;
		free (m_pVlIKMFilt) ;
		m_pVlIKMFilt = NULL;
	}
}

void CIKMTree::vl_ikm_new_tree()
{
	if (m_pVlIKMFilt) {	
		vl_ikm_delete_tree();
	}
	m_pVlIKMFilt =  (VlIKMFilt*)malloc (sizeof(VlIKMFilt)) ;
	m_pVlIKMFilt -> centers = NULL ;
	m_pVlIKMFilt -> M = 0 ;
	m_pVlIKMFilt -> K = 0 ;
	m_pVlIKMFilt -> max_niters = 200;
	m_pVlIKMFilt -> verb       = 1 ;
}

/************************************************************************/
/*   write the information                                              */
/************************************************************************/
vl_bool CIKMTree::vl_ikm_write_tree(const char* cluster_file_name)
{
	FILE * file;
	file = fopen(cluster_file_name,"wb");
	if (!file) {
		return FALSE;
	}

									// M, K, max_niters, verb
	if(fprintf(file, "%u %u %d %d\n", m_pVlIKMFilt->M, m_pVlIKMFilt->K, 
									m_pVlIKMFilt->max_niters, m_pVlIKMFilt->verb) < 0){
		return FALSE;
	}
	
	vl_uint32 i, j;
	for (i = 0; i < m_pVlIKMFilt->K; i++) {  // center
		for (j = 0; j < m_pVlIKMFilt->M; j++) {
			if(fprintf(file,CLUSTER_WRITE_CONTROL, m_pVlIKMFilt->centers[i*(m_pVlIKMFilt->M)+j]) < 0){
				return FALSE;
			}
		}
		if(fprintf(file,"\n") < 0){
			return FALSE;
		}
	}
	fclose(file);
	return TRUE;
}

/************************************************************************/
/*   read the information                                               */
/************************************************************************/
vl_bool CIKMTree::vl_ikm_read_tree(const char* cluster_file_name)
{
	FILE * file;
	file = fopen(cluster_file_name,"rb");
	if (!file) {
		return 0;
	}

	vl_ikm_new_tree();
	
			// M, K, max_niters, verb
	if(fscanf(file, "%u %u %d %d\n", &(m_pVlIKMFilt->M), &(m_pVlIKMFilt->K),
									&(m_pVlIKMFilt->max_niters), &(m_pVlIKMFilt->verb))	!=	4){
		return FALSE;
	}

	
	m_pVlIKMFilt->centers = (vl_ikm_data*)malloc(sizeof(vl_ikm_data)* (m_pVlIKMFilt->K) * (m_pVlIKMFilt->M));
	if (!m_pVlIKMFilt->centers)
	{
		return FALSE;
	}

	vl_uint32 i, j;
	for (i = 0; i < m_pVlIKMFilt->K; i++) {  // center
		for (j = 0; j < m_pVlIKMFilt->M; j++) {
			int temp;
			if( fscanf( file, CLUSTER_READ_CONTROL, &temp ) != 1 ){
				return FALSE;
			}
			vl_uint32 iter = i * m_pVlIKMFilt->M + j;
			m_pVlIKMFilt->centers[iter] = (vl_ikm_data) temp;
		}
	}
	
	fclose(file);
	return TRUE;
}
