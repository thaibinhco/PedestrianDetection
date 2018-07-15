#include "stdafx.h"
#include "OverlappingDetection.h"

OverlappingDetection::OverlappingDetection(void)
{
	_numberOfWindows = 0;
}

OverlappingDetection::OverlappingDetection(std::vector<ScanWindow*> *detectedWindows)
{
	_windows = detectedWindows;
	_nGroup = 0;
}

OverlappingDetection::OverlappingDetection(int numberOfDetectedWindows, std::vector<ScanWindow*> *detectedWindows)
{
	_numberOfWindows = numberOfDetectedWindows;
	_windows = detectedWindows;
}

OverlappingDetection::~OverlappingDetection(void)
{
	_windows->clear();
}

std::vector<float> OverlappingDetection::getOverlappingPercent(ScanWindow *window1, ScanWindow *window2)
{
	std::vector<float> overlappingPercent;
	std::vector<CvPoint> overlappingWindow;

	CvPoint *factorToCreateCornerPoint = new CvPoint[3];
	CvPoint *window1Corners = new CvPoint[4];
	CvPoint *window2Corners = new CvPoint[4];

	factorToCreateCornerPoint[0] = cvPoint(1, 0);
	factorToCreateCornerPoint[1] = cvPoint(0, 1);
	factorToCreateCornerPoint[2] = cvPoint(1, 1);

	// Khoi tao gia tri
	overlappingPercent.push_back(0);
	overlappingPercent.push_back(0);

	// Tao ra tap 4 diem cua cua so 1 va 2
	window1Corners[0] = cvPoint((int)window1->getXCoordinate() * window1->getScale(), (int)window1->getYCoordinate() * window1->getScale());
	window2Corners[0] = cvPoint((int)window2->getXCoordinate() * window2->getScale(), (int)window2->getYCoordinate() * window2->getScale());

	// Tao ra cac width va height hop ly
	for (int i = 0; i < 3; i++)
	{
		window1Corners[i + 1] = cvPoint((int)window1Corners[0].x +
			(int)(factorToCreateCornerPoint[i].x * (window1->getWidth() * window1->getScale())),
			(int)window1Corners[0].y + (int)(factorToCreateCornerPoint[i].y * (window1->getHeight() * window1->getScale())));

		window2Corners[i + 1] = cvPoint((int)window2Corners[0].x +
			(int)(factorToCreateCornerPoint[i].x * (window2->getWidth() * window2->getScale())),
			(int)window2Corners[0].y + (int)(factorToCreateCornerPoint[i].y * (window2->getHeight() * window2->getScale())));
	}

	// Kiem tra xem nhung diem goc cua cua so A co nam trong phan dien tich cua cua so B hay khong va nguoc lai
	for (int i = 0; i < 4; i++)
	{
		if (window1Corners[i].x >= window2Corners[0].x &&
			window1Corners[i].x <= window2Corners[3].x &&
			window1Corners[i].y >= window2Corners[0].y &&
			window1Corners[i].y <= window2Corners[3].y)
		{
			overlappingWindow.push_back(window1Corners[i]);
		}

		if (window2Corners[i].x >= window1Corners[0].x &&
			window2Corners[i].x <= window1Corners[3].x &&
			window2Corners[i].y >= window1Corners[0].y &&
			window2Corners[i].y <= window1Corners[3].y)
		{
			overlappingWindow.push_back(window2Corners[i]);
		}
	}

	// Kiem tra xem nhung diem giao nhau cua cac boundary cua window A va B
	CvPoint *boundaryLineSegment = new CvPoint[4];

	boundaryLineSegment[0] = cvPoint(0, 1);
	boundaryLineSegment[1] = cvPoint(0, 2);
	boundaryLineSegment[2] = cvPoint(1, 3);
	boundaryLineSegment[3] = cvPoint(2, 3);

	LineSegment *l1 = new LineSegment();
	LineSegment *l2 = new LineSegment();

	for (int i = 0; i < 4; i++)
	{
		l1->_begin = window1Corners[boundaryLineSegment[i].x];
		l1->_end = window1Corners[boundaryLineSegment[i].y];
		l1->calculateCoEfficient();

		for (int j = 0; j < 4; j++)
		{
			l2->_begin = window2Corners[boundaryLineSegment[j].x];
			l2->_end = window2Corners[boundaryLineSegment[j].y];
			l2->calculateCoEfficient();

			if (l1->_a * l2->_b != l2->_a * l1->_b)	// Neu nhu 2 duong thang khong song song voi nhau thi moi tim giao diem
			{
				CvPoint intersection = l1->getIntersection(l1, l2);
				if (intersection.x != -1 && intersection.y != -1)	// Neu ton tai giao diem
				{
					overlappingWindow.push_back(intersection);
				}
			}

		}

	}

	delete l1;
	delete l2;
	// Tim dien tich hinh chu nhat giao giua 2 cua so 
	if (overlappingWindow.size() >= 4)
	{
		CvPoint minPoint = overlappingWindow[0];
		CvPoint maxPoint = overlappingWindow[0];
		for (int i = 0; i < overlappingWindow.size(); i++)
		{
			if (minPoint.x >= overlappingWindow[i].x && minPoint.y >= overlappingWindow[i].y)
			{
				minPoint = overlappingWindow[i];
			}

			if (maxPoint.x <= overlappingWindow[i].x && maxPoint.y <= overlappingWindow[i].y)
			{
				maxPoint = overlappingWindow[i];
			}
		}

		float intersectionArea = (maxPoint.x - minPoint.x) * (maxPoint.y - minPoint.y);
		float window1Area = (window1Corners[3].x - window1Corners[0].x) * (window1Corners[3].y - window1Corners[0].y);
		float window2Area = (window2Corners[3].x - window2Corners[0].x) * (window2Corners[3].y - window2Corners[0].y);

		overlappingPercent[0] = intersectionArea / window1Area;
		overlappingPercent[1] = intersectionArea / window2Area;
	}

	// Release vung nho	
	overlappingWindow.clear();

	/*delete[]factorToCreateCornerPoint;
	delete[]window1Corners;
	delete[]window2Corners;
	delete[]boundaryLineSegment;*/

	return overlappingPercent;
}

