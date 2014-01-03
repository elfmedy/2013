/***************************************************************
 * Name:      FexWrite.cpp
 * Purpose:   Code for : extract and write the descriptors according to the input parameters
 * Author:    Muhammet Bastan (bastan@cs.bilkent.edu.tr)
 * Created:   16.06.2009
 * Copyright: Muhammet Bastan (http://www.cs.bilkent.edu.tr/~bastan)
 * License:
 **************************************************************/

#include <iostream>

//#include "cxcore.h"
//#include "cv.h"


#include "FexWrite.h"
#include <fstream>

// Color Structure
void FexWrite::computeWriteCSD( Frame* frame, int descSize )
{
	if(!frame)
		return;

    // compute the descriptor
	XM::ColorStructureDescriptor* csd = Feature::getColorStructureD(frame, descSize);

	// write to screen
	for(unsigned int i = 0; i < csd->GetSize(); i++)
		std::cout << (int)csd->GetElement(i) << " " ;
	std::cout << std::endl;

    // release descriptor
	delete csd;
}


// Scalable Color
void FexWrite::computeWriteSCD( Frame* frame, bool maskFlag, int descSize )
{
	if(!frame)
		return;

    // compute descriptor
	XM::ScalableColorDescriptor* scd = Feature::getScalableColorD( frame, maskFlag, descSize );

	// write to screen
	for(unsigned int i = 0; i < scd->GetNumberOfCoefficients(); i++)
		std::cout << (int)  scd->GetCoefficient(i) << " ";
	std::cout << std::endl;

    // release descriptor
	delete scd;
}


// Color Layout
void FexWrite::computeWriteCLD( Frame* frame, int numYCoef, int numCCoef )
{
	if(!frame)
		return;

    // compute the descriptor
	XM::ColorLayoutDescriptor* cld = Feature::getColorLayoutD( frame, numYCoef, numCCoef );

	// write to screen

	// number of coefficients
	int numberOfYCoeff = cld->GetNumberOfYCoeff();
	int numberOfCCoeff = cld->GetNumberOfCCoeff();


	// values
	int *y_coeff, *cb_coeff, *cr_coeff;
	y_coeff = cld->GetYCoeff();
	cb_coeff = cld->GetCbCoeff();
	cr_coeff = cld->GetCrCoeff();


	int i = 0;
	// Y coeff (DC and AC)
	for ( i = 0; i < numberOfYCoeff; i++ )
		std::cout << y_coeff[i] << " " ;

	//Cb coeff  (DC and AC)
	for ( i = 0; i < numberOfCCoeff; i++ )
		std::cout << cb_coeff[i] << " ";

	//Cr coeff  (DC and AC)
	for ( i = 0; i < numberOfCCoeff; i++ )
		std::cout << cr_coeff[i] << " ";

	std::cout << std::endl;

    // release the descriptor
	delete cld;
}


void insertion_sort(int arr[], unsigned int arr2[], int first, int last)
 {
	int i,j;
	int temp, temp2;
	for (i = first + 1; i <= last; i++)
	{
		temp = arr[i];
		temp2 = arr2[i];
		j = i - 1;
 
		while((j >= first) && (arr[j] < temp)) 
		{
			arr[j+1] = arr[j];
			arr2[j+1] = arr2[j];
			j--;
		}
		arr[j+1] = temp; 
		arr2[j+1] = temp2;
	}
}

