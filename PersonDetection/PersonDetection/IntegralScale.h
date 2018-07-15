#pragma once

#include "cxcore.h"

class IntegralScale
{
public:
	IplImage** _integral;
	float _scale;

	IntegralScale(void);
	IntegralScale(IplImage** integral, float scale);

	~IntegralScale(void);
};
