#pragma once

#include "cxcore.h"
#include "ml.h"
#include "HOGProcessor.h"

class HOGClassifier
{
public:
	CvSVM svmClassifier;
	HOGProcessor hog;

	HOGClassifier(void);

	void setParams(CvSize cell, CvSize block, float stepOverLap);

	IplImage** calcIntegralHOG(IplImage* in);

	CvMat* trainDataset_64x128(std::string src, CvSize cell, CvSize window,
		char* saveXML = NULL, int canny = 0,
		int block = 1, int normalization = 4);

	CvMat* trainLargeDataset(std::string src, CvSize cell, CvSize window,
		int horizontalScans, int verticalScans,
		char* saveXML = NULL, int normalization = 4);

	void loadTrainSVM(char* trainXMLPath);

	void trainSVM(CvMat* posMat, CvMat* negMat, char* saveXML, char* posData = NULL, char* negData = NULL);

	float testSVM(char* trainXML, char* testData, char* saveData);
	float testSVM(char* trainXML, IplImage* testImg, char* saveData);
	// Ham TestSVM su dung integrals HOG (ung voi moi bins)
	float testSVM(char* trainXML, IplImage** integrals, char* saveData);

	~HOGClassifier(void);
};
