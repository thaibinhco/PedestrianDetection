#pragma once

#include "cxcore.h"
#include "ScanWindow.h"
#include "LineSegment.h"

class OverlappingDetection
{
public:
	int _numberOfWindows;
	std::vector<ScanWindow*> *_windows;
	std::vector<std::vector<int>> _clusteredWindows;
	int _nGroup;

	OverlappingDetection(void);
	OverlappingDetection(std::vector<ScanWindow*> *detectedWindows);
	OverlappingDetection(int numberOfDetectedWindows, std::vector<ScanWindow*> *detectedWindows);

	std::vector<float> getOverlappingPercent(ScanWindow *window1, ScanWindow *window2);
	void clusterWindows();

	~OverlappingDetection(void);
};
