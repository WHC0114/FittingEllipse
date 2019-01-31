#include "opencv2/opencv.hpp"

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
float Compute(Point NowPoint, float x, EllipsePara EP, Mat& mask);
int main()
{
	Mat src = imread("E:/pictures/ellipse.png");
	cout << src.size() << endl;
	Mat srcGray;
	cvtColor(src, srcGray, COLOR_BGR2GRAY);
	threshold(srcGray, srcGray, 20, 255, THRESH_BINARY_INV);
	vector<vector<Point>> contours;
	//Mat dst;
	//Canny(srcGray, dst, 30, 70);

	findContours(srcGray, contours, RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
	Mat cimage = Mat::zeros(src.size(), CV_8UC3);
	Mat mask = Mat::zeros(src.size(), CV_8UC1);
	vector<Point> contours2;
	for (int i = 0; i < contours[0].size(); i++)
	{
		contours2.push_back(contours[0][i]);
		circle(cimage, contours[0][i], 2, Scalar(0, 255, 255), 2);
	}

	//cout << contours2[1]<< endl;


	size_t count = contours2.size();
	//if (count < 6)
	//	continue;
	RotatedRect box = fitEllipse(contours2);

	EllipsePara EP;
	getEllipsePara(box, EP);

	drawContours(cimage, contours, -1, Scalar::all(255), 2);
	ellipse(mask, box, Scalar::all(255), 1);
	Mat Dom;
	maxminPoint(mask, Dom);
	Mat colorMask = Mat::zeros(src.size(), CV_8UC3);
	for (int i = 0; i < contours[0].size(); i++)
	{
		circle(colorMask, contours[0][i], 2, Scalar(0, 0, 255), 2);
		float x;
		int	flag = ExchangeVar(contours[0][i], x, Dom);
		if (flag == 0)
			continue;
		float y = Compute(contours[0][i], x, EP);
		circle(colorMask, Point(x, y), 2, Scalar(0, 255, 0), 2);
		imshow("colorMask", colorMask);
		colorMask = Mat::zeros(src.size(), CV_8UC3);
		waitKey(20);
	}

	//cout << "OUT Y:" << y << endl;
	//circle(src, Point(300, y), 2, Scalar(0, 255, 0), 2);

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

float Compute(Point NowPoint, float x, EllipsePara EP)
{
	float VarX = x - EP.center.x;
	float y = -(EP.b*VarX - 2 * EP.c*EP.center.y + sqrt((pow(EP.b, 2)*pow(VarX, 2) - 4 * EP.a*EP.c*pow(VarX, 2) - 4 * EP.c*EP.f))) / (2 * EP.c);
	float y2 = (-EP.b*VarX + 2 * EP.c*EP.center.y + sqrt((pow(EP.b, 2)*pow(VarX, 2) - 4 * EP.a*EP.c*pow(VarX, 2) - 4 * EP.c*EP.f))) / (2 * EP.c);
	float crossCenter1 = (NowPoint.x - EP.center.x)*(y - EP.center.y) - (x - EP.center.x) * (NowPoint.y - EP.center.y);
	float crossCenter2 = (NowPoint.x - EP.center.x)*(y2 - EP.center.y) - (x - EP.center.x) * (NowPoint.y - EP.center.y);

	/*if (crossCenter1 < 0 && crossCenter2 < 0)
	{
	if (distance(NowPoint, Point(x, y)) < distance(NowPoint, Point(x, y2)))
	return y;
	else
	return y2;
	}
	else if (crossCenter1 > 0 && crossCenter2 < 0)
	return y2;
	else
	return NowPoint.y;*/
	if (distance(NowPoint, Point(x, y)) < distance(NowPoint, Point(x, y2)))
		return y;
	else
		return y2;
}

void maxminPoint(Mat& mask, Mat& Dom)
{
	Rect rect = boundingRect(mask);
	Dom = (Mat_<int>(2, 2) <<
		rect.tl().x, rect.br().x,
		rect.tl().y, rect.br().y
		);
	//cout << rect << endl;
	//circle(mask, rect.tl(), 2, Scalar::all(255), 3);
	//rectangle(mask, rect, Scalar::all(255));
}


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


	//cout << VarX << endl;
	x = VarX + NewPoint.x;

	if (x <= Dom.at<int>(0, 0))
	{
		x = Dom.at<int>(0, 0) + abs(Dom.at<int>(0, 0) - x);
	}

	if (x >= Dom.at<int>(0, 1))
	{
		x = Dom.at<int>(0, 1) - abs(Dom.at<int>(0, 1) - x);
	}
	//cout << x << endl;
}

double distance(Point point1, Point point2)
{
	double distance;
	distance = powf((point1.x - point2.x), 2) + powf((point1.y - point2.y), 2);
	return distance;
}

float Compute(Point NowPoint, float x, EllipsePara EP, Mat& mask)
{
	float VarX = x - EP.center.x;
	float y = -(EP.b*VarX - 2 * EP.c*EP.center.y + sqrt((pow(EP.b, 2)*pow(VarX, 2) - 4 * EP.a*EP.c*pow(VarX, 2) - 4 * EP.c*EP.f))) / (2 * EP.c);
	float y2 = (-EP.b*VarX + 2 * EP.c*EP.center.y + sqrt((pow(EP.b, 2)*pow(VarX, 2) - 4 * EP.a*EP.c*pow(VarX, 2) - 4 * EP.c*EP.f))) / (2 * EP.c);
	float crossCenter = (NowPoint.x - EP.center.x)*(y - EP.center.y) - (x - EP.center.x) * (NowPoint.y - EP.center.y);
	circle(mask, Point(x, y), 2, Scalar(255, 0, 0), 2);
	circle(mask, Point(x, y2), 2, Scalar(0, 255, 0), 2);
	circle(mask, NowPoint, 2, Scalar(0, 0, 255), 2);
	cout << crossCenter << endl;
	if (crossCenter < 0)
		return y;
	else
		return y2;
}
