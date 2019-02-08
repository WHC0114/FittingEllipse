#include "opencv2/opencv.hpp"
#include "FittingEllipse.h"
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

bool FittingEllipse::judgeNumber()
{
	if (this->points.size() < 6)
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
	this->ellipsemege = fitEllipse(this->points);
	this->theta = (*this).ellipsemege.angle * 180 / CV_PI;
	float a = this->ellipsemege.size.width / 2.0;
	float b = this->ellipsemege.size.height / 2.0;

	this->center = this->ellipsemege.center;
	this->A = a*a*sin(this->theta)*sin(this->theta) + b*b*cos(this->theta)*cos(this->theta);
	this->B = (-2.0)*(a*a - b*b)*sin(this->theta)*cos(this->theta);
	this->C = a*a*cos(this->theta)*cos(this->theta) + b*b*sin(this->theta)*sin(this->theta);
	this->F = (-1.0)*a*a*b*b;
}

inline double FittingEllipse::distance(Point p1, Point p2)
{
	return powf((p1.x - p2.x), 2) + powf((p1.y - p2.y), 2);
}

float FittingEllipse::Compute()
{
	float VarX = cvCeil(this->PredictX - this->center.x);
	float y1 = -(this->B*VarX - 2 * this->B*this->center.y + sqrt((pow(this->B, 2)*pow(VarX, 2) - 4 * this->A*this->C*pow(VarX, 2) - 4 * this->C*this->F))) / (2 * this->C);
	float y2 = (-this->B*VarX + 2 * this->B*this->center.y + sqrt((pow(this->B, 2)*pow(VarX, 2) - 4 * this->A*this->C*pow(VarX, 2) - 4 * this->C*this->F))) / (2 * this->C);
	double distanceY1 = distance(this->NowPoint, Point(this->PredictX, y1));
	double distanceY2 = distance(this->NowPoint, Point(this->PredictX, y2));

	//there are three options
	switch (this->Object)
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
	Mat mask = Mat::zeros(this->InputImage.size(), CV_8UC1);
	ellipse(mask, this->ellipsemege, Scalar::all(255), 1);
	Rect rect = boundingRect(mask);
	this->Dom = (Mat_<int>(2, 2) <<
		rect.tl().x + 2, rect.br().x - 2,
		rect.tl().y, rect.br().y
		);
}

FittingEllipse::FittingEllipse(Mat InputImage, vector<Point> points)
{
	this->points = points;
	this->InputImage = InputImage;
	this->getEllipsePara();
	this->minmaxPoint();
};

int FittingEllipse::ExchangeVarX()
{
	static Point LastPoint = Point(0, 0);
	static Point NewPoint = Point(0, 0);
	static int count = 0;
	if (LastPoint == Point(0, 0))
	{
		LastPoint = this->NowPoint;
		return 0;
	}
	if (LastPoint != this->NowPoint && NewPoint == Point(0, 0))
	{
		NewPoint = this->NowPoint;
	}


	if (NewPoint != this->NowPoint)
	{
		LastPoint = NewPoint;
		NewPoint = this->NowPoint;
	}

	int VarX = NewPoint.x - LastPoint.x;

	if (VarX != 0)
		count = VarX;
	else
		VarX = count;

	VarX = (VarX / abs(VarX)) * 8;
	this->PredictX = VarX + this->NowPoint.x;

	if (this->PredictX <= Dom.at<int>(0, 0))
	{
		this->PredictX = Dom.at<int>(0, 0) + abs(Dom.at<int>(0, 0) - this->PredictX);
		this->Object = 1;
	}
	else
		this->Object = 0;

	if (this->PredictX >= Dom.at<int>(0, 1))
	{
		this->PredictX = Dom.at<int>(0, 1) - abs(Dom.at<int>(0, 1) - this->PredictX);
		this->Object = 2;
	}

}

void FittingEllipse::apply(Point NowPoint)
{
	this->NowPoint = NowPoint;
}
void FittingEllipse::getPredictY(float &y)
{
	this->ExchangeVarX();
	y = this->Compute();
}


FittingEllipse::~FittingEllipse()
{
}
