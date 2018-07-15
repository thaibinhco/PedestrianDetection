#include "stdafx.h"
#include "Localisation.h"

Localisation::Localisation(void)
{
	_sigmaX = 8;
	_sigmaY = 16;
	_sigmaS = log(1.3);

	_logScale.resize(_detectedWindows.size());
	_CovarianceMatrixH.resize(_detectedWindows.size());
	_weightOfWindows.resize(_detectedWindows.size());
}

Localisation::~Localisation() {}

Localisation::Localisation(std::vector<ScanWindow*> detectedWindows, float sigmaX, float sigmaY, float sigmaS, float scaleRatio,
	HOGClassifier* classifier, char* trainPath, IplImage* testImage, std::vector<IntegralScale*> integralScale)
{
	_sigmaX = sigmaX;
	_sigmaY = sigmaY;
	_sigmaS = sigmaS;
	_scaleRatio = scaleRatio;

	_detectedWindows = detectedWindows;
	_logScale.resize(_detectedWindows.size());
	_CovarianceMatrixH.resize(_detectedWindows.size());
	_weightOfWindows.resize(_detectedWindows.size());

	// Khoi tao cac cua so mean shift
	_previousMeanShiftWindow = cvCreateMat(3, 1, CV_32FC1);
	_finalLocalisationWindow = cvCreateMat(3, 1, CV_32FC1);

	cvSetReal2D(_previousMeanShiftWindow, 0, 0, -1);
	cvSetReal2D(_previousMeanShiftWindow, 1, 0, -1);
	cvSetReal2D(_previousMeanShiftWindow, 2, 0, -1);

	cvSetReal2D(_finalLocalisationWindow, 0, 0, -1);
	cvSetReal2D(_finalLocalisationWindow, 1, 0, -1);
	cvSetReal2D(_finalLocalisationWindow, 2, 0, -1);

	// Khoi tao classifier
	_hogClassifier = classifier;
	_trainPath = trainPath;
	_testImage = testImage;

	_integralScale = integralScale;
}

Localisation::Localisation(ScanWindow** detectedWindows2, float sigmaX, float sigmaY, float sigmaS, float scaleRatio,
	HOGClassifier* classifier, char* trainPath, IplImage* testImage, std::vector<IntegralScale*> integralScale)
{
	_sigmaX = sigmaX;
	_sigmaY = sigmaY;
	_sigmaS = sigmaS;
	_scaleRatio = scaleRatio;

	_detectedWindows2 = detectedWindows2;
	_logScale.resize(_detectedWindows.size());
	_CovarianceMatrixH.resize(_detectedWindows.size());
	_weightOfWindows.resize(_detectedWindows.size());

	// Khoi tao cac cua so mean shift
	_previousMeanShiftWindow = cvCreateMat(3, 1, CV_32FC1);
	_finalLocalisationWindow = cvCreateMat(3, 1, CV_32FC1);

	cvSetReal2D(_previousMeanShiftWindow, 0, 0, -1);
	cvSetReal2D(_previousMeanShiftWindow, 1, 0, -1);
	cvSetReal2D(_previousMeanShiftWindow, 2, 0, -1);

	cvSetReal2D(_finalLocalisationWindow, 0, 0, -1);
	cvSetReal2D(_finalLocalisationWindow, 1, 0, -1);
	cvSetReal2D(_finalLocalisationWindow, 2, 0, -1);

	// Khoi tao classifier
	_hogClassifier = classifier;
	_trainPath = trainPath;
	_testImage = testImage;

	_integralScale = integralScale;
}

void Localisation::constructPositionScaleSpace()
{

	_positionScaleSpace.resize(_detectedWindows.size());

	for (int i = 0; i < _positionScaleSpace.size(); i++)
	{
		CvMat *yi = cvCreateMat(3, 1, CV_32FC1);	// Vector cot (3x1)

		cvSetReal2D(yi, 0, 0, _detectedWindows[i]->getXCoordinate());
		cvSetReal2D(yi, 1, 0, _detectedWindows[i]->getYCoordinate());
		cvSetReal2D(yi, 2, 0, log(_detectedWindows[i]->getScale()));

		_weightOfWindows[i] = _detectedWindows[i]->getWeight();
		_positionScaleSpace[i] = yi;
	}
}

