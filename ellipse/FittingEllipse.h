#pragma once


#ifndef  FITTINGELLIPSE_H
#define FITTINGELLIPSE_H
#include <iostream>

#include "opencv2/opencv.hpp"

using namespace cv;
using namespace std;


class FittingEllipse
{
public:
	FittingEllipse(Mat InputImage, vector<Point> points);
	~FittingEllipse();
	bool judgeNumber();
	void apply(Point NowPoint);
	inline double distance(Point p1, Point p2);
	void getPredictY(float &y);
	Mat Dom;
private:
	void getEllipsePara();
	void minmaxPoint();
	float Compute();
	int ExchangeVarX();
	RotatedRect ellipsemege;
	vector<Point> points;
	Point NowPoint;
	Point2f center;
	double theta;
	double A;
	double B;
	double C;
	double F;
	Mat InputImage;
	int PredictX;
	int Object;
};

#endif // ! FITTINGELLIPSE_H