void OverlappingDetection::clusterWindows()
{
	for (int i = 0; i < _windows->size(); i++)
	{
		if (i == 0)	// Lay cua so dau tien lam cum dau tien
		{
			std::vector<int> group;
			group.push_back(i);
			_clusteredWindows.push_back(group);
		}
		else
		{
			float maxPercentOverlappedInOneCluster = 0;
			int maxOverlappedCluster = 0;

			for (int j = 0; j < _clusteredWindows.size(); j++)
			{
				int overlapWindowsInOneCluster = 0;
				float percentOverlappedInOneCluster = 0;

				for (int k = 0; k < _clusteredWindows[j].size(); k++)
				{
					std::vector<float> overlap = getOverlappingPercent(_windows->at(i), _windows->at(_clusteredWindows[j][k]));	// Tinh phan tram overlap cua 2 cua so
					float maxPercent = (float)MAX(overlap[0], overlap[1]);

					if (maxPercent >= 0.60)	// Neu overlap hon 2/3, bo if nay de tang toc
					{
						overlapWindowsInOneCluster++;	// Neu cua so thu i dang xet overlap voi tung cua so trong cum thi dem so cua so bi overlap
					}
				}

				// Tinh phan tram so cua so bi overlap trong 1 cluster
				percentOverlappedInOneCluster = ((float)(overlapWindowsInOneCluster)) / _clusteredWindows[j].size();

				if (percentOverlappedInOneCluster > maxPercentOverlappedInOneCluster)	// Tim ra cluster vi overlap nhieu nhat trong N cluster
				{
					maxPercentOverlappedInOneCluster = percentOverlappedInOneCluster;
					maxOverlappedCluster = j;	// Luu lai thu tu cua cum bi overlap nhieu nhat
				}
			}

			// Neu cua so khong overlap voi bat ky cum nao, thi no tu tao thanh cum moi
			if (maxPercentOverlappedInOneCluster == 0)
			{
				_nGroup++;
				std::vector<int> group;
				group.push_back(i);
				_clusteredWindows.push_back(group);
			}
			else	// Con neu no overlap nhieu nhat voi cum nao thi cho no vao cum do
			{
				_clusteredWindows[maxOverlappedCluster].push_back(i);
			}
		}
	}
}
