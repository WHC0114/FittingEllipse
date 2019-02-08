#include "opencv2/opencv.hpp"
#include "FittingEllipse.h"
using namespace cv;
using namespace std;


struct EllipsePara
{
	Point2f center;
	float theta;
	float a;
	float b;
	float c;
	float f;
};
void getEllipsePara(RotatedRect &ellipsemege, EllipsePara& EP);
float Compute(Point, float x, EllipsePara EP);
void maxminPoint(Mat& mask, Mat& Dom);
int ExchangeVar(Point UpdatePoint, float& x, Mat Dom);
double distance(Point point1, Point point2);

int thresholdValue = 0;       //model selection

int main()
{
	Mat src = imread("E:/pictures/ellipse2.png");
	cout << src.size() << endl;
	Mat srcGray;
	cvtColor(src, srcGray, COLOR_BGR2GRAY);
	threshold(srcGray, srcGray, 20, 255, THRESH_BINARY_INV);
	vector<vector<Point>> contours;

	findContours(srcGray, contours, RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
	Mat cimage = Mat::zeros(src.size(), CV_8UC3);
	Mat mask = Mat::zeros(src.size(), CV_8UC1);
	vector<Point> contours2;
	for (int i = 0; i < contours[0].size(); i++)
	{
		contours2.push_back(contours[0][i]);
		circle(cimage, contours[0][i], 2, Scalar(0, 255, 255), 2);
	}

	size_t count = contours2.size();
	RotatedRect box = fitEllipse(contours2);

	EllipsePara EP;
	getEllipsePara(box, EP);

	drawContours(cimage, contours, -1, Scalar::all(255), 2);
	ellipse(mask, box, Scalar::all(255), 1);
	Mat Dom;
	maxminPoint(mask, Dom);
	cout << "Dom :" << Dom << endl;
	Mat colorMask = Mat::zeros(src.size(), CV_8UC3);
	FittingEllipse fe(src, contours[0]);
	for (int i = 0; i < contours[0].size(); i++)
	{
		circle(colorMask, contours[0][i], 2, Scalar(0, 0, 255), 2);
		float x;
		int	flag = ExchangeVar(contours[0][i], x, Dom);
		if (flag == 0)
			continue;
		fe.apply(contours2[i]);
		float y = Compute(contours[0][i], x, EP);
		circle(colorMask, Point(x, y), 2, Scalar(0, 255, 0), 2);
		imshow("colorMask", colorMask);
		colorMask = Mat::zeros(src.size(), CV_8UC3);
		waitKey(20);
	}

	Point2f points[4];
	box.points(points);
	for (int i = 0; i < 4; i++)
	{
		line(cimage, points[i], points[(i + 1) % 4], Scalar(255, 0, 0), 2);
	}
	cout << box.angle << endl;

	imshow("src", src);
	imshow("mask", mask);
	imshow("cimage", cimage);
	waitKey(0);
	return 0;
}

//**************calculation of ellipse parameters*************
void getEllipsePara(RotatedRect &ellipsemege, EllipsePara& EP)
{
	EP.theta = ellipsemege.angle * CV_PI / 180;
	//cout << theta << endl;
	float a = ellipsemege.size.width / 2.0;
	float b = ellipsemege.size.height / 2.0;


	EP.center.x = ellipsemege.center.x;
	EP.center.y = ellipsemege.center.y;

	EP.a = a*a*sin(EP.theta)*sin(EP.theta) + b*b*cos(EP.theta)*cos(EP.theta);
	EP.b = (-2.0)*(a*a - b*b)*sin(EP.theta)*cos(EP.theta);
	EP.c = a*a*cos(EP.theta)*cos(EP.theta) + b*b*sin(EP.theta)*sin(EP.theta);
	EP.f = (-1.0)*a*a*b*b;
	cout << "A:" << EP.a << endl;
	cout << "B:" << EP.b << endl;
	cout << "C:" << EP.c << endl;
	cout << "F:" << EP.f << endl;
	cout << "center:" << EP.center << endl;
}


//calculation y value and select correct y value
float Compute(Point NowPoint, float x, EllipsePara EP)
{
	float VarX = cvCeil(x - EP.center.x);
	float y = -(EP.b*VarX - 2 * EP.c*EP.center.y + sqrt((pow(EP.b, 2)*pow(VarX, 2) - 4 * EP.a*EP.c*pow(VarX, 2) - 4 * EP.c*EP.f))) / (2 * EP.c);
	float y2 = (-EP.b*VarX + 2 * EP.c*EP.center.y + sqrt((pow(EP.b, 2)*pow(VarX, 2) - 4 * EP.a*EP.c*pow(VarX, 2) - 4 * EP.c*EP.f))) / (2 * EP.c);
	/*float crossCenter1 = (NowPoint.x - EP.center.x)*(y - EP.center.y) - (x - EP.center.x) * (NowPoint.y - EP.center.y);
	float crossCenter2 = (NowPoint.x - EP.center.x)*(y2 - EP.center.y) - (x - EP.center.x) * (NowPoint.y - EP.center.y);*/

	//calculation the distance between (x,y) and (x, y2);
	double distanceXY = distance(NowPoint, Point(x, y));
	double distanceXY2 = distance(NowPoint, Point(x, y2));


	//there are three options
	switch (thresholdValue)
	{
	case 0:             //if x is not less than the minimum
	{
		if (distanceXY < distanceXY2)   //we pick the closest point
			return y;
		else
			return y2;
	}
	case 1:             //if x is close to the minimum
		return y2;
	case 2:
		return y;       //if x is close to the maximum
	default:
		break;
	}
}

//Place the bounding value in a container of type Mat 
void maxminPoint(Mat& mask, Mat& Dom)
{
	Rect rect = boundingRect(mask);
	Dom = (Mat_<int>(2, 2) <<
		rect.tl().x + 2, rect.br().x - 2,
		rect.tl().y, rect.br().y
		);
}

//To calculation the x value
int ExchangeVar(Point UpdatePoint, float& x, Mat Dom)
{
	static Point LastPoint = Point(0.0);
	static Point NewPoint = Point(0, 0);
	static int count = 0;
	if (LastPoint == Point(0, 0))
	{
		LastPoint = UpdatePoint;
		return 0;
	}
	if (LastPoint != UpdatePoint && NewPoint == Point(0, 0))
	{
		NewPoint = UpdatePoint;
	}


	if (NewPoint != UpdatePoint)
	{
		LastPoint = NewPoint;
		NewPoint = UpdatePoint;
	}

	int VarX = (NewPoint.x - LastPoint.x) * 5;
	VarX = VarX > 5 ? 5 : VarX;
	VarX = VarX < -5 ? -5 : VarX;

	if (VarX != 0)
		count = VarX;
	else
		VarX = count;

	x = VarX + NewPoint.x;

	if (x <= Dom.at<int>(0, 0))
	{
		x = Dom.at<int>(0, 0) + abs(Dom.at<int>(0, 0) - x);
		thresholdValue = 1;
	}
	else
		thresholdValue = 0;

	if (x >= Dom.at<int>(0, 1))
	{
		x = Dom.at<int>(0, 1) - abs(Dom.at<int>(0, 1) - x);
		thresholdValue = 2;
	}

}

//return distance(No sqrt)
double distance(Point point1, Point point2)
{
	double distance;
	distance = powf((point1.x - point2.x), 2) + powf((point1.y - point2.y), 2);
	return distance;
}

