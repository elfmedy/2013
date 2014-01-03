// svibe.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "helper.h"
#include "svibe.h"
#include "stdlib.h"
#include "math.h"
#include "siltp.h"
#include "string.h"
#include "cv.h"
#include "highgui.h"

using namespace cv;

int32_t g_frameCount = 0;

VibeModel_t * VibeModelInitC1(int32_t width, int32_t height, int32_t nChannel, uint8_t *image_data)
{
    int32_t x, y, xNG, yNG, index, i;
    //int32_t image_offset = 0, sample_offset = 0;
	uint8_t siltpValue;
	uint32_t rand;

	RandInit();
	RandNeighborInit(width, height);
	Siltp41C1Init(width, height);

    VibeModel_t *vbM = (VibeModel_t *)malloc(sizeof(VibeModel_t)); 
    vbM->width		 = width;
    vbM->height		 = height;
    vbM->nChannel	 = nChannel;
    vbM->nSample	 = P_SAMPLES;
    vbM->threshold	 = P_THRESHOLD;
    vbM->nMatch		 = P_MATCH;
    vbM->upSpeed	 = P_UPSPEED;
	vbM->bg_samples  = (uint8_t *)malloc(vbM->width * vbM->height * vbM->nChannel * vbM->nSample);

	Siltp41C1(image_data);
	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			for (index = 0; index < vbM->nSample; index++) {
				GetRandNeighborXY(x, y, &xNG, &yNG);
				vbM->bg_samples[y * vbM->width * vbM->nSample + x * vbM->nSample + index] = image_data[yNG * vbM->width + xNG];
			}
			//--siltp (gauss)
			siltpValue = Siltp41C1GetImage(x, y);
			for (i = 0; i < SILTPSAMPLES / 3; i++)
			{
				Siltp41C1SetModel(x, y, i, siltpValue);
			}
			for (i = SILTPSAMPLES - SILTPSAMPLES / 3; i < SILTPSAMPLES; i++)
			{
				rand = GetRand() % 8;
				uint8_t newSiltp = siltpValue ^ (0x01 << rand);
				Siltp41C1SetModel(x, y, i, newSiltp);
			}
		}
	}
    
    return vbM;
}


VibeModel_t * VibeModelInitC3(int32_t width, int32_t height, int32_t nChannel, uint8_t *image_data, uint8_t *greyImage)
{
    int32_t x, y, xNG, yNG, index, i;
    int32_t image_offset = 0, sample_offset = 0;
	uint8_t siltpValue;
	uint32_t rand;

	RandInit();
	RandNeighborInit(width, height);
	Siltp41C1Init(width, height);

    VibeModel_t *vbM = (VibeModel_t *)malloc(sizeof(VibeModel_t)); 
    vbM->width		 = width;
    vbM->height		 = height;
    vbM->nChannel	 = nChannel;
    vbM->nSample	 = P_SAMPLES;
    vbM->threshold	 = P_THRESHOLD;
    vbM->nMatch		 = P_MATCH;
    vbM->upSpeed	 = P_UPSPEED;
	vbM->bg_samples  = (uint8_t *)malloc(vbM->width * vbM->height * vbM->nChannel * vbM->nSample);

	Siltp41C1(greyImage);
	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			//vibe
			for (index = 0; index < vbM->nSample; index++) {
				GetRandNeighborXY(x, y, &xNG, &yNG);
				sample_offset = (y * vbM->width * vbM->nSample + x * vbM->nSample + index) * 3;
				image_offset  = (yNG * vbM->width + xNG) * 3;
				vbM->bg_samples[sample_offset]     = image_data[image_offset];
				vbM->bg_samples[sample_offset + 1] = image_data[image_offset + 1];
				vbM->bg_samples[sample_offset + 2] = image_data[image_offset + 2];
			}
			//--siltp (gauss)
			siltpValue = Siltp41C1GetImage(x, y);
			for (i = 0; i < SILTPSAMPLES / 3; i++)
			{
				Siltp41C1SetModel(x, y, i, siltpValue);
			}
			for (i = SILTPSAMPLES - SILTPSAMPLES / 3; i < SILTPSAMPLES; i++)
			{
				rand = GetRand() % 8;
				uint8_t newSiltp = siltpValue ^ (0x01 << rand);
				Siltp41C1SetModel(x, y, i, newSiltp);
			}
		}
	}
    
	g_frameCount++;

    return vbM;
}


