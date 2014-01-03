// Index.h: interface for the CIndex class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_INDEX_H__AA305216_AAA7_4AA6_B01D_570CF69CF752__INCLUDED_)
#define AFX_INDEX_H__AA305216_AAA7_4AA6_B01D_570CF69CF752__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../atom.h"

	// number of bits for image id
#define BITS_IAMGE_ID		19
	// number of bits for counting the number of the features in one visual word
#define	BITS_COUNT_FEATURE	13
	// lenght of the image name
#define VL_MAX_IMAGE_NAME 100

	// Forward List
typedef struct _VlForwardList
{
	char		     imageName[VL_MAX_IMAGE_NAME];
	double			 total_w;
}VlForwardList;

	// Query Result List
typedef struct _VlQueryResultList
{
	double			total_idf;
	int             num_cooccurrence_words;
	vl_uint8		isPicked;
}VlQueryResultList;

	// the element storing the information of image
typedef struct _VlIndexElem
{
	vl_uint16		data1;			
	vl_uint16		data2;
}VlIndexElem;

	// the item storing the information of visual word
typedef struct _VlIndexItem
{
	vl_uint32		nElements;			// the number of images, which contain the corresponding visual word
	VlIndexElem *	pElements;			// the point to the Elements, which record the information about the images
	double			weightVisualWord;	// the tf-idf weight of the visual word
}VlIndexItem;

class CIndex  
{
public:
	CIndex();
	virtual ~CIndex();
	// operation --- utility
public:
	vl_uint32* vl_index_assign_to_hist(vl_uint32* assign, vl_uint32 n);
	inline vl_uint32 vl_index_get_nImage(){ return m_nTotalImages; }
	inline vl_uint32 vl_index_get_nIndex(){ return m_nIndex; }
	// operation --- index
public:
	vl_bool	vl_index_create_new_item_array(vl_uint32 n);
	vl_bool	vl_index_free_item_array();
	vl_bool	vl_index_push_one_image(vl_uint32* hist, vl_uint32 imageID);
	vl_bool vl_index_calc_wi();

	// operation --- forward list
public:
	vl_bool vl_forwardlist_push_one_image(const char* imageName);
	vl_bool vl_index_calc_total_wi();
	char *	vl_index_get_image_name(vl_uint32 imageID);

	// operation --- query
public:
	vl_bool				vl_index_query(vl_uint32* hist);
	VlQueryResultList*	vl_index_query_pick_first(vl_uint32& imageID);
	
	// operation --- read and write
public:
	vl_bool	vl_index_write(const char* index_file_name);
	vl_bool vl_forwardlist_write(const char* forwardlist_file_name);
	vl_bool vl_index_read(const char* index_file_name);
	vl_bool vl_forwardlist_read(const char* forwardlist_file_name);

private:
	VlIndexItem *	m_pIndex;
	vl_uint32		m_nIndex;

	vl_uint32		m_nTotalImages;
	VlForwardList*	m_pForwardList;
	VlQueryResultList* m_pQueryResultList;

public:
    VlQueryResultList* m_pQueryResultList_Public;
};

#endif // !defined(AFX_INDEX_H__AA305216_AAA7_4AA6_B01D_570CF69CF752__INCLUDED_)
