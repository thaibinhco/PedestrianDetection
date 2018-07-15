#include "stdafx.h"
#include "PersonDetector.h"

PersonDetector::PersonDetector()
{
	m_cell = cvSize(8, 8);
	m_block = cvSize(2, 2);
	m_winSize = cvSize(64, 128);
	m_stepOverlap = 1;
	classifier.setParams(m_cell, m_block, m_stepOverlap);

	startScale = 1;
	scaleStep = 1.5;
	m_widthStep = 8;
	m_heigthStep = 8;

	svmTrainPath = "Dataset/small train/TrainSVM.xml";
}

//PersonDetector::~PersonDetector() {}

int PersonDetector::DetectPosWindows(IplImage* img)
{
	int nWins = 0;
	return nWins;
}

void PersonDetector::setParams(CvSize cell, CvSize block, float stepOverLap)
{
	m_cell = cell;
	m_block = block;
	m_stepOverlap = stepOverLap;
	classifier.setParams(cell, block, stepOverLap);
}

void PersonDetector::setSVMTrainPath(char* path)
{
	svmTrainPath = path;
	classifier.loadTrainSVM(path);
}

/// Ham detect people multiscale
// Params: srcImgPath: Duong dan toi file anh test
// Return value: So luong nguoi detect duoc
std::pair<IplImage*, std::vector<window>> PersonDetector::DetectMultiScale(IplImage* srcImg)
{
	int count = 0;
	CvSize size = cvGetSize(srcImg);
	std::vector<IntegralScale*> integralsScale;

	if (size.width > 400)
	{
		startScale = 2.5;
	}
	else if (size.width > 300)
	{
		startScale = 2;
	}

	// Tinh toan so buoc scale anh	
	endScale = std::min((float)size.width / m_winSize.width, (float)size.height / m_winSize.height);
	nScaleStep = floor(log(endScale / startScale) / log(scaleStep) + 1);

	// Khoi tao kich thuoc scale ban dau
	float scale = startScale;

	// Duyet qua tat ca cac muc scale
	for (int k = 0; k <= nScaleStep; k++)
	{
		IplImage* input = NULL;
		if (k == nScaleStep)
		{
			// Kiem tra neu buoc scale truoc do da cho ti le anh input bang ti le cua so
			if ((int)(srcImg->width / scale) == m_winSize.width &&
				(int)(srcImg->height / scale) == m_winSize.height)
			{
				break;
			}
			else// Nguoc lai thuc hien them 1 lan test voi kich thuoc anh scale bang dung kich thuoc cua so
			{
				input = cvCreateImage(cvSize((int)(m_winSize.width), (int)(m_winSize.height)),
					srcImg->depth, srcImg->nChannels);
			}
		}
		else
		{
			input = cvCreateImage(cvSize((int)(srcImg->width / scale), (int)(srcImg->height / scale)),
				srcImg->depth, srcImg->nChannels);
		}
		cvResize(srcImg, input);

		int x = 0;
		int y = 0;
		int nBins = classifier.hog.NumOfBins;
		IplImage** integrals = classifier.calcIntegralHOG(input);

		// Add integral image + scale to list
		IntegralScale *integralScaleImage = new IntegralScale(integrals, scale);

		integralsScale.push_back(integralScaleImage);

		IplImage** integralsInput = (IplImage**)malloc(nBins * sizeof(IplImage*));
		while (y + m_winSize.height <= integrals[0]->height)
		{
			x = 0;
			while (x + m_winSize.width <= integrals[0]->width)
			{
				// Set region of interest				
				for (int i = 0; i < nBins; i++)
				{
					cvSetImageROI(integrals[i], cvRect(x, y, m_winSize.width + 1, m_winSize.height + 1));
					integralsInput[i] = cvCreateImage(cvGetSize(integrals[i]), integrals[i]->depth, integrals[i]->nChannels);
					cvCopy(integrals[i], integralsInput[i], NULL);
				}

				// Copy subimage
				//cvCopy(input, img, NULL);

				float weight = classifier.testSVM(svmTrainPath, integralsInput, NULL);
				if (weight != 2)	// -2.4115036f
				{
					ScanWindow *window = new ScanWindow(x, y, scale, 64, 128);	// Changed 3rd parameter from 1 to scale
					window->setWeight(weight);
					m_detectedWindows.push_back(window);
				}
				for (int i = 0; i < nBins; i++)
				{
					cvResetImageROI(integrals[i]);
				}

				x += m_widthStep;
				for (int i = 0; i < nBins; i++)
				{
					cvReleaseImage(&integralsInput[i]);
				}
			}
			y += m_heigthStep;
		}

		for (int i = 0; i < nBins; i++)
		{
			cvReleaseImage(&integrals[i]);
		}

		free(integrals);
		free(integralsInput);
		cvReleaseImage(&input);
		scale *= scaleStep;
	}

	// PHAN LOCALIZATION 
	// Input: tap cac cua so o cac muc scale da detect duoc tren anh test input
	// Output: tra ra tap cac cua so da HOI TU
	int numberWindows = -1;
	OverlappingDetection *overlapDetector;
	std::vector<CvMat*> boundaryList;
	while (true)
	{
		overlapDetector = new OverlappingDetection(&m_detectedWindows);
		// Cluster nhung cum cua so overlap len nhau
		overlapDetector->clusterWindows();

		for (int i = 0; i < overlapDetector->_clusteredWindows.size(); i++)
		{
			// Doi voi moi cum, tao ra mot doi tuong localization de localize
			ScanWindow** listOverlapped = new ScanWindow*[overlapDetector->_clusteredWindows[i].size()];
			for (int j = 0; j < overlapDetector->_clusteredWindows[i].size(); j++)
			{
				listOverlapped[j] = m_detectedWindows[overlapDetector->_clusteredWindows[i][j]];
			}

			Localisation *localize = new Localisation(listOverlapped, m_widthStep, m_heigthStep, log(1.03), scaleStep, &classifier, svmTrainPath, srcImg, integralsScale);

			// Tinh toan cua so cuoi cung de tim boundary 
			localize->localize2(overlapDetector->_clusteredWindows[i].size());

			// Lay ra boundary
			boundaryList.push_back(localize->_finalLocalisationWindow);
			for (int j = 0; j < overlapDetector->_clusteredWindows[i].size(); j++)
			{
				delete listOverlapped[j];
			}
			delete listOverlapped;
			delete localize;
		}

		if (boundaryList.size() == numberWindows)
		{
			break;
		}
		else
		{
			numberWindows = boundaryList.size();
			// Huy vung nho
			m_detectedWindows.clear();

			for (int i = 0; i < boundaryList.size(); i++)
			{
				float scaleWindow = cvGetReal2D(boundaryList[i], 2, 0);
				int x = (int)cvGetReal2D(boundaryList[i], 0, 0);
				int y = (int)cvGetReal2D(boundaryList[i], 1, 0);
				x = (int)x / scaleWindow;
				y = (int)y / scaleWindow;
				ScanWindow* window = new ScanWindow(x, y, scaleWindow, 64, 128);
				m_detectedWindows.push_back(window);
			}

			for (int i = 0; i < boundaryList.size(); i++)
			{
				cvReleaseMat(&boundaryList[i]);
			}

			free(overlapDetector);
			boundaryList.clear();
		}
	}// Ket thuc phan localization

	 // Ve Bounding box o day, su dung danh sach boundaryList 
	 // Trong do, cua so thu i bao gom
	 // x = cvGetReal2D(boundaryList[i], 0, 0)
	 // y = cvGetReal2D(boundaryList[i], 1, 0)
	 // scale = cvGetReal2D(boundaryList[i], 2, 0)
	IplImage* resImage = cvCloneImage(srcImg);
	std::vector<window> windows;

	float scaleWindow;
	window w;
	for (int i = 0; i < boundaryList.size(); i++)
	{
		scaleWindow = cvGetReal2D(boundaryList[i], 2, 0);
		w.x = (int)cvGetReal2D(boundaryList[i], 0, 0);
		w.y = (int)cvGetReal2D(boundaryList[i], 1, 0);

		w.width = 64 * scaleWindow;
		w.height = 128 * scaleWindow;

		cvRectangle(resImage, cvPoint(w.x, w.y), cvPoint(w.x + (int)w.width, w.y + (int)w.height), cvScalar(0, 255, 255, 0), 2, 8, 0);
		windows.push_back(w);
	}

	count = boundaryList.size();

	// Huy vung nho	
	for (int i = 0; i < boundaryList.size(); i++)
	{
		cvReleaseMat(&boundaryList[i]);
	}

	// Free vung nho
	free(overlapDetector);
	boundaryList.clear();
	integralsScale.clear();
	m_detectedWindows.clear();

	return make_pair(resImage, windows);
}
