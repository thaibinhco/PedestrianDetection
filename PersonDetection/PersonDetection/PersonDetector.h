#pragma once

#include "HOGClassifier.h"
#include "ScanWindow.h"
#include "OverlappingDetection.h"
#include "Localisation.h"
#include "IntegralScale.h"
#include "window.h"

class PersonDetector
{
private:
	int DetectPosWindows(IplImage* img);
	int m_widthStep;	// Tinh theo pixel
	int m_heigthStep;	// Tinh theo pixel
	CvSize m_cell;
	CvSize m_block;
	CvSize m_winSize;
	int m_stepOverlap;

	// Tham so dung cho viec duyet cua so
	float startScale;
	float endScale;
	float scaleStep;
	int nScaleStep;

	char* svmTrainPath;

	//void calcScaleStep();

public:
	HOGClassifier classifier;
	std::vector<ScanWindow*> m_detectedWindows;

	PersonDetector();

	void setParams(CvSize cell, CvSize block, float stepOverLap);
	void setSVMTrainPath(char* path);
	std::pair<IplImage*, std::vector<window>> DetectMultiScale(IplImage* srcImg);

	//~PersonDetector();
};
