
#pragma once


//just for gray image -_-! i'm so lazzy

#ifndef _MY_SVIBE_H_
#define _MY_SVIBE_H_

#include <stdlib.h>
#include <stdint.h>

#define P_SAMPLES	35
#define P_MATCH		2
#define P_THRESHOLD 20
#define P_UPSPEED	16

#define DEFAULT_COLOR_BACKGROUND 0
#define DEFAULT_COLOR_SHADOW     80
#define DEFAULT_COLOR_FOREGROUND 255

typedef struct VibeModel
{
	uint8_t *bg_samples;
	int32_t width;
	int32_t height; 
    int32_t nChannel;

	int32_t nSample;      //number of samples in per pixel's background model
	int32_t nMatch;       //number of close samples for being part of the background
	int32_t threshold;    //radius of the sphere, the distance to compare current pixel with samples in its background model
	int32_t upSpeed;      //random of update speed

}VibeModel_t;

VibeModel_t * VibeModelInitC1(int32_t width, int32_t height, int32_t nChannel, uint8_t *image_data);
VibeModel_t * VibeModelInitC3(int32_t width, int32_t height, int32_t nChannel, uint8_t *image_data, uint8_t *greyImage);
void VibeModelUpdateC1(VibeModel_t *model, uint8_t *image, uint8_t *output);
void VibeModelUpdateC3(VibeModel_t *model, uint8_t *image, uint8_t *greyImage, uint8_t *output);
void VibeModelFree(VibeModel_t *model);

#endif

