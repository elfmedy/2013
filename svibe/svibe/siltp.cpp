
#include "stdafx.h"
#include "siltp.h"
#include "stdint.h"
#include "stdlib.h"
#include "string.h"
#include "helper.h"

static uint8_t *g_siltpTable;			//siltp feature table 
static uint8_t *g_siltpImage;			//current image's siltp feature
//static uint8_t *g_siltpFeaturesTable;	//current image's features table
static uint32_t *g_siltpDisTable;		//calc hanming distance's table
//static uint32_t *g_siltpValidTable;	//
//static uint32_t *g_siltpCalFeatureTable;//table to calc two feature hist's distance
static int32_t g_siltpWidth;			
static int32_t g_siltpHeight;
static int32_t g_siltpSamples;


void Siltp41C1Init(int32_t width, int32_t height)
{
	int32_t i = 0;

	g_siltpTable = (uint8_t *)malloc(width * height * SILTPSAMPLES * sizeof(uint8_t));
	g_siltpImage = (uint8_t *)malloc(width * height * sizeof(uint8_t));
	//g_siltpFeaturesTable = (uint8_t *)malloc(width * height * SILTPFEATURES * sizeof(uint8_t));
	g_siltpDisTable = (uint32_t *)malloc(256 * sizeof(uint32_t));
	//g_siltpValidTable = (uint32_t *)malloc(256 * sizeof(uint32_t));
	//g_siltpCalFeatureTable = (uint32_t *)malloc(256 * sizeof(uint32_t));
	g_siltpWidth = width;
	g_siltpHeight = height;
	g_siltpSamples = SILTPSAMPLES;

	//init hanming distance
	for (i = 0; i < 256; i++)
	{
		uint32_t value = i;
		int32_t count = 0;
		for (int32_t j = 0; j < 8; j++)
		{
			count += value & 0x01;
			value >>= 1;
		}
		g_siltpDisTable[i] = count;
	}
	//init table
	//for (i = 0; i < 256; i++)
	//{
	//	//g_siltpValidTable[i] = 0;
	//	g_siltpCalFeatureTable[i] = 0;
	//}
}

//calc 4-1 siltp
#define SILTP_CMP_INC(ptr) { value <<= 2; if(*ptr > th) value |= 0x01; else if(*ptr < tl) value |= 0x02; ptr++; }

uint8_t * Siltp41C1(uint8_t *pImg)
{
	uint8_t *pTop	 = pImg    + 1,
			*pCenter = pTop    + g_siltpWidth,
			*pRight  = pCenter + 1,
			*pLeft   = pCenter - 1,
			*pBottom = pCenter + g_siltpWidth;
	uint8_t *pSiltp  = g_siltpImage + g_siltpWidth + 1;

	for (int i = 0; i < g_siltpHeight - 2; i++)
	{
		for (int j = 0; j < g_siltpWidth - 2; j++)
		{
			double th = *pCenter * (1 + SILTPDELTA);
			double tl = *pCenter * (1 - SILTPDELTA);
			uint8_t value = 0;

			SILTP_CMP_INC(pTop);
			SILTP_CMP_INC(pRight);
			SILTP_CMP_INC(pBottom);
			SILTP_CMP_INC(pLeft);
			pCenter++;
			*pSiltp++ = value;
		}
		pTop    += 2;
		pRight  += 2;
		pBottom += 2;
		pLeft   += 2;
		pCenter += 2;
		pSiltp  += 2;
	}	
	return g_siltpImage;
}

uint8_t Siltp41C1GetImage(int32_t x, int32_t y)
{
	return g_siltpImage[g_siltpWidth * y + x];
}

//void SiltpFeatures3()
//{
//	int32_t arr[9];
//	int32_t x, y;
//	for (y = 1; y < g_siltpHeight - 1; y++)
//	{
//		for (x = 1; x < g_siltpWidth - 1; x++)
//		{
//			g_siltpFeaturesTable[y * g_siltpWidth * 9 + x * 9] = Siltp41C1GetImage(x, y);
//			g_siltpFeaturesTable[y * g_siltpWidth * 9 + x * 9 + 1] = Siltp41C1GetImage(x + 1, y);
//			g_siltpFeaturesTable[y * g_siltpWidth * 9 + x * 9 + 2] = Siltp41C1GetImage(x + 1, y - 1);
//			g_siltpFeaturesTable[y * g_siltpWidth * 9 + x * 9 + 3] = Siltp41C1GetImage(x , y - 1);
//			g_siltpFeaturesTable[y * g_siltpWidth * 9 + x * 9 + 4] = Siltp41C1GetImage(x - 1, y - 1);
//			g_siltpFeaturesTable[y * g_siltpWidth * 9 + x * 9 + 5] = Siltp41C1GetImage(x - 1, y);
//			g_siltpFeaturesTable[y * g_siltpWidth * 9 + x * 9 + 6] = Siltp41C1GetImage(x - 1, y + 1);
//			g_siltpFeaturesTable[y * g_siltpWidth * 9 + x * 9 + 7] = Siltp41C1GetImage(x , y + 1 );
//			g_siltpFeaturesTable[y * g_siltpWidth * 9 + x * 9 + 8] = Siltp41C1GetImage(x + 1, y + 1);
//		}
//	}
//}

