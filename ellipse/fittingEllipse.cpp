#include <iostream>
#include "opencv2/opencv.hpp"
#include "FittingEllipse.h"
using namespace std;
using namespace cv;



bool FittingEllipse::judgeNumber()
{
	if (points.size() < 6)
	{
		cout << "Insufficient target point" << endl;
		return false;
	}
	else
		return true;
}

void FittingEllipse::getEllipsePara()
{
	if (!judgeNumber())
		exit(0);
	ellipsemege = fitEllipse(points);
	theta = ellipsemege.angle * CV_PI / 180;
	float a = ellipsemege.size.width / 2.0;
	float b = ellipsemege.size.height / 2.0;

	center = ellipsemege.center;
	A = a*a*sin(theta)*sin(theta) + b*b*cos(theta)*cos(theta);
	B = (-2.0)*(a*a - b*b)*sin(theta)*cos(theta);
	C = a*a*cos(theta)*cos(theta) + b*b*sin(theta)*sin(theta);
	F = (-1.0)*a*a*b*b;
	
	cout << "A:"  << "\t"<< A << endl;
	cout << "B:" << "\t" << B << endl;
	cout << "C:" << "\t" << C << endl;
	cout << "F:" << "\t" << F << endl;
	cout << "center"<< "\t" << ellipsemege.center<< endl;
}

inline double FittingEllipse::distance(Point p1, Point p2)
{
	return powf((p1.x - p2.x), 2) + powf((p1.y - p2.y), 2);
}

float FittingEllipse::Compute()
{
	float VarX = cvCeil(PredictX - center.x);
	float y1 = -(B*VarX - 2 * C*center.y + sqrt((pow(B, 2)*pow(VarX, 2) - 4 * A*C*pow(VarX, 2) - 4 * C*F))) / (2 * C);
	float y2 = (-B*VarX + 2 * C*center.y + sqrt((pow(B, 2)*pow(VarX, 2) - 4 * A*C*pow(VarX, 2) - 4 * C*F))) / (2 * C);
	double distanceY1 = distance(NowPoint, Point(PredictX, y1));
	double distanceY2 = distance(NowPoint, Point(PredictX, y2));

	//there are three options
	switch (Object)
	{
	case 0:             //if x is not less than the minimum
	{
		if (distanceY1 < distanceY2)   //we pick the closest point
			return y1;
		else
			return y2;
	}
	case 1:             //if x is close to the minimum
		return y2;
	case 2:
		return y1;       //if x is close to the maximum
	default:
		break;
	}
}

void FittingEllipse::minmaxPoint()
{
	Mat mask = Mat::zeros(InputImage.size(), CV_8UC1);
	ellipse(mask, ellipsemege, Scalar::all(255), 1);
	Rect rect = boundingRect(mask);
	Dom = (Mat_<int>(2, 2) <<
		rect.tl().x + 2, rect.br().x - 2,
		rect.tl().y, rect.br().y
		);
}

FittingEllipse::FittingEllipse(Mat InputImage, vector<Point> points)
{
	this->points = points;
	this->InputImage = InputImage;
	getEllipsePara();
	minmaxPoint();
};

int FittingEllipse::ExchangeVarX()
{
	static Point LastPoint = Point(0, 0);
	static Point NewPoint = Point(0, 0);
	static int count = 0;
	if (LastPoint == Point(0, 0))
	{
		LastPoint = NowPoint;
		return 0; 
	}
	if (LastPoint != NowPoint && NewPoint == Point(0, 0))
	{
		NewPoint = NowPoint;
	}


	if (NewPoint != NowPoint)
	{
		LastPoint = NewPoint;
		NewPoint = NowPoint;
	}

	int VarX = NewPoint.x - LastPoint.x;

	if (VarX != 0)
		count = VarX;
	else
		VarX = count;

	VarX = (VarX / abs(VarX)) * 8;
	PredictX = VarX + NowPoint.x;

	if (PredictX <= Dom.at<int>(0, 0))
	{
		PredictX = Dom.at<int>(0, 0) + abs(Dom.at<int>(0, 0) - PredictX);
		Object = 1;
	}
	else
		Object = 0;

	if (PredictX >= Dom.at<int>(0, 1))
	{
		PredictX = Dom.at<int>(0, 1) - abs(Dom.at<int>(0, 1) - PredictX);
		Object = 2;
	}

}

void FittingEllipse::apply(Point NowPoint)
{
	this->NowPoint = NowPoint;
}
void FittingEllipse::getPredictY(float &y)
{
	ExchangeVarX();
	y = Compute();
}


FittingEllipse::~FittingEllipse()
{
	;
}