void Localisation::calculateCovarianceMatrixH()
{
	// Tinh gia tri cua so trung binh de tinh ma tran covariance
	CvMat* meanWindows = cvCreateMat(3, 1, CV_32FC1);	// Vector cot 3x1
	cvSetReal2D(meanWindows, 0, 0, 0);
	cvSetReal2D(meanWindows, 1, 0, 0);
	cvSetReal2D(meanWindows, 2, 0, 0);

	for (int i = 0; i < _positionScaleSpace.size(); i++)
	{
		float x = cvGetReal2D(meanWindows, 0, 0);
		float y = cvGetReal2D(meanWindows, 1, 0);
		float scale = cvGetReal2D(meanWindows, 2, 0);

		cvSetReal2D(meanWindows, 0, 0, x + cvGetReal2D(_positionScaleSpace[i], 0, 0));
		cvSetReal2D(meanWindows, 1, 0, y + cvGetReal2D(_positionScaleSpace[i], 1, 0));
		cvSetReal2D(meanWindows, 2, 0, scale + cvGetReal2D(_positionScaleSpace[i], 2, 0));
	}

	// Tinh trung binh cua cua so
	cvSetReal2D(meanWindows, 0, 0, cvGetReal2D(meanWindows, 0, 0) / _positionScaleSpace.size());
	cvSetReal2D(meanWindows, 1, 0, cvGetReal2D(meanWindows, 1, 0) / _positionScaleSpace.size());
	cvSetReal2D(meanWindows, 2, 0, cvGetReal2D(meanWindows, 2, 0) / _positionScaleSpace.size());

	// Tinh ma tran Hi = (yi - y(mean)) * (yi - y(mean))^T
	CvMat* standardDeviation = cvCreateMat(3, 1, CV_32FC1);	// Vector cot 3x1
	CvMat* standardDeviationTranspose = cvCreateMat(1, 3, CV_32FC1);	// Vector dong 1x3
	for (int i = 0; i < _positionScaleSpace.size(); i++)
	{
		_CovarianceMatrixH[i] = cvCreateMat(3, 3, CV_32FC1);

		// Tinh (yi - y(mean))
		cvSub(_positionScaleSpace[i], meanWindows, standardDeviation, 0);

		// Tao ma tran chuyen vi cua (yi - y(mean))
		cvTranspose(standardDeviation, standardDeviationTranspose);

		float x = cvGetReal2D(standardDeviation, 0, 0);
		float y = cvGetReal2D(standardDeviation, 1, 0);
		float scale = cvGetReal2D(standardDeviation, 2, 0);

		// Tinh ma tran Hi
		cvGEMM(standardDeviation, standardDeviationTranspose, 1, NULL, 0, _CovarianceMatrixH[i], 0);

		// Tinh phan Uncertain Matrix Hi bang cach thay duong cheo cua Hi bang (exp(si) * sigmaX)^2, (exp(si) * sigmaY)^2, (sigmaS)^2
		// Viec lam nay giup cho ma tran Hi kha nghich

		float si = cvGetReal2D(_positionScaleSpace[i], 2, 0);
		cvSetReal2D(_CovarianceMatrixH[i], 0, 0, (exp(si) * _sigmaX) * (exp(si) * _sigmaX));
		cvSetReal2D(_CovarianceMatrixH[i], 1, 1, (exp(si) * _sigmaY) * (exp(si) * _sigmaY));
		cvSetReal2D(_CovarianceMatrixH[i], 2, 2, _sigmaS * _sigmaS);
	}

	// Release vung nho	
	cvReleaseData(&meanWindows);
	cvReleaseData(&standardDeviation);
	cvReleaseData(&standardDeviationTranspose);
}

float Localisation::calculateMahalanobisDistance(CvMat* y, CvMat* yi, CvMat* Hi)
{
	// distance = (y - yi)^T * Hi^-1 * (y - yi)
	// y: 3x1
	// y - yi : 3x1
	// Hi: 3x3
	float distance = 0;
	CvMat* invertible_Hi = cvCreateMat(3, 3, CV_32FC1);	// Ma tran Hi^-1

	CvMat* firstResult = cvCreateMat(1, 3, CV_32FC1);	// Ma tran ket qua dau tien: (y - yi)^T * Hi^-1
	CvMat* finalResult = cvCreateMat(1, 1, CV_32FC1);	// Ma tran ket qua cuoi cung (y - yi)^T * Hi^-1 * (y - yi)

	CvMat* standardDeviation = cvCreateMat(3, 1, CV_32FC1);	// Vector cot 3x1
	CvMat* standardDeviationTranspose = cvCreateMat(1, 3, CV_32FC1);	// Vector dong 1x3

																	// Tinh y - yi (3x1)
	cvSub(y, yi, standardDeviation, 0);

	// Tinh (y - yi)^T (1x3)
	cvTranspose(standardDeviation, standardDeviationTranspose);

	// Tinh ma tran Hi^-1 (3x3)
	cvInvert(Hi, invertible_Hi, 0);

	// Tinh firstResult = (y - yi)^T * Hi^-1 (1x3)
	cvGEMM(standardDeviationTranspose, invertible_Hi, 1, NULL, 0, firstResult, 0);

	// Tinh ket qua cuoi cung (1x1)
	cvGEMM(firstResult, standardDeviation, 1, NULL, 0, finalResult, 0);

	distance = (float)cvGetReal2D(finalResult, 0, 0);

	// Release vung nho
	cvReleaseData(&invertible_Hi);
	cvReleaseData(&firstResult);
	cvReleaseData(&finalResult);
	cvReleaseData(&standardDeviation);
	cvReleaseData(&standardDeviationTranspose);

	return distance;
}