// Dominant Color
//void FexWrite::computeWriteDCD( Frame* frame,
//                                bool normalize, bool variance, bool spatial,
//                                int bin1, int bin2, int bin3 )
unsigned int FexWrite::computeWriteDCD( Frame* frame, int num, unsigned int *arr, 
                                bool normalize, bool variance, bool spatial,
                                int bin1, int bin2, int bin3 )
{
	if(!frame)
		return 0;

    // compute the descriptor
	XM::DominantColorDescriptor* dcd = Feature::getDominantColorD( frame, normalize, variance, spatial, bin1, bin2, bin3 );


	// write to screen

	// number of dominant colors
	int ndc = dcd->GetDominantColorsNumber();
	//std::cout << "num of dominant colors " << ndc << " " << std::endl;	//my add

	// spatial coherency
	if(spatial)
        std::cout << dcd->GetSpatialCoherency();

	// dominant colors: percentage(1) centroid value (3) color variance (3)
	XM::DOMCOL* domcol = dcd->GetDominantColors();
	
	/*
	for( int i = 0; i < ndc; i++ )
	{
		std::cout << " " << domcol[i].m_Percentage
                  << " " << domcol[i].m_ColorValue[0]
                  << " " << domcol[i].m_ColorValue[1]
                  << " " << domcol[i].m_ColorValue[2]
				  << std::endl;
        if(variance)
        std::cout << " " << domcol[i].m_ColorVariance[0]
                  << " " << domcol[i].m_ColorVariance[1]
                  << " " << domcol[i].m_ColorVariance[2];
	}
	std::cout << std::endl;
	*/
	
	using namespace std;
	char tmpStr[30];
	sprintf(tmpStr, "feature\\%d_1.txt", num);
	ofstream f(tmpStr);
	int numArr[8];
	for( int i = 0; i < ndc; i++ )
	{
		f	<< " " << domcol[i].m_Percentage
			<< " " << domcol[i].m_ColorValue[0]
			<< " " << domcol[i].m_ColorValue[1]
			<< " " << domcol[i].m_ColorValue[2];
			//<< std::endl;
			numArr[i] = domcol[i].m_Percentage;
			unsigned int a1 = (unsigned int)(domcol[i].m_ColorValue[0]) << 10;
			unsigned int a2 = (unsigned int)(domcol[i].m_ColorValue[1]) << 5;
			unsigned int a3 = (unsigned int)(domcol[i].m_ColorValue[2]);
			arr[i] = a1 | a2 | a3;

        if(variance)
        f	<< " " << domcol[i].m_ColorVariance[0]
            << " " << domcol[i].m_ColorVariance[1]
            << " " << domcol[i].m_ColorVariance[2];
	}
	//f << std::endl;
	f.close();

	insertion_sort(numArr, arr, 0, ndc - 1);

    // release the descriptor
	delete dcd;

	//return 0;
	return ndc;
}

// Homogeneous Texture
void FexWrite::computeWriteHTD( Frame* frame, int layerFlag )
{
	if(!frame)
		return;

	XM::HomogeneousTextureDescriptor* htd = Feature::getHomogeneousTextureD( frame, layerFlag );

    // get a pointer to the values
	int* values = htd->GetHomogeneousTextureFeature();

	// write to screen

	// values[0]: mean, values[1]: std, values[2-31] base layer (energy)
	int i;
	for(i = 0; i < 32; i++)
		std::cout << values[i] << " " ;

    // if full layer, print values[32-61] (energy deviation)
    if(layerFlag)
        for(i = 32; i < 62; i++)
            std::cout << values[i] << " " ;

	std::cout << std::endl;

    // release the descriptor
	delete htd;
}


// Edge Histogram
//void FexWrite::computeWriteEHD( Frame* frame )
unsigned int FexWrite::computeWriteEHD( Frame* frame, int num)
{
	if(!frame)
		return 0;

    // compute the descriptor
	XM::EdgeHistogramDescriptor* ehd = Feature::getEdgeHistogramD( frame );

    // get a pointer to the values
	char* de = ehd->GetEdgeHistogramElement();

	//std::cout << "Edge Histogram size " << ehd->GetSize() << std::endl;

	// write to screen
	/*
	for( unsigned int i = 0; i < ehd->GetSize(); i++)
		std::cout << (int)de[i] << " ";
	std::cout  << std::endl;
	*/

	using namespace std;
	char tmpStr[30];
	sprintf(tmpStr, "feature\\%d_2.txt", num);
	ofstream f(tmpStr);
	for( unsigned int i = 0; i < ehd->GetSize(); i++)
	{
		f << " " << (int)(de[i]);
	}
	f.close();

	int startArr[4] = {0, 10, 40, 50};
	int arr[20];
	unsigned int re  = 0;
	
	int k = 0;
	for (int i = 0; i < 4; i++)
	{
		for (int j = startArr[i]; j < startArr[i] + 5; j++)
		{
			arr[k] = (int)(de[j]) + (int)(de[j+5]) + (int)(de[j+20]) + (int)(de[j+25]);
			re <<= 1;
			if (arr[k] >= 14)
				re |= 0x00000000000000000001;
			k++;
		}
	}
    // release the descriptor
	delete ehd;

	return re;
}


// Region Shape
void FexWrite::computeWriteRSD( Frame* frame )
{
    if(!frame)
		return;

    // compute the descriptor
    XM::RegionShapeDescriptor* rsd = Feature::getRegionShapeD( frame );

    int i,j;
	for(i=0; i<ART_ANGULAR; i++)
		for(j=0; j<ART_RADIAL; j++)
			if(i != 0 || j != 0)
				std::cout << (int)rsd->GetElement(i, j) << " ";

	std::cout << std::endl;

    // release the descriptor
    delete rsd;
}

// Contour Shape
void FexWrite::computeWriteCShD( Frame* frame )
{
    if(!frame)
		return;

}
