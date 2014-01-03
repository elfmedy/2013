
#include "stdafx.h"
#include "helper.h"
#include <stdlib.h>

static uint32_t *g_randTable;

void RandInit()
{
	static int32_t flag = 0;
	if (flag == 0)
	{
		flag = 1;
		int32_t i = 0;
		g_randTable = (uint32_t *)malloc(0x10000 * sizeof(uint32_t));
		for (i = 0; i < 0x10000; i++)
		{
			g_randTable[i] = (uint32_t)(rand());
		}		
	}
}

uint32_t GetRand()
{
	static uint32_t i = 0;
	return g_randTable[0xFFFF & i++];
}

void FreeRand()
{
	free(g_randTable);
}

static int32_t *g_XTable;
static int32_t *g_YTable;
static int32_t g_XNeighborTable[8] = {2, 2, 1, 0, 0, 0, 1, 2};
static int32_t g_YNeighborTable[8] = {1, 0, 0, 0, 1, 2, 2, 2};

void RandNeighborInit(int32_t width, int32_t height)
{
	static int32_t flag = 0;
	int32_t i;
	if (flag == 0)
	{
		flag = 1;
		g_XTable = (int32_t *)malloc((width + 2) * sizeof(int32_t));
		for (i = 0; i < width; i++)
		{
			g_XTable[i + 1] = i;
		}
		g_XTable[0] = 0;
		g_XTable[width + 1] = width - 1;

		g_YTable = (int32_t *)malloc((height + 2) * sizeof(int32_t));
		for (i = 0; i < height; i++)
		{
			g_YTable[i + 1] = i;
		}
		g_XTable[0] = 0;
		g_XTable[height + 1] = height - 1;
	}
}

void GetRandNeighborXY(int32_t x, int32_t y, int32_t *_x, int32_t *_y)
{
	uint32_t r = GetRand() & 0x07;
	*_x = g_XTable[x + g_XNeighborTable[r]];
	*_y = g_XTable[y + g_YNeighborTable[r]];
}

void GetNeighborXY(int32_t x, int32_t y, int32_t *_x, int32_t *_y, int32_t flag)
{
	*_x = g_XTable[x + g_XNeighborTable[flag]];
	*_y = g_XTable[y + g_YNeighborTable[flag]];
}

void GetNeighborTL(int32_t x, int32_t y, int32_t *_x, int32_t *_y) 
{
	GetNeighborXY(x, y, _x, _y, 3);
}

void GetNeighborTC(int32_t x, int32_t y, int32_t *_x, int32_t *_y) 
{
	GetNeighborXY(x, y, _x, _y, 2);
}

void GetNeighborTR(int32_t x, int32_t y, int32_t *_x, int32_t *_y) 
{
	GetNeighborXY(x, y, _x, _y, 1);
}

void GetNeighborML(int32_t x, int32_t y, int32_t *_x, int32_t *_y) 
{
	GetNeighborXY(x, y, _x, _y, 4);
}

void GetNeighborMR(int32_t x, int32_t y, int32_t *_x, int32_t *_y) 
{
	GetNeighborXY(x, y, _x, _y, 0);
}

void GetNeighborBL(int32_t x, int32_t y, int32_t *_x, int32_t *_y) 
{
	GetNeighborXY(x, y, _x, _y, 5);
}

void GetNeighborBC(int32_t x, int32_t y, int32_t *_x, int32_t *_y) 
{
	GetNeighborXY(x, y, _x, _y, 6);
}

void GetNeighborBR(int32_t x, int32_t y, int32_t *_x, int32_t *_y) 
{
	GetNeighborXY(x, y, _x, _y, 7);
}

void FreeRandNeighbor()
{
	free(g_XTable);
	free(g_YTable);
}
