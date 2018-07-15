#pragma once

#include "cxcore.h"
#include <cmath>
#include "HOGProcessor.h"
#include "HOGClassifier.h"
#include "ScanWindow.h"
#include "OverlappingDetection.h"
#include "IntegralScale.h"

class Localisation
{
public:
	int _numberOfDetectedWindows;
	HOGProcessor hogProcessor;
	HOGClassifier *_hogClassifier;
	IplImage* _testImage;
	char* _trainPath;

	std::vector<ScanWindow*> _detectedWindows;	// SCALE luu trong nay la nguyen ban, chua dua qua ham Log
	ScanWindow** _detectedWindows2;
	std::vector<CvMat*> _positionScaleSpace;
	std::vector<float> _logScale;
	std::vector<float> _weightOfWindows;

	std::vector<CvMat*> _CovarianceMatrixH;

	std::vector<IntegralScale*> _integralScale;
	CvMat* _previousMeanShiftWindow;
	CvMat* _finalLocalisationWindow;

	float _sigmaX;	// Gia tri paper 8, nhung 4 lai cho best performance
	float _sigmaY;	// Gia tri paper 16, nhung 8 lai cho best performance
	float _sigmaS;	// Gia tri paper log(1.3)
	float _scaleRatio;	// Gia tri paper 1.05

	Localisation(void);
	Localisation(std::vector<ScanWindow*> detectedWindows, float sigmaX, float sigmaY, float sigmaS, float scaleRatio,
		HOGClassifier* classifier, char* trainPath, IplImage* testImage, std::vector<IntegralScale*> integralScale);

	Localisation(ScanWindow** detectedWindowsPtr, float sigmaX, float sigmaY, float sigmaS, float scaleRatio,
		HOGClassifier* classifier, char* trainPath, IplImage* testImage, std::vector<IntegralScale*> integralScale);

	void constructPositionScaleSpace();	// Tao khong gian 3 chieu x, y, scale (log)
	void calculateCovarianceMatrixH();
	float calculateMahalanobisDistance(CvMat* y, CvMat* yi, CvMat* Hi);
	float clipWiFunction(float wi, float threshold_c, float a, std::string clippingType);
	float calculatePowerOfDeterminant(CvMat* matrix, float power);
	float calculateVariantPi(int index_yi, int index_y);
	CvMat* calculateDataWeightedHarmonicMean(int index_y);
	CvMat* computeMeanShiftWindow(int index_y);
	void increasePositionScaleSpace(int increaseSize);
	void addASampleToSpace(CvMat* newSample);
	void updateCovarianceMatrix();
	void localize();
	void localize2(int size);

	~Localisation(void);
};
