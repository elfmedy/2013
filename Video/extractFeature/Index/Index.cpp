 // Index.cpp: implementation of the CIndex class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Index.h"
#include "math.h"
#include <iostream>

using namespace std;

#define FORWARDLIST_ALLOC	100

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CIndex::CIndex()
{
	m_pIndex = NULL;
	m_nIndex = 0;

	m_nTotalImages = 0;
	m_pForwardList = NULL;
	m_pQueryResultList = NULL;
    m_pQueryResultList_Public = NULL;
} 

CIndex::~CIndex()
{
	vl_index_free_item_array();
	if(m_pForwardList) free(m_pForwardList);
	if(m_pQueryResultList)
	{
		free(m_pQueryResultList);
		if (m_pQueryResultList == m_pQueryResultList_Public) m_pQueryResultList_Public = NULL;
	}
	if(m_pQueryResultList_Public) 
	{
		free(m_pQueryResultList_Public);
	}
}

//////////////////////////////////////////////////////////////////////
// create and free index item array
//////////////////////////////////////////////////////////////////////
vl_bool	CIndex::vl_index_create_new_item_array(vl_uint32 n)
{
	if ( n <= 0 ) {
		return FALSE;
	}

	if (m_pIndex) {
		vl_index_free_item_array();	
	}

	m_pIndex = (VlIndexItem*)malloc(sizeof(VlIndexItem) * n);
	memset(m_pIndex, 0, sizeof(VlIndexItem) * n);

	m_nIndex = n;

	return TRUE;
}

