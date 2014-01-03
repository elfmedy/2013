
#pragma once

#ifndef _SILTP_H_
#define _SILTP_H_

#include <stdint.h>

#define SILTPSAMPLES         100		//siltp feature samples //35

#define SILTPDISTHRESHOLD    3			//distance threshold			
#define SILTPCOUNTTHRESHOLD  8			//#min

#define SILTPDELTA			 0.05

#define SILTPUPDATESPEEDHIGH 1			//2
#define SILTPUPDATESPEEDNOR  1			//4
#define SILTPUPDATESPEEDLOW  1			//16

void Siltp41C1Init(int32_t width, int32_t height);
uint8_t * Siltp41C1(uint8_t *pImg);
//void SiltpFeatures3();
uint8_t Siltp41C1GetImage(int32_t x, int32_t y);
uint8_t Siltp41C1GetModel(int32_t x, int32_t y, int32_t n);
void Siltp41C1SetModel(int32_t x, int32_t y, int32_t n, uint8_t val);
//uint32_t Siltp4C1CalDistance2w(int32_t x, int32_t y, int32_t n);
uint32_t Siltp41C1CalDistance(uint8_t s1, uint8_t s2);
//void Siltp41C1SetModel3w(int32_t x, int32_t y, int32_t n);
//uint32_t Siltp4C1CalDistance3w(int32_t x, int32_t y, int32_t n);
//uint32_t IsSiltpValid(int32_t x, int32_t y);
void Siltp41C1Free();

#endif