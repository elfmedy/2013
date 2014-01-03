
#pragma once

#ifndef _HELPER_H_
#define _HELPER_H_

#include <stdint.h>

//random num
void RandInit();
uint32_t GetRand();
void FreeRand();

//random neighbor
void RandNeighborInit(int32_t width, int32_t height);
void GetRandNeighborXY(int32_t x, int32_t y, int32_t *_x, int32_t *_y);

//get neighbor
void GetNeighborXY(int32_t x, int32_t y, int32_t *_x, int32_t *_y, int32_t flag);
void GetNeighborTL(int32_t x, int32_t y, int32_t *_x, int32_t *_y);
void GetNeighborTC(int32_t x, int32_t y, int32_t *_x, int32_t *_y);
void GetNeighborTR(int32_t x, int32_t y, int32_t *_x, int32_t *_y);
void GetNeighborML(int32_t x, int32_t y, int32_t *_x, int32_t *_y);
void GetNeighborMR(int32_t x, int32_t y, int32_t *_x, int32_t *_y);
void GetNeighborBL(int32_t x, int32_t y, int32_t *_x, int32_t *_y);
void GetNeighborBC(int32_t x, int32_t y, int32_t *_x, int32_t *_y);
void GetNeighborBR(int32_t x, int32_t y, int32_t *_x, int32_t *_y);

void FreeRandNeighbor();

//img to model

#endif