void VibeModelUpdateC1(VibeModel_t *model, uint8_t *image, uint8_t *output)
{
	int32_t  x, y, xNG, yNG;
	int32_t count, index, dist, count2, index2, dist2;
	int32_t sampleOffset, imageOffset;
	uint32_t rand;
	//int32_t arr8[8] = {-1, -1, 0, -1, -1, 0, 0, 0};
	uint8_t backPoint;

	//siltp
	Siltp41C1(image);
    for (y = 0; y < model->height; y++) 
	{
        for (x = 0; x < model->width; x++) 
		{
			sampleOffset = y * model->width * model->nSample + x * model->nSample;
			imageOffset  = y * model->width + x;
            //1. compare pixel to background model
            count = 0; index = 0;
            while ((count < model->nMatch) && (index < model->nSample)) 
			{
                //Euclidean distance computation
				dist = abs(image[imageOffset] - model->bg_samples[sampleOffset + index]);
				if (dist < model->threshold) 
				{
					backPoint =  model->bg_samples[sampleOffset + index];
					count++;
				}
            }
                index++;
			//siltp
			count2 = 0; index2 = 0;
			while ((count2 < SILTPCOUNTTHRESHOLD) && (index2 < SILTPSAMPLES)) 
			{
				dist2 = Siltp41C1CalDistance(Siltp41C1GetImage(x, y), Siltp41C1GetModel(x, y, index2));
				if (dist2 < SILTPDISTHRESHOLD) 
				{ 
					count2++;
				}
				index2++;
			}

			// classify pixel and update model
			// siltp & point update, if both background -> background
			//						 if point is background, siltp is forground -> check if siltp is valid
			//						 if point is forground, siltp is backround  -> shadow
			//						 if both forground, forground -> foreground
			if (count < model->nMatch && count2 < SILTPCOUNTTHRESHOLD)
			{
				output[imageOffset] = DEFAULT_COLOR_FOREGROUND;
				//update siltp back model(slowly update since it's fore)
				rand = GetRand() % SILTPUPDATESPEEDLOW;
				if (rand == 0)
				{
					rand = GetRand() % SILTPSAMPLES;
					Siltp41C1SetModel(x, y, rand, Siltp41C1GetImage(x, y));
				}
			}
			else
			{
				//shadow (vibe classify as forground & siltp classify as background)
				if (count < model->nMatch) 
				{
					if (image[imageOffset] >= model->bg_samples[sampleOffset + 0])
						output[imageOffset] = DEFAULT_COLOR_FOREGROUND;
					else output[imageOffset] = DEFAULT_COLOR_SHADOW;
					////update current pixel model
					//rand = GetRand() % (model->upSpeed * 3);
					//if (rand == 0) 
					//{
					////	//replace randomly chosen sample
					//	rand = GetRand() % (model->nSample);
					//	model->bg_samples[sampleOffset + rand] = image[imageOffset];  
					//}
					////update neighboring pixel model
					//rand = GetRand() % (model->upSpeed * 3);
					//if (rand == 0) 
					//{
					//	GetRandNeighborXY(x, y, &xNG, &yNG);
					//	rand = GetRand() % (model->nSample);
					//	model->bg_samples[yNG * model->width * model->nSample + xNG * model->nSample + rand] = image[imageOffset];
					//}
					//update siltp back model(quickly update since it's back)
					rand = GetRand() % SILTPUPDATESPEEDNOR;
					if (rand == 0)
					{
						rand = GetRand() % SILTPSAMPLES;
						Siltp41C1SetModel(x, y, rand, Siltp41C1GetImage(x, y));
					}
				}
				//siltp judged as forground, vibe judged as background( since this area is high dynamic)
				else if (count2 < SILTPCOUNTTHRESHOLD)
				{
					output[imageOffset] = DEFAULT_COLOR_BACKGROUND;

					//update current pixel model
					rand = GetRand() % (model->upSpeed);
					if (rand == 0) 
					{
						rand = GetRand() % (model->nSample);
						model->bg_samples[sampleOffset + rand] = image[imageOffset];  
					}
					//update neighboring pixel model
					rand = GetRand() % (model->upSpeed);
					if (rand == 0) 
					{
						GetRandNeighborXY(x, y, &xNG, &yNG);
						rand = GetRand() % (model->nSample);
						model->bg_samples[yNG * model->width * model->nSample + xNG * model->nSample + rand] = image[imageOffset];
					}
					//siltp
					rand = GetRand() % SILTPUPDATESPEEDHIGH;
					if (rand == 0)
					{
						rand = GetRand() % SILTPSAMPLES;
						Siltp41C1SetModel(x, y, rand, Siltp41C1GetImage(x, y));
					}
				}
				//both judged as background
				else 
				{
					output[imageOffset] = DEFAULT_COLOR_BACKGROUND;

					//update current pixel model
					rand = GetRand() % (model->upSpeed);
					if (rand == 0) 
					{
						rand = GetRand() % (model->nSample);
						model->bg_samples[sampleOffset + rand] = image[imageOffset];  
					}
					//update neighboring pixel model
					rand = GetRand() % (model->upSpeed);
					if (rand == 0) 
					{
						GetRandNeighborXY(x, y, &xNG, &yNG);
						rand = GetRand() % (model->nSample);
						model->bg_samples[yNG * model->width * model->nSample + xNG * model->nSample + rand] = image[imageOffset];
					}
					//siltp (update quickly since it's back point)
					rand = GetRand() % SILTPUPDATESPEEDNOR;
					if (rand == 0)
					{
						rand = GetRand() % SILTPSAMPLES;
						Siltp41C1SetModel(x, y, rand, Siltp41C1GetImage(x, y));
					}

				}
			}

        }// end of for x loop
    }// end of for y loop
}

