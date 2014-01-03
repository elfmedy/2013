#pragma once

#ifndef DLLEXPORT
#define DLLEXPORT __declspec(dllexport)
#endif

#include <windows.h>
#include <vector>

#include "cxcore.h"

#include "Detector.h"
#include "DetectorManager.h"
#include "PostProcess.h"
#include "Common.h"

using namespace std;

class CFaceDetector
{
public:
	 DLLEXPORT CFaceDetector(void);
	 DLLEXPORT  ~CFaceDetector(void);
	 DLLEXPORT bool InitSize(int srcW, int srcH);
	 DLLEXPORT int Detect(IplImage * srcImage, SFaceItems &faceItems);

protected:
	vector<RECT> faceRects;
	vector<int> faceViews,faceAngles;
		
	int m_srcW;
	int m_srcH;

	CDetectorManager m_DetectorManager;
	CDetector m_Detector;
	CMerger m_Merger;

	SDetectorConfig m_Config;
	STreeConfig m_TreeConfig;
	SMergeConfig m_MergeConfig;

	SFaceItems m_Faces;
};