float Localisation::clipWiFunction(float wi, float threshold_c, float a, std::string clippingType)
{
	float clippedWi = 0;

	if (clippingType.compare("hard") == 0)	// hard function
	{
		if (wi < threshold_c)	// linear SVM c = 0
		{
			clippedWi = 0;
		}
		else
		{
			clippedWi = wi - threshold_c;
		}
	}
	else if (clippingType.compare("soft") == 0)
	{
		// Paper su dung a = 10, c = 0;
		clippedWi = (1 / a) * log(1 + exp(a * (wi + threshold_c)));
	}
	else if (clippingType.compare("sigmoid") == 0)
	{
		// Paper su dung a = -4.1, c = -0.029
		clippedWi = 1 / (1 + exp(a * (wi + threshold_c)));
	}

	return clippedWi;
}

float Localisation::calculatePowerOfDeterminant(CvMat* matrix, float power)
{
	float detPow;
	float det = (float)cvDet(matrix);

	if (det < 0)
	{
		return 0;
	}

	detPow = pow(det, power);

	return detPow;
}

float Localisation::calculateVariantPi(int index_yi, int index_y)
{
	float variantPi = 0;

	float determinant_Hi_pow;
	std::string clipType = "hard";
	float threshold_c = 0;
	float coefficient_a = 0;

	float clippedWi;

	float MahalanobisDistance;
	float exp_MahalanobisDistance;

	float numerator = 0;	// Tu so cua phep chia de tinh Wi (Variant Pi)
	float denominator = 0;	// Mau so cua phep chia de tinh Wi
	float temp = 0;

	for (int i = 0; i < _positionScaleSpace.size(); i++)
	{
		// Khoi gian temp
		temp = 0;

		determinant_Hi_pow = calculatePowerOfDeterminant(_CovarianceMatrixH[i], -0.5);	// Tinh gia tri dinh thuc |Hi|^-1/2

		clipType = "hard";
		threshold_c = 0;
		coefficient_a = 0;

		clippedWi = clipWiFunction(_weightOfWindows[i], threshold_c, coefficient_a, clipType);	// Chuyen trong so wi sang positive

		MahalanobisDistance = calculateMahalanobisDistance(_positionScaleSpace[index_y], _positionScaleSpace[i], _CovarianceMatrixH[i]);	// tinh Mahalanobis
		exp_MahalanobisDistance = exp(-1.0 * MahalanobisDistance / 2);

		temp = determinant_Hi_pow * clippedWi * exp_MahalanobisDistance;

		if (index_yi == i)
		{
			numerator = temp;	// Tinh tu so khi i == index_yi
		}

		denominator += temp;
	}

	variantPi = numerator / denominator;

	return variantPi;
}

CvMat* Localisation::calculateDataWeightedHarmonicMean(int index_y)
{
	CvMat* harmonicMean = cvCreateMat(3, 3, CV_32FC1);
	CvMat* invertible_Hi = cvCreateMat(3, 3, CV_32FC1);

	CvMat *temp = cvCreateMat(3, 3, CV_32FC1);

	// Khoi gan harmonic mean
	cvSetZero(harmonicMean);

	// H_h^-1(y) = sum(variantPi_i(y) * H_i^-1)
	float variantPi_i;
	for (int i = 0; i < _positionScaleSpace.size(); i++)
	{
		variantPi_i = calculateVariantPi(i, index_y);

		// Tinh Hi^-1 tu covariance matrix Hi
		cvInvert(_CovarianceMatrixH[i], invertible_Hi, 0);

		// Tinh temp = variantPi_i(y) * H_i^-1
		for (int r = 0; r < invertible_Hi->height; r++)
		{
			for (int c = 0; c < invertible_Hi->width; c++)
			{
				cvSetReal2D(temp, r, c, cvGetReal2D(invertible_Hi, r, c) * variantPi_i);
			}
		}

		// Sau do tinh cong temp vao trong harmonic mean matrix
		cvAdd(harmonicMean, temp, harmonicMean, 0);
	}

	// Release vung nho
	cvReleaseData(&temp);
	cvReleaseData(&invertible_Hi);

	return harmonicMean;
}

