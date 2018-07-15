#pragma once

#include <vector>

class ScanWindow
{
private:
	float _xCoordinate;
	float _yCoordinate;
	float _width;
	float _height;
	float _scale;
	float _weight;

public:
	ScanWindow(void);
	ScanWindow(float x, float y, float scale);
	ScanWindow(float x, float y, float scale, float width, float height);

	float getXCoordinate();
	float getYCoordinate();
	std::vector<float> getPosition();

	float getWidth();
	float getHeight();

	float getScale();
	float getWeight();

	void setXCoordinate(float xValue);
	void setYCoordinate(float yValue);
	void setPosition(float xValue, float yValue);

	void setWidth(float w);
	void setHeight(float h);

	void setScale(float sValue);
	void setWeight(float wValue);

	~ScanWindow(void);
};