void VibeModelUpdateC3(VibeModel_t *model, uint8_t *image, uint8_t *greyImage, uint8_t *output)
{
	int32_t x, y, xNG, yNG, i;
	int32_t count, index, count2, index2, dist2;
	double dist;
	int32_t sampleOffset, imageOffset;
	uint32_t rand;

	Mat tmpPointBGR(1, 1, CV_8UC3), pointHSVBack(1, 1, CV_8UC3), pointHSVFore(1, 1, CV_8UC3);
	uint32_t bgrSum[3];
	uint8_t  bgrDiv[3];
	uint32_t isShadow;

	Siltp41C1(greyImage);
    for (y = 0; y < model->height; y++) 
	{
        for (x = 0; x < model->width; x++) 
		{
			sampleOffset = (y * model-> width * model->nSample + x * model->nSample) * 3;
			imageOffset  = (y * model->width + x) * 3;
            //compare pixel to background model
            count = 0; index = 0;
            while ((count < model->nMatch) && (index < model->nSample)) 
			{
                //Euclidean distance computation
				dist = (double)((image[imageOffset] - model->bg_samples[sampleOffset + index * 3])*(image[imageOffset] - model->bg_samples[sampleOffset + index * 3])) +
					   (double)((image[imageOffset + 1] - model->bg_samples[sampleOffset + index * 3 + 1])*(image[imageOffset + 1] - model->bg_samples[sampleOffset + index * 3 + 1])) +
					   (double)((image[imageOffset + 2] - model->bg_samples[sampleOffset + index * 3 + 2])*(image[imageOffset + 2] - model->bg_samples[sampleOffset + index * 3 + 2]));
				if (dist < model->threshold * model->threshold * 3) 
				{
					count++;
				}
                index++;
            }
			//siltp
			count2 = 0; index2 = 0;
			while ((count2 < SILTPCOUNTTHRESHOLD) && (index2 < SILTPSAMPLES)) 
			{
				dist2 = Siltp41C1CalDistance(Siltp41C1GetImage(x, y), Siltp41C1GetModel(x, y, index2));
				if (dist2 < SILTPDISTHRESHOLD) 
				{ 
					count2++;
				}
				index2++;
			}
            //classify pixel and update model
			// classify pixel and update model
			// siltp & point update, if both background -> background
			//						 if point is background, siltp is forground -> check if siltp is valid
			//						 if point is forground, siltp is backround  -> shadow
			//						 if both forground, forground -> foreground
			if (count < model->nMatch && count2 < SILTPCOUNTTHRESHOLD)
			{
				output[y * model->width + x] = DEFAULT_COLOR_FOREGROUND;

				if (g_frameCount < model->nSample)
				{
					//current pixel
					rand = GetRand() % (model->nSample);
					model->bg_samples[sampleOffset + rand * 3] = image[imageOffset];  
					model->bg_samples[sampleOffset + rand * 3 + 1] = image[imageOffset + 1];  
					model->bg_samples[sampleOffset + rand * 3 + 2] = image[imageOffset + 2]; 
					//neighbor
					GetRandNeighborXY(x, y, &xNG, &yNG);
					rand = GetRand() % (model->nSample);
					model->bg_samples[(yNG * model->width * model->nSample + xNG * model->nSample + rand) * 3] = image[imageOffset];
					model->bg_samples[(yNG * model->width * model->nSample + xNG * model->nSample + rand) * 3 + 1] = image[imageOffset + 1];
					model->bg_samples[(yNG * model->width * model->nSample + xNG * model->nSample + rand) * 3 + 2] = image[imageOffset + 2];
				}
				if (g_frameCount < SILTPSAMPLES)
				{
					//siltp
					rand = GetRand() % SILTPSAMPLES;
					Siltp41C1SetModel(x, y, rand, Siltp41C1GetImage(x, y));
				}
				else
				{
					//update siltp back model(slowly update since it's fore)
					rand = GetRand() % SILTPUPDATESPEEDLOW;
					if (rand == 0)
					{
						rand = GetRand() % SILTPSAMPLES;
						Siltp41C1SetModel(x, y, rand, Siltp41C1GetImage(x, y));
					}
				}
			}
			else
			{
				//shadow (vibe classify as forground & siltp classify as background)
				if (count < model->nMatch) 
				{
					isShadow = 0;
					if (g_frameCount < model->nSample)
					{
						output[y * model->width + x] = DEFAULT_COLOR_FOREGROUND;
						//current pixel
						rand = GetRand() % (model->nSample);
						model->bg_samples[sampleOffset + rand * 3] = image[imageOffset];  
						model->bg_samples[sampleOffset + rand * 3 + 1] = image[imageOffset + 1];  
						model->bg_samples[sampleOffset + rand * 3 + 2] = image[imageOffset + 2]; 
						//neighbor
						GetRandNeighborXY(x, y, &xNG, &yNG);
						rand = GetRand() % (model->nSample);
						model->bg_samples[(yNG * model->width * model->nSample + xNG * model->nSample + rand) * 3] = image[imageOffset];
						model->bg_samples[(yNG * model->width * model->nSample + xNG * model->nSample + rand) * 3 + 1] = image[imageOffset + 1];
						model->bg_samples[(yNG * model->width * model->nSample + xNG * model->nSample + rand) * 3 + 2] = image[imageOffset + 2];
					}
					else
					{
						bgrSum[0] = 0; bgrSum[1] = 0; bgrSum[2] = 0;
						for (i = 0; i < model->nSample; i++)
						{
							bgrSum[0] += model->bg_samples[sampleOffset + i * 3];
							bgrSum[1] += model->bg_samples[sampleOffset + i * 3 + 1];
							bgrSum[2] += model->bg_samples[sampleOffset + i * 3 + 2];
						}
						bgrDiv[0] = bgrSum[0] / model->nSample;
						bgrDiv[1] = bgrSum[1] / model->nSample;
						bgrDiv[2] = bgrSum[2] / model->nSample;
						tmpPointBGR.data = bgrDiv;
						cvtColor(tmpPointBGR, pointHSVBack, CV_BGR2HSV);
						tmpPointBGR.data = &(image[imageOffset]);
						cvtColor(tmpPointBGR, pointHSVFore, CV_BGR2HSV);
						//((pointHSVFore.data)[2] * 1.0 / (pointHSVBack.data)[2] < 0.9)
						if ( ((pointHSVFore.data)[2] * 1.0 / (pointHSVBack.data)[2] < 0.9) && (abs((pointHSVFore.data)[1] - (pointHSVBack.data)[1]) <= 25) && (abs((pointHSVFore.data)[0] - (pointHSVBack.data)[0]) <= 15))
						{
							output[y * model->width + x] = DEFAULT_COLOR_SHADOW;
							//update current pixel model
							//rand = GetRand() % (model->upSpeed * 3);
							//if (rand == 0) 
							//{
							//	//replace randomly chosen sample
							//	rand = GetRand() % (model->nSample);
							//	model->bg_samples[sampleOffset + rand] = image[imageOffset];  
							//}
							////update neighboring pixel model
							//rand = GetRand() % (model->upSpeed * 3);
							//if (rand == 0) 
							//{
							//	GetRandNeighborXY(x, y, &xNG, &yNG);
							//	rand = GetRand() % (model->nSample);
							//	model->bg_samples[yNG * model->width * model->nSample + xNG * model->nSample + rand] = image[imageOffset];
							//}
						}
						else
						{
							output[y * model->width + x] = DEFAULT_COLOR_FOREGROUND;
						}
					}

					if (g_frameCount < SILTPSAMPLES)
					{
						rand = GetRand() % SILTPSAMPLES;
						Siltp41C1SetModel(x, y, rand, Siltp41C1GetImage(x, y));
					}
					else
					{
						//update siltp back model(quickly update since it's back)
						if (isShadow) rand = GetRand() % SILTPUPDATESPEEDLOW;
						else rand = GetRand() % SILTPUPDATESPEEDNOR;
						if (rand == 0)
						{
							rand = GetRand() % SILTPSAMPLES;
							Siltp41C1SetModel(x, y, rand, Siltp41C1GetImage(x, y));
						}
					}
				}
				//siltp judged as forground, vibe judged as background( since this area is high dynamic)
				else if (count2 < SILTPCOUNTTHRESHOLD)
				{
					if (g_frameCount < model->nSample)
					{
						output[y * model->width + x] = DEFAULT_COLOR_BACKGROUND;
						//current pixel
						rand = GetRand() % (model->nSample);
						model->bg_samples[sampleOffset + rand * 3] = image[imageOffset];  
						model->bg_samples[sampleOffset + rand * 3 + 1] = image[imageOffset + 1];  
						model->bg_samples[sampleOffset + rand * 3 + 2] = image[imageOffset + 2]; 
						//neighbor
						GetRandNeighborXY(x, y, &xNG, &yNG);
						rand = GetRand() % (model->nSample);
						model->bg_samples[(yNG * model->width * model->nSample + xNG * model->nSample + rand) * 3] = image[imageOffset];
						model->bg_samples[(yNG * model->width * model->nSample + xNG * model->nSample + rand) * 3 + 1] = image[imageOffset + 1];
						model->bg_samples[(yNG * model->width * model->nSample + xNG * model->nSample + rand) * 3 + 2] = image[imageOffset + 2];
					}
					else
					{
						output[y * model->width + x] = DEFAULT_COLOR_FOREGROUND;
						//update current pixel model
						rand = GetRand() % (model->upSpeed);
						if (rand == 0) 
						{
							rand = GetRand() % (model->nSample);
							model->bg_samples[sampleOffset + rand * 3] = image[imageOffset];
							model->bg_samples[sampleOffset + rand * 3 + 1] = image[imageOffset + 1];          
							model->bg_samples[sampleOffset + rand * 3 + 2] = image[imageOffset + 2];    
						}
						//update neighboring pixel model
						rand = GetRand() % (model->upSpeed);
						if (rand == 0) 
						{
							GetRandNeighborXY(x, y, &xNG, &yNG);
							rand = GetRand() % (model->nSample);
							model->bg_samples[(yNG * model->width * model->nSample + xNG * model->nSample + rand) * 3] = image[imageOffset];
							model->bg_samples[(yNG * model->width * model->nSample + xNG * model->nSample + rand) * 3 + 1] = image[imageOffset + 1];
							model->bg_samples[(yNG * model->width * model->nSample + xNG * model->nSample + rand) * 3 + 2] = image[imageOffset + 2];
						}
					}
					//siltp
					if (g_frameCount < SILTPSAMPLES)
					{
						rand = GetRand() % SILTPUPDATESPEEDHIGH;
						if (rand == 0)
						{
							rand = GetRand() % SILTPSAMPLES;
							Siltp41C1SetModel(x, y, rand, Siltp41C1GetImage(x, y));
						}
					}
					else
					{
						rand = GetRand() % SILTPSAMPLES;
						Siltp41C1SetModel(x, y, rand, Siltp41C1GetImage(x, y));
					}
				}
				//both judged as background
				else 
				{
					output[y * model->width + x] = DEFAULT_COLOR_BACKGROUND;

					if (g_frameCount < model->nSample)
					{
						//curr
						rand = GetRand() % (model->nSample);
						model->bg_samples[sampleOffset + rand * 3] = image[imageOffset];  
						model->bg_samples[sampleOffset + rand * 3 + 1] = image[imageOffset + 1];  
						model->bg_samples[sampleOffset + rand * 3 + 2] = image[imageOffset + 2]; 
						//neighbor
						GetRandNeighborXY(x, y, &xNG, &yNG);
						rand = GetRand() % (model->nSample);
						model->bg_samples[(yNG * model->width * model->nSample + xNG * model->nSample + rand) * 3] = image[imageOffset];
						model->bg_samples[(yNG * model->width * model->nSample + xNG * model->nSample + rand) * 3 + 1] = image[imageOffset + 1];
						model->bg_samples[(yNG * model->width * model->nSample + xNG * model->nSample + rand) * 3 + 2] = image[imageOffset + 2];
					}
					else
					{
						//update current pixel model
						rand = GetRand() % (model->upSpeed);
						if (rand == 0) 
						{
							rand = GetRand() % (model->nSample);
							model->bg_samples[sampleOffset + rand * 3] = image[imageOffset];  
							model->bg_samples[sampleOffset + rand * 3 + 1] = image[imageOffset + 1];  
							model->bg_samples[sampleOffset + rand * 3 + 2] = image[imageOffset + 2];  
						}
						//update neighboring pixel model
						rand = GetRand() % (model->upSpeed);
						if (rand == 0) 
						{
							GetRandNeighborXY(x, y, &xNG, &yNG);
							rand = GetRand() % (model->nSample);
							model->bg_samples[(yNG * model->width * model->nSample + xNG * model->nSample + rand) * 3] = image[imageOffset];
							model->bg_samples[(yNG * model->width * model->nSample + xNG * model->nSample + rand) * 3 + 1] = image[imageOffset + 1];
							model->bg_samples[(yNG * model->width * model->nSample + xNG * model->nSample + rand) * 3 + 2] = image[imageOffset + 2];
						}
					}
					if (g_frameCount < SILTPSAMPLES)
					{
						rand = GetRand() % SILTPSAMPLES;
						Siltp41C1SetModel(x, y, rand, Siltp41C1GetImage(x, y));
					}
					else
					{
						//siltp (update quickly since it's back point)
						rand = GetRand() % SILTPUPDATESPEEDNOR;
						if (rand == 0)
						{
							rand = GetRand() % SILTPSAMPLES;
							Siltp41C1SetModel(x, y, rand, Siltp41C1GetImage(x, y));
						}
					}
				}
			}

        }// end of for x loop
    }// end of for y loop

	g_frameCount++;
}

void VibeModelFree(VibeModel_t *model)
{
	free(model->bg_samples);
	FreeRand();
	FreeRandNeighbor();
	Siltp41C1Free();
}