CvMat* Localisation::computeMeanShiftWindow(int index_y)
{
	CvMat* meanShiftWindow = cvCreateMat(3, 1, CV_32FC1);

	CvMat* invertible_HarmonicMean = cvCreateMat(3, 3, CV_32FC1);
	CvMat* invertible_Hi = cvCreateMat(3, 3, CV_32FC1);
	CvMat* Hi_multiply_yi = cvCreateMat(3, 1, CV_32FC1);
	CvMat* temp = cvCreateMat(3, 1, CV_32FC1);
	CvMat* sum_temp = cvCreateMat(3, 1, CV_32FC1);

	CvMat* harmonicMean = calculateDataWeightedHarmonicMean(index_y);

	// Khoi tao cac gia tri ban dau
	cvSetZero(sum_temp);

	// Tinh H_h(y)
	cvInvert(harmonicMean, invertible_HarmonicMean);

	// Tinh phan sum(variant_Pi * Hi^-1 * yi)
	float variantPi_i;
	for (int i = 0; i < _positionScaleSpace.size(); i++)
	{
		variantPi_i = calculateVariantPi(i, index_y);
		cvInvert(_CovarianceMatrixH[i], invertible_Hi, 0);

		// Tinh Hi^-1 * yi
		cvGEMM(invertible_Hi, _positionScaleSpace[i], 1, NULL, 0, Hi_multiply_yi, 0);

		// Tinh variant_Pi * Hi^-1 * yi
		for (int k = 0; k < Hi_multiply_yi->height; k++)
		{
			cvSetReal2D(temp, k, 0, cvGetReal2D(Hi_multiply_yi, k, 0) * variantPi_i);
		}

		// Tinh sum(variant_Pi * Hi^-1 * yi)
		cvAdd(sum_temp, temp, sum_temp, 0);
	}

	// Tinh cua so meanshift
	cvGEMM(invertible_HarmonicMean, sum_temp, 1, NULL, 0, meanShiftWindow, 0);

	// Release vung nho
	cvReleaseData(&invertible_HarmonicMean);
	cvReleaseData(&harmonicMean);
	cvReleaseData(&invertible_Hi);
	cvReleaseData(&Hi_multiply_yi);
	cvReleaseData(&temp);
	cvReleaseData(&sum_temp);

	return meanShiftWindow;
}

void Localisation::increasePositionScaleSpace(int increaseSize)
{
	_positionScaleSpace.resize(_positionScaleSpace.size() + increaseSize);
	_CovarianceMatrixH.resize(_CovarianceMatrixH.size() + increaseSize);
}

void Localisation::addASampleToSpace(CvMat* newSample)
{
	_positionScaleSpace.push_back(newSample);
}

void Localisation::updateCovarianceMatrix()
{
	_CovarianceMatrixH.resize(_positionScaleSpace.size());
	calculateCovarianceMatrixH();
}