uint8_t Siltp41C1GetModel(int32_t x, int32_t y, int32_t n)
{
	return g_siltpTable[g_siltpWidth * SILTPSAMPLES * y + SILTPSAMPLES * x + n];
}

void Siltp41C1SetModel(int32_t x, int32_t y, int32_t n, uint8_t val)
{
	g_siltpTable[g_siltpWidth * SILTPSAMPLES * y + SILTPSAMPLES * x + n] = val;
}

uint32_t Siltp41C1CalDistance(uint8_t s1, uint8_t s2)
{
	return g_siltpDisTable[s1 ^ s2];
}

//void Siltp41C1SetModel3w(int32_t x, int32_t y, int32_t n)
//{
//	int32_t i;
//	uint8_t *p1, *p2;
//	p1 = &(g_siltpFeaturesTable[y * g_siltpWidth * SILTPFEATURES + x * SILTPFEATURES]);
//	p2 = &(g_siltpTable[y * g_siltpWidth * SILTPSAMPLES * SILTPFEATURES + x * SILTPSAMPLES * SILTPFEATURES + n * SILTPFEATURES]);
//	for (i = 0; i < 9; i++)
//	{
//		*p1++ = *p2++;
//	}
//}


//uint32_t Siltp4C1CalDistance3w(int32_t x, int32_t y, int32_t n)
//{
//	int32_t i;
//	int32_t count = 0;
//	uint8_t *p1;
//	uint8_t feature;
//	int32_t xNG, yNG;
//
//	memset(g_siltpCalFeatureTable, 0, sizeof(int32_t) * 256);
//
//	p1 = &(g_siltpFeaturesTable[y * g_siltpWidth * SILTPFEATURES + x * SILTPFEATURES]);
//	for (i = 0; i < 9; i++)
//	{
//		feature = *(p1 + i);
//		++(g_siltpCalFeatureTable[feature]);
//	}
//	p1 = &(g_siltpTable[y * g_siltpWidth * SILTPSAMPLES * SILTPFEATURES + x * SILTPSAMPLES * SILTPFEATURES + n * SILTPFEATURES]);
//	for (i = 0; i < 9; i++)
//	{
//		feature = *(p1 + i);
//		if (g_siltpCalFeatureTable[feature] != 0)
//		{
//			count++;
//			g_siltpCalFeatureTable[feature] -= 1;
//		}
//	}
//	return (9 - count);
//}

//uint32_t Siltp4C1CalDistance2w(int32_t x, int32_t y, int32_t n)
//{
//	int32_t i;
//	int32_t count = 0;
//	x = x / 2 * 2;
//	y = y / 2 * 2;
//
//	memset(g_siltpCalFeatureTable, 0, sizeof(int32_t) * 256);
//
//	g_siltpCalFeatureTable[Siltp41C1GetModel(x,     y,     n)]++;
//	g_siltpCalFeatureTable[Siltp41C1GetModel(x + 1, y,     n)]++;
//	g_siltpCalFeatureTable[Siltp41C1GetModel(x,     y + 1, n)]++;
//	g_siltpCalFeatureTable[Siltp41C1GetModel(x + 1, y + 1, n)]++;
//
//	if (g_siltpCalFeatureTable[Siltp41C1GetImage(x,     y    )]-- > 0) count++;
//	if (g_siltpCalFeatureTable[Siltp41C1GetImage(x + 1, y    )]-- > 0) count++;
//	if (g_siltpCalFeatureTable[Siltp41C1GetImage(x,     y + 1)]-- > 0) count++;
//	if (g_siltpCalFeatureTable[Siltp41C1GetImage(x + 1, y + 1)]-- > 0) count++;
//
//	return (4 -count);
//}

//uint32_t IsSiltpValid(int32_t x, int32_t y)
//{
//	int32_t i;
//	uint32_t curNum;
//	uint32_t maxNum = 0;
//	uint8_t *p = &(g_siltpTable[g_siltpWidth * SILTPSAMPLES * y + SILTPSAMPLES * x + 0]);
//
//	memset(g_siltpValidTable, 0, sizeof(int32_t) * 256);
//	for (i = 0; i < SILTPSAMPLES; i++)
//	{
//		curNum = ++g_siltpValidTable[*p++];
//		if (curNum > maxNum) 
//		{
//			maxNum = curNum;
//		}
//	}
//	return maxNum;
//}

//uint8_t Siltp41C1
void  Siltp41C1Free()
{
	free(g_siltpTable);
	free(g_siltpImage);
	//free(g_siltpFeaturesTable);
	free(g_siltpDisTable);
	//free(g_siltpValidTable);
	//free(g_siltpCalFeatureTable);
}