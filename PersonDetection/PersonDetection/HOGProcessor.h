#pragma once

#include "cxcore.h"
#include "cv.h"
#include "highgui.h"
#include <fstream>
#include <cmath>

#define PI 3.14

class HOGProcessor
{
public:
	static const int NumOfBins = 9;
	CvSize m_cell;// The default size of each cell is 8x8 (pixels)
	CvSize m_block;// The default size of each block is 2x2 (cells)
	float m_fStepOverlap;// Calculated in unit of cell rate

	HOGProcessor(void);

	void setParams(CvSize cell, CvSize block, float stepOverLap);
	int getWindowFeatureVectorSize(CvSize window);
	int getWindowFeatureVectorSize(CvSize window, CvSize block, CvSize cell, float overlap);
	IplImage* doSobel(IplImage* in, int xorder, int yorder, int apertureSize);
	IplImage** calculateIntegralHOG(IplImage* in);
	void calcHOGForCell(CvRect cell, CvMat* hogCell, IplImage** integrals, int normalization);
	void calcHOGForBlock(CvRect block, CvMat* hogBlock, IplImage** integrals, CvSize cell, int normalization);
	CvMat* calcHOGWindow(IplImage** integrals, CvRect window, int normalization);
	CvMat* calcHOGWindow(IplImage *img, IplImage** integrals, CvRect window, int normalization);
	CvMat* calcHOGFromImage(char* filename, CvSize window, int normalization);
	void writeFeatureVector(CvMat* mat, char* className, std::ofstream &fout);

	~HOGProcessor(void);
};