void Localisation::localize()
{
	CvMat* currentMeanShiftWindow = cvCreateMat(3, 1, CV_32FC1);
	// Xay dung khong gian 3D position scale
	constructPositionScaleSpace();

	// Tinh ma tran covariance
	calculateCovarianceMatrixH();

	float difference = 0;
	float epsilon = 0.005;

	do
	{
		// Tinh cua so mean shift hien tai
		currentMeanShiftWindow = computeMeanShiftWindow(_positionScaleSpace.size() - 1);

		int x1 = (int)cvGetReal2D(currentMeanShiftWindow, 0, 0);
		int y1 = (int)cvGetReal2D(currentMeanShiftWindow, 1, 0);
		float scale1 = (float)cvGetReal2D(currentMeanShiftWindow, 2, 0);


		// Tinh su sai biet giua 2 cua so mean shift o lan tinh nay va lan tinh truoc		
		if (cvGetReal2D(_previousMeanShiftWindow, 0, 0) == -1)	// Neu day la lan dau tien tinh cua so meanshift
		{														// thi luu lai cua so nay de so sanh cho nhung cua so mean shift lan tinh sau
			_previousMeanShiftWindow = currentMeanShiftWindow;
		}
		else
		{
			// Neu khong phai lan dau tien tinh cua so mean shift thi tinh su sai biet giua 2 cua so mean shift
			float xDiff = (float)cvGetReal2D(currentMeanShiftWindow, 0, 0) - (float)cvGetReal2D(_previousMeanShiftWindow, 0, 0);
			xDiff *= xDiff;
			float yDiff = (float)cvGetReal2D(currentMeanShiftWindow, 1, 0) - (float)cvGetReal2D(_previousMeanShiftWindow, 1, 0);
			yDiff *= yDiff;
			float scaleDiff = (float)cvGetReal2D(currentMeanShiftWindow, 2, 0) - (float)cvGetReal2D(_previousMeanShiftWindow, 2, 0);
			scaleDiff *= scaleDiff;

			difference = sqrt((float)xDiff + yDiff + scaleDiff);

			// Neu su khac biet nho hon epsilon thi gan ket qua cuoi cung roi thoat
			if (difference <= epsilon)
			{
				_finalLocalisationWindow = cvCloneMat(currentMeanShiftWindow);
				return;
			}
			else
			{
				_previousMeanShiftWindow = currentMeanShiftWindow;
			}
		}

		// Bo cua so nay vao SVM de lay ra lai Weight
		int x = (int)cvGetReal2D(currentMeanShiftWindow, 0, 0);
		int y = (int)cvGetReal2D(currentMeanShiftWindow, 1, 0);
		float scale = (float)cvGetReal2D(currentMeanShiftWindow, 2, 0);

		// Resize lai test Image theo dung muc scale
		scale = exp((float)scale);

		// Tinh dac trung cho cua so mean shift
		int index = -1;
		IplImage** integralsInput = (IplImage**)malloc(9 * sizeof(IplImage*));

		for (int i = 0; i < _integralScale.size(); i++)
		{
			if (scale == _integralScale[i]->_scale)
			{
				index = i;
				break;
			}
		}

		if (index >= 0)
		{
			for (int i = 0; i < 9; i++)
			{
				cvSetImageROI(_integralScale[index]->_integral[i], cvRect(x, y, 64, 128));
				integralsInput[i] = cvCreateImage(cvGetSize(_integralScale[index]->_integral[i]), _integralScale[index]->_integral[i]->depth, _integralScale[index]->_integral[i]->nChannels);
				cvCopy(_integralScale[index]->_integral[i], integralsInput[i], NULL);
			}
		}

		// Lay ra lai trong so sau khi test bang SVM
		//float weight = _hogClassifier.testSVM(_trainPath, img, NULL);

		float weight = _hogClassifier->testSVM(_trainPath, integralsInput, NULL);

		// Release vung nho
		/*cvResetImageROI(rescaledImage);
		cvReleaseImage(&rescaledImage);
		cvReleaseImage(&img);*/

		for (int i = 0; i < 9; i++)
		{
			cvResetImageROI(_integralScale[index]->_integral[i]);
		}
		for (int i = 0; i < 9; i++)
		{
			cvReleaseImage(&integralsInput[i]);
		}

		free(integralsInput);

		// Sau khi lay duoc trong so, ta tao ra sample moi va them vao khong gian Position Scale

		addASampleToSpace(currentMeanShiftWindow);

		_weightOfWindows.push_back(weight);

		updateCovarianceMatrix();	// Cap nhat lai ma tran covariance
	} while (true);
}

void Localisation::localize2(int size)
{
	int x, y;
	float scale;
	x = y = 0;
	scale = 0;
	for (int i = 0; i < size; i++)
	{
		x += _detectedWindows2[i]->getXCoordinate() * _detectedWindows2[i]->getScale();
		y += _detectedWindows2[i]->getYCoordinate() * _detectedWindows2[i]->getScale();
		scale += _detectedWindows2[i]->getScale();
	}

	x = (int)x / size;
	y = (int)y / size;
	scale = (float)scale / size;

	_finalLocalisationWindow = cvCreateMat(3, 1, CV_32FC1);

	cvSetReal2D(_finalLocalisationWindow, 0, 0, x);
	cvSetReal2D(_finalLocalisationWindow, 1, 0, y);
	cvSetReal2D(_finalLocalisationWindow, 2, 0, scale);
}