vl_bool	CIndex::vl_index_free_item_array()
{
	if (!m_pIndex) {
		m_nIndex = 0;
		return TRUE;
	}

	if (!m_nIndex) {
		return FALSE;
	}
	
	vl_uint32 i;
	for (i=0; i<m_nIndex; i++) {
		free(m_pIndex[i].pElements);
	}
	free(m_pIndex);

	m_pIndex = NULL;
	m_nIndex = 0;
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// create and free index item array
//////////////////////////////////////////////////////////////////////
static void encapsulate_index_element(vl_uint32 imageID, vl_uint32 nCountFeature, VlIndexElem* element)
{
		// encapsulate the information of an image to the element
	vl_uint32 encapsulate_data = 0;
	encapsulate_data += imageID << BITS_COUNT_FEATURE;	
	encapsulate_data += nCountFeature;						

	element->data1 = (vl_uint16)(encapsulate_data >> 16);//存储高16位
	element->data2 = (vl_uint16)	(encapsulate_data & 0x0000FFFF);//存储低16位
}

static void parse_index_element(vl_uint32& imageID, vl_uint32& nCountFeature, VlIndexElem* element)
{
		// parse the information of an element
	vl_uint32 encapsulate_data = 0;
	vl_uint32 data1, data2;
	data1 = element->data1;		data2 = element->data2;

	encapsulate_data += data1 << 16;
	encapsulate_data += data2;

	nCountFeature = encapsulate_data & 0x00001FFF;//截取低13位
	imageID = encapsulate_data >> BITS_COUNT_FEATURE;
}

vl_bool	CIndex::vl_index_push_one_image(vl_uint32* hist, vl_uint32 imageID)
{
	if (!hist) {
		return FALSE;
	}
	
	if (!m_pIndex) {
		return FALSE;
	}

	vl_uint32		i;
	VlIndexItem *	iter;	
	for (iter = m_pIndex, i = 0; i < m_nIndex; i++, iter++) {
		vl_uint32 nCountFeauture	= hist[i];
		if (!nCountFeauture) 
			continue;
			// the number of elements in this visual word increase
		iter->nElements++;
			// add to the end of the list
		iter->pElements = (VlIndexElem*)realloc(iter->pElements, sizeof(VlIndexElem) * iter->nElements);
		VlIndexElem *	elemNew = iter->pElements + iter->nElements - 1;
		
		encapsulate_index_element(imageID, nCountFeauture, elemNew);
	}

	return TRUE;
}

vl_bool CIndex::vl_forwardlist_push_one_image(const char* imageName)
{
	if (!imageName) {
		return FALSE;
	}

	if (!(m_nTotalImages%FORWARDLIST_ALLOC)) {
		m_pForwardList = (VlForwardList*)realloc(m_pForwardList, sizeof(VlForwardList) * (m_nTotalImages + FORWARDLIST_ALLOC));
	}
	VlForwardList* pForwardListNode = m_pForwardList + m_nTotalImages;
	strcpy(pForwardListNode->imageName, imageName);
	pForwardListNode->total_w = 0;
	
	//printf("%s : %lf \n", pForwardListNode->imageName, pForwardListNode->total_w);

	m_nTotalImages ++;
	
	return TRUE;
}

vl_uint32* CIndex::vl_index_assign_to_hist(vl_uint32* assign, vl_uint32 n)
{
	if (!assign) {
		return NULL;
	}

	if (!m_pIndex) {
		return NULL;
	}

	vl_uint32* count = (vl_uint32*)malloc(sizeof(vl_uint32) * m_nIndex);
	memset(count, 0, sizeof(vl_uint32) * m_nIndex);
	
	vl_uint32 i;
	for (i=0; i<n; i++) {
		vl_uint32 this_asgn = assign[i];
		++count[this_asgn];
	}

	return count;
}


/************************************************************************/
/* Calculation of the weight                                            */
/************************************************************************/
vl_bool CIndex::vl_index_calc_wi()
{
	if (m_nTotalImages <= 0) {
		return FALSE;
	}

	if (!m_pIndex) {
		return FALSE;
	}

	vl_uint32		i;
	VlIndexItem *	iter;
		// iterate all the visual words
	for (iter = m_pIndex, i = 0; i < m_nIndex; i++, iter++) {
			// weight = ln N/Ni
		iter->weightVisualWord = log(m_nTotalImages / (iter->nElements + VL_EPSILON_D));	//IDF
	}

	return TRUE;
}

vl_bool CIndex::vl_index_calc_total_wi()
{
	if (!m_pForwardList) {
		return FALSE;
	}
	
	if (!m_pIndex) {
		return FALSE;
	}

	vl_uint32		i;
	VlIndexItem*	iter;
		// iterate all the visual words
	for (iter = m_pIndex, i = 0; i < m_nIndex; i++, iter++) {
		vl_uint32		j;
		VlIndexElem *	flag;
			// iterate all the elements in each visual word
		for(flag = iter->pElements, j = 0; j < iter->nElements; j++, flag++){
			vl_uint32 imageID, nCountFeature;
			parse_index_element(imageID, nCountFeature, flag);
			
			VlForwardList * pForwardNode = m_pForwardList + imageID;
			pForwardNode->total_w += nCountFeature * iter->weightVisualWord; 
		}
	}
	
	return TRUE;
}

/************************************************************************/
/*		Query					                                        */
/************************************************************************/
vl_bool	CIndex::vl_index_query(vl_uint32* hist)
{
	if (!hist) {
		return FALSE;
	}
	
	if (!m_pIndex) {
		return FALSE;
	}

	if (!m_pForwardList) {
		return FALSE;
	}
		// free the last query result
	if (m_pQueryResultList) {
		free(m_pQueryResultList);
		m_pQueryResultList = NULL;
	}
		// reallocate the memory for query result list
	m_pQueryResultList = (VlQueryResultList*)malloc(sizeof(VlQueryResultList) * m_nTotalImages);
	memset(m_pQueryResultList, 0, sizeof(VlQueryResultList) * m_nTotalImages);

	vl_uint32	i;
	VlIndexItem *	iter;	
		// iterate all the assigned visual words
	for (iter = m_pIndex, i = 0; i < m_nIndex; i++, iter++) {
		vl_uint32 nCountFeautureQ	= hist[i];
		if(!nCountFeautureQ)
			continue;		

		vl_uint32		j;
		VlIndexElem *	flag;
			// iterate all the elements in each visual word
		for (flag = iter->pElements, j = 0; j < iter->nElements; j++, flag++) {
			vl_uint32 imageID, nCountFeatureD;
			parse_index_element(imageID, nCountFeatureD, flag);
			
			VlForwardList * pForwardNode = m_pForwardList + imageID;
			double			total_weight = pForwardNode->total_w;
				// calculate the score
			VlQueryResultList * pQueryNode = m_pQueryResultList + imageID;
			pQueryNode->num_cooccurrence_words += 1;
			pQueryNode->total_idf += iter->weightVisualWord;
			//(nCountFeatureD * nCountFeautureQ * iter->weightVisualWord * iter->weightVisualWord);
			//nCountFeatureD * nCountFeautureQ;
			//(nCountFeatureD * nCountFeautureQ * iter->weightVisualWord)/(sqrt(total_weight) + VL_EPSILON_D);
			//(nCountFeatureD * nCountFeautureQ * iter->weightVisualWord * iter->weightVisualWord)/(total_weight + VL_EPSILON_D);
		}
		
	}
    m_pQueryResultList_Public = m_pQueryResultList;
	return TRUE;
}


VlQueryResultList* CIndex::vl_index_query_pick_first(vl_uint32& imageID)
{

   
	double best_score = -1;
	VlQueryResultList * bestQuery = NULL;
	vl_uint32 flag = 0;
	vl_uint32 ID = 0;

	while(flag < m_nTotalImages){
		if (!m_pQueryResultList[flag].isPicked && m_pQueryResultList[flag].total_idf > best_score) {
			best_score = m_pQueryResultList[flag].total_idf;
			ID	= flag;
			bestQuery = m_pQueryResultList + flag;
		}
		flag ++;
	}
	bestQuery->isPicked = 1;

	imageID = ID;
	return bestQuery;
}

char * CIndex::vl_index_get_image_name(vl_uint32 imageID)
{
	if (!m_pForwardList) {
		return NULL;
	}
	VlForwardList* iter = m_pForwardList + imageID;
	return iter->imageName;
}

/************************************************************************/
/*		Write					                                        */
/************************************************************************/
vl_bool	CIndex::vl_index_write(const char* index_file_name)
{
	FILE * file = fopen(index_file_name, "wb");
	if (!file) {
		return FALSE;
	}

	if (!m_pIndex) {
		return FALSE;
	}

	if(fprintf(file, "%u\n", m_nIndex) < 0){
		return FALSE;
	}

	vl_uint32		i;
	VlIndexItem*	iter;
	for (iter = m_pIndex, i = 0; i < m_nIndex; i++, iter++) {
		if(fprintf(file, "%u %u %lf\n", i, iter->nElements, iter->weightVisualWord) < 0){
			return FALSE;
		}
		if (iter->nElements){
			//fwrite(iter->pElements, sizeof(VlIndexElem), iter->nElements, file);
			vl_uint32 j;
			for (j=0; j<iter->nElements; j++){
				vl_uint32 imageID, nCountFeatureD;
				parse_index_element(imageID, nCountFeatureD, iter->pElements + j);
				if(fprintf(file, "%u %u\n", imageID, nCountFeatureD) < 0){
					return FALSE;
				}
			}
		}
	}

	fclose(file);
	return TRUE;
}

vl_bool CIndex::vl_index_read(const char* index_file_name)
{
	FILE * file = fopen(index_file_name, "rb");
	if (!file) {
		return FALSE;
	}

	if(fscanf(file, "%u\n", &m_nIndex) != 1 ){
		return FALSE;
	}
	
	vl_index_create_new_item_array(m_nIndex);

	vl_uint32		i;
	VlIndexItem*	iter;
	for (iter = m_pIndex, i = 0; i < m_nIndex; i++, iter++) {
		vl_uint32 temp = 0, temp1;
		if( fscanf(file, "%u %u %lf\n", &temp, &temp1, &(iter->weightVisualWord)) != 3){
			return FALSE;
		}
		if (temp != i){
			return FALSE;
		}
		iter->nElements = (vl_uint16) temp1;
		if (iter->nElements){
			iter->pElements = (VlIndexElem*)malloc( sizeof(VlIndexElem) * iter->nElements);
			//fread(iter->pElements, sizeof(VlIndexElem), iter->nElements, file);

			vl_uint32 j;
			for (j=0; j<iter->nElements; j++){
				vl_uint32 imageID, nCountFeatureD;
				if(fscanf(file, "%u %u\n", &imageID, &nCountFeatureD) != 2){
					return FALSE;
				}
				encapsulate_index_element(imageID, nCountFeatureD, iter->pElements + j);
			}			
		}
	}
	return TRUE;
}

vl_bool CIndex::vl_forwardlist_write(const char* forwardlist_file_name)
{
	FILE * file = fopen(forwardlist_file_name, "wb");
	if (!file) {
		return FALSE;
	}

	if(fprintf(file, "%u\n", m_nTotalImages) < 0 ){
		return FALSE;
	}

	vl_uint32		i;
	VlForwardList*	iter;
	for (iter = m_pForwardList, i = 0; i < m_nTotalImages; i++, iter++) {

		//cout<<i<<" "<<iter->imageName<<" "<<iter->total_w<<endl;
		if(fprintf(file, "%s %lf\n", iter->imageName, iter->total_w) < 0){
			return FALSE;
		}
	}

	fclose(file);
	return TRUE;
}

vl_bool CIndex::vl_forwardlist_read(const char* forwardlist_file_name)
{
	FILE * file = fopen(forwardlist_file_name, "rb");
	if (!file) {
		return FALSE;
	}

	if(fscanf(file, "%u\n", &m_nTotalImages) != 1){
		return FALSE;
	}

	if (m_pForwardList) {
		free(m_pForwardList);
		m_pForwardList = NULL;
	}
	
	m_pForwardList = (VlForwardList*)malloc(sizeof(VlForwardList) * m_nTotalImages);
	memset(m_pForwardList, 0, sizeof(VlForwardList) * m_nTotalImages);

	vl_uint32		i;
	VlForwardList*	iter;
	for (iter = m_pForwardList, i = 0; i < m_nTotalImages; i++, iter++) {
		if(fscanf(file, "%s %lf\n", iter->imageName, &(iter->total_w)) != 2){
			return FALSE;
		}
	}

	return TRUE;
}