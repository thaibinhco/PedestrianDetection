#pragma once

#include "cxcore.h"

class LineSegment
{
public:
	float _a;
	float _b;
	CvPoint _begin;
	CvPoint _end;

	LineSegment(void);
	LineSegment(CvPoint p1, CvPoint p2);

	void calculateCoEfficient();
	CvPoint getIntersection(LineSegment *l1, LineSegment *l2);

	~LineSegment(void);
};
