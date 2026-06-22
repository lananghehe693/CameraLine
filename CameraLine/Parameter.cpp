#include "Parameter.h"
//#include <ppl.h>
//using namespace concurrency;

#pragma region Class Parameter

Parameter::Parameter() {

}

void Parameter::PutText(string str) {
	cv::putText(TextImage, str, cv::Point(0, 18 + 18 * textcount), cv::FONT_HERSHEY_TRIPLEX, 0.5, 0, 1);
	textcount++;
}

void Parameter::PutText(stringstream& ss) {
	string str;

	while (getline(ss, str, '\n')) {
		PutText(str);
	}

	textcount = 0;
	ss.str("");
	ss.clear(stringstream::goodbit);
}

#pragma endregion

#pragma region SampleInspection

string SampleInspection::FilePath;
string SampleInspection::FileName;
string SampleInspection::WriteFilePath;
int SampleInspection::SampleNo;
int SampleInspection::N;

SampleInspection::SampleInspection() {

	pPara = PParameter(new Parameter());

}

bool SampleInspection::ReadFile() {

	string extension = FileName.substr(FileName.size() - 3, 3);

	if (extension == "png" || extension == "bmp") {

		InputImage = cv::imread(FilePath + FileName, cv::IMREAD_COLOR);
		if (InputImage.empty()) {
			return false;
		}

	} else if (extension == "avi") {

		Capture = cv::VideoCapture(FilePath + FileName);
		if (!Capture.isOpened()) {
			return false;
		}

	} else {
		cout << "extension is not [jpeg, bmp, avi]" << endl;
		return false;
	}

	return true;
}

void SampleInspection::SetFilePath(string Path, string Name) {
	FilePath = Path;
	FileName = Name;
}

void SampleInspection::StartInspection() {

}

#pragma endregion

#pragma region Line

cvex::Line::Line() {
}

cvex::Line::Line(double slope, double intercept, int slope_type)
	:intercept(intercept) {
	switch (slope_type) {

	case GRADIENT:
		this->slope = slope;
		break;

	case DEGREE:
		this->slope = tan(slope / 180 * M_PI);
		break;

	case RADIAN:
		this->slope = tan(slope);
		break;
	}
}

cvex::Line::Line(double slope, cv::Point2d p, int slope_type) {
	switch (slope_type) {

	case GRADIENT:
		this->slope = slope;
		break;

	case DEGREE:
		this->slope = tan(slope / 180 * M_PI);
		break;

	case RADIAN:
		this->slope = tan(slope);
		break;
	}

	this->intercept = p.y - this->slope * p.x;
}

cvex::Line::Line(cv::Point2d p1, cv::Point2d p2) {
	if (abs(p1.x - p2.x) < 1e-6) this->slope = 1e6;
	else this->slope = (p1.y - p2.y) / ((p1.x - p2.x));

	this->intercept = p1.y - this->slope * p1.x;
}

cvex::Line::Line(vector<Point>& ctr, int roop) {
	cvex::CalcMinLengthLineParameter(ctr, *this, roop);
}

double cvex::Line::getY(double x) {
	return this->slope * x + this->intercept;
}

double cvex::Line::getX(double y) {
	return (y - this->intercept) / slope;
}

cv::Point2d cvex::Line::getXPoint(double x) {
	return cv::Point2d(x, getY(x));
}

cv::Point2d cvex::Line::getYPoint(double y) {
	return cv::Point2d(getX(y), y);
}

void cvex::Line::offsetX(double x) {
	this->intercept -= this->slope * x;
}
void cvex::Line::offsetX(double x, cvex::Line& l) {
	l.slope = this->slope;
	l.intercept = this->intercept - this->slope * x;
}

void cvex::Line::offsetY(double y) {
	this->intercept += y;
}
void cvex::Line::offsetY(double y, cvex::Line& l) {
	l.slope = this->slope;
	l.intercept = this->intercept + y;
}

void cvex::Line::offsetP(cv::Point2d p) {
	offsetX(p.x);
	offsetY(p.y);
}
void cvex::Line::offsetP(cv::Point2d p, cvex::Line& l) {
	offsetX(p.x, l);
	l.offsetY(p.y, l);
}

void cvex::Line::offsetA(double degree, cv::Point2d center) {

	cv::Point2d p1 = cv::Point2d(0, this->intercept);
	cv::Point2d p2 = cv::Point2d(1, this->slope + this->intercept);

	p1 = cvex::CalcRotatePos(p1, center, degree);
	p2 = cvex::CalcRotatePos(p2, center, degree);

	cvexLine line(p1, p2);
	this->slope = line.slope;
	this->intercept = line.intercept;
}

void cvex::Line::offsetA(double degree, cv::Point2d center, cvex::Line& l) {

	cv::Point2d p1 = cv::Point2d(0, this->intercept);
	cv::Point2d p2 = cv::Point2d(1, this->slope + this->intercept);

	p1 = cvex::CalcRotatePos(p1, center, degree);
	p2 = cvex::CalcRotatePos(p2, center, degree);

	l = cvexLine(p1, p2);
}

cvex::Line cvex::Line::getNormal(cv::Point2d p) {
	cvex::Line l;
	l.slope = -1 / (this->slope + 1e-8);
	l.intercept = p.y + 1 / (this->slope + 1e-8) * p.x;
	return l;
}

cv::Point2d cvex::Line::crossPoint(cvex::Line line) {
	if (this->slope == line.slope) return cv::Point2d();

	cv::Point2d crossP;
	crossP.x = (line.intercept - this->intercept) / (this->slope - line.slope);
	crossP.y = (this->slope * line.intercept - this->intercept * line.slope) / (this->slope - line.slope);
	return crossP;
}

vector<Point2d> cvex::Line::crossPoint(cvex::Circle circle) {

	double x1 = 0.;
	double y1 = this->intercept;
	double x2 = 1.;
	double y2 = this->intercept + this->slope;
	double xd = x2 - x1;
	double yd = y2 - y1;
	double X = x1 - circle.center.x;
	double Y = y1 - circle.center.y;
	double a = xd * xd + yd * yd;
	double b = xd * X + yd * Y;
	double c = X * X + Y * Y - circle.radius * circle.radius;
	double D = b * b - a * c;

	if (D < 0) return vector<Point2d>();

	double s1 = (-b + sqrt(D)) / a;
	double s2 = (-b - sqrt(D)) / a;

	cv::Point2d p1 = cv::Point2d(x1 + xd * s1, y1 + yd * s1);
	cv::Point2d p2 = cv::Point2d(x1 + xd * s2, y1 + yd * s2);

	if (p1 == p2) return { p1 };
	else return { p1, p2 };
}

double cvex::Line::calcLength(cv::Point2d p) {
	return cvex::CalcPointToLineLength(p, pair<double, double>(this->slope, this->intercept));
}

void cvex::Line::draw(cv::Mat& image, cv::Scalar color, int thickness, int line_type, int gap, int linewidth) {
	switch (line_type) {
	case SOLID_LINE:

		if (abs(this->slope) > 1) {
			cv::line(image, getYPoint(0), getYPoint(image.rows), color, thickness);
		} else {
			cv::line(image, getXPoint(0), getXPoint(image.cols), color, thickness);
		}

		break;

	default:

		if (abs(this->slope) > 1) {
			draw(image, getYPoint(0), getYPoint(image.rows), color, thickness, line_type, gap, linewidth);
		} else {
			draw(image, getXPoint(0), getXPoint(image.cols), color, thickness, line_type, gap, linewidth);
		}

		break;
	}

}

void cvex::Line::draw(cv::Mat& image, cv::Point start, cv::Point end, cv::Scalar color, int thickness, int line_type, int gap, int linewidth) {
	cvexLine normal_line = getNormal(start);
	cv::Point start_line_point = crossPoint(normal_line);

	normal_line = getNormal(end);
	cv::Point end_line_point = crossPoint(normal_line);

	if (thickness <= 0) return;

	if (gap <= 0) gap = thickness * 2;
	if (linewidth <= 0) linewidth = thickness * 4;

	switch (line_type) {
	case SOLID_LINE:
		cv::line(image, start_line_point, end_line_point, color, thickness);
		break;

	case DASHED_LINE:
	{
		vector<Point> line_pos_vec = create_bresenham_points(start_line_point, end_line_point);

		int count = 0;
		for (int i = 0; i < int(line_pos_vec.size()); i++) {
			if (count == 0) {
				if (i + linewidth >= int(line_pos_vec.size())) linewidth = int(line_pos_vec.size()) - i - 1;
				cv::line(image, line_pos_vec[i], line_pos_vec[i + linewidth], color, thickness);
				count = gap;
				i += linewidth;
			} else {
				count--;
			}
		}
		break;
	}

	case DOT_DASH_LINE:
	{
		vector<Point> line_pos_vec = create_bresenham_points(start_line_point, end_line_point);

		int count = 0;
		int dot_or_dash_count = 0;
		for (int i = 0; i < int(line_pos_vec.size()); i++) {
			if (count == 0) {
				int delta = dot_or_dash_count % 2 == 0 ? linewidth : 0;
				if (i + delta >= int(line_pos_vec.size())) linewidth = int(line_pos_vec.size()) - i - 1;
				cv::line(image, line_pos_vec[i], line_pos_vec[dot_or_dash_count % 2 == 0 ? i + linewidth : i], color, thickness);
				count = gap;
				i += dot_or_dash_count % 2 == 0 ? linewidth : 0;
				dot_or_dash_count++;
			} else {
				count--;
			}
		}
	}
	break;

	case TWO_DOT_DASH_LINE:
	{
		vector<Point> line_pos_vec = create_bresenham_points(start_line_point, end_line_point);

		int count = 0;
		int dot_or_dash_count = 0;
		for (int i = 0; i < int(line_pos_vec.size()); i++) {
			if (count == 0) {
				int delta = dot_or_dash_count % 3 == 0 ? linewidth : 0;
				if (i + delta >= int(line_pos_vec.size())) linewidth = int(line_pos_vec.size()) - i - 1;
				cv::line(image, line_pos_vec[i], line_pos_vec[dot_or_dash_count % 3 == 0 ? i + linewidth : i], color, thickness);
				count = gap;
				i += dot_or_dash_count % 3 == 0 ? linewidth : 0;
				dot_or_dash_count++;
			} else {
				count--;
			}
		}
	}
	break;
	}
}

cv::Mat cvex::Line::BinaryBorder(cv::Mat Src, int binary_type) {

	cv::Mat pallet = cv::Mat(Src.size(), CV_8UC1, 255);
	cv::Mat src = Src.clone();

	if (abs(slope) < 1) {
		cv::line(pallet, getXPoint(0), getXPoint(pallet.cols), 0, 2);
	} else {
		cv::line(pallet, getYPoint(0), getYPoint(pallet.rows), 0, 2);
	}

	vector<vector<Point>> contours;
	cv::findContours(pallet, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

	cv::Point center[2];
	center[0] = cvex::CalcMoments(contours[0]);

	switch (binary_type) {

	case UPPER_WHITE:
	{
		if (this->slope * center[0].x + this->intercept - center[0].y > 0) {
			cv::drawContours(src, contours, 0, cv::Scalar(255, 255, 255), -1);
		} else {
			cv::drawContours(src, contours, 1, cv::Scalar(255, 255, 255), -1);
		}
		break;
	}
	case UPPER_BLACK:
	{
		if (this->slope * center[0].x + this->intercept - center[0].y > 0) {
			cv::drawContours(src, contours, 0, cv::Scalar(0, 0, 0), -1);
		} else {
			cv::drawContours(src, contours, 1, cv::Scalar(0, 0, 0), -1);
		}
		break;
	}
	case LOWER_WHITE:
	{
		if (this->slope * center[0].x + this->intercept - center[0].y < 0) {
			cv::drawContours(src, contours, 0, cv::Scalar(255, 255, 255), -1);
		} else {
			cv::drawContours(src, contours, 1, cv::Scalar(255, 255, 255), -1);
		}
		break;
	}
	case LOWER_BLACK:
	{
		if (this->slope * center[0].x + this->intercept - center[0].y < 0) {
			cv::drawContours(src, contours, 0, cv::Scalar(0, 0, 0), -1);
		} else {
			cv::drawContours(src, contours, 1, cv::Scalar(0, 0, 0), -1);
		}
		break;
	}
	case LEFT_WHITE:
	{
		if ((center[0].y - this->intercept) / (this->slope + 1e-6) - center[0].x > 0) {
			cv::drawContours(src, contours, 0, cv::Scalar(255, 255, 255), -1);
		} else {
			cv::drawContours(src, contours, 1, cv::Scalar(255, 255, 255), -1);
		}
		break;
	}
	case LEFT_BLACK:
	{
		if ((center[0].y - this->intercept) / (this->slope + 1e-6) - center[0].x > 0) {
			cv::drawContours(src, contours, 0, cv::Scalar(0, 0, 0), -1);
		} else {
			cv::drawContours(src, contours, 1, cv::Scalar(0, 0, 0), -1);
		}
		break;
	}
	case RIGHT_WHITE:
	{
		if ((center[0].y - this->intercept) / (this->slope + 1e-6) - center[0].x < 0) {
			cv::drawContours(src, contours, 0, cv::Scalar(255, 255, 255), -1);
		} else {
			cv::drawContours(src, contours, 1, cv::Scalar(255, 255, 255), -1);
		}
		break;
	}
	case RIGHT_BLACK:
	{
		if ((center[0].y - this->intercept) / (this->slope + 1e-6) - center[0].x < 0) {
			cv::drawContours(src, contours, 0, cv::Scalar(0, 0, 0), -1);
		} else {
			cv::drawContours(src, contours, 1, cv::Scalar(0, 0, 0), -1);
		}
		break;
	}
	}

	cv::dilate(src, src, cv::Mat(), cv::Point(-1, -1), 1);

	return src;
}

vector<Point> cvex::Line::create_bresenham_points(cv::Point start_point, cv::Point end_point) {

	int sx = start_point.x;
	int sy = start_point.y;
	int dx = end_point.x;
	int dy = end_point.y;

	int x = sx, y = sy; // 現在位置
	int wx = (sx >= dx ? sx - dx : dx - sx); // x座標の差の絶対値
	int wy = (sy >= dy ? sy - dy : dy - sy); // y座標の差の絶対値
	int xmode = (wx >= wy); // x方向の変位がy方向の変位以上か
	int gosa = 0;

	vector<Point> bresenham_points;
	while (x != dx || y != dy) {
		bresenham_points.push_back(cv::Point(x, y));
		if (xmode) { // x方向の変位の方が小さくないので、主にx方向に移動する
			if (sx < dx) x++; else x--; // x方向に進む
			// y方向に進むべき変位を進んでいないので、誤差として加える
			// 誤差は整数にするために(wx * 2)倍で扱う
			gosa += ((dy - sy) << 1);
			if (gosa > wx) { // yの正の方向に進むべきなのに進んでいない誤差が0.5を超えた
				y++; // yの正の方向に進む
				gosa -= (wx << 1); // 進んだ分を誤差に反映させる
			} else if (gosa < -wx) { // yの負の方向(ry
				y--; // yの負(ry
				gosa += (wx << 1); // 進んだ(ry
			}
		} else { // y方向の変位の方が大きいので、主にy方向に移動する
			 // xの時と同様
			if (sy < dy) y++; else y--;
			gosa += ((dx - sx) << 1);
			if (gosa > wy) {
				x++;
				gosa -= (wy << 1);
			} else if (gosa < -wy) {
				x--;
				gosa += (wy << 1);
			}
		}
	}

	return bresenham_points;
};


std::ostream& cvex::operator << (ostream& os, const Line& L) {
	return os << "[s:" << L.slope << ", i:" << L.intercept << "]";
}

#pragma endregion


#pragma region Circle

cvex::Circle::Circle() {
}

cvex::Circle::Circle(cv::Point2d center, double radius)
	:center(center), radius(radius) {
}

cvex::Circle::Circle(cv::Point2d p1, cv::Point2d p2) {
	if (p1 == p2) return;
	this->center = (p1 + p2) / 2.;
	this->radius = cv::norm(p1 - p2) / 2.;
}

cvex::Circle::Circle(cv::Point2d p1, cv::Point2d p2, cv::Point2d p3) {
	if (p1 == p2 && p2 == p3 && p3 == p1) {
		this->center = p1;
		this->radius = 0;
		return;
	}

	if (p1 == p2) {
		*this = Circle(p1, p3);
	} else if (p1 == p3) {
		*this = Circle(p1, p2);
	} else if (p2 == p3) {
		*this = Circle(p2, p3);
	}

	cvexLine line(p1, p2);
	if (line.calcLength(p3) <= DBL_MIN) return;

	calc_params_from(p1, p2, p3);
}

template<typename T>
cvex::Circle::Circle(vector<T>& ctr, int roop) {
	if (ctr.empty()) {
		return;
	} else if (ctr.size() == 1) {
		this->center = cv::Point2d(ctr[0]);
	} else if (ctr.size() == 2) {
		cvexCircle circle(ctr[0], ctr[1]);
		*this = circle;
	} else {
		cvex::CalcCircleParameter(ctr, *this, roop);
	}
}
template cvex::Circle::Circle(std::vector<cv::Point>&, int roop);
template cvex::Circle::Circle(std::vector<cv::Point2f>&, int roop);
template cvex::Circle::Circle(std::vector<cv::Point2d>&, int roop);

void cvex::Circle::calc_params_from(cv::Point2d p1, cv::Point2d p2, cv::Point2d p3) {

	double x1 = p1.x, x2 = p2.x, x3 = p3.x;
	double y1 = p1.y, y2 = p2.y, y3 = p3.y;

	double X1 = x1 * x1 + y1 * y1;
	double X2 = x2 * x2 + y2 * y2;
	double X3 = x3 * x3 + y3 * y3;

	double alpha = x1 - x2;
	double beta = y1 - y2;
	double gamma = x2 - x3;
	double delta = y2 - y3;

	cv::Mat A = cv::Mat(2, 1, CV_64FC1);
	cv::Mat B = (cv::Mat_<double>(2, 2) <<
		delta, -beta,
		-gamma, alpha);
	cv::Mat C = (cv::Mat_<double>(2, 1) <<
		X1 - X2,
		X2 - X3);

	A = 1 / (2 * (alpha * delta - beta * gamma)) * B * C;

	double a = A.ptr<double>(0)[0];
	double b = A.ptr<double>(1)[0];

	this->center = cv::Point2d(a, b);
	this->radius = sqrt(pow(x1 - a, 2) + pow(y1 - b, 2));
}

double cvex::Circle::area() {
	return M_PI * this->radius * this->radius;
}

double cvex::Circle::arcLength() {
	return 2 * M_PI * this->radius;
}

cv::Mat cvex::Circle::warpPolar(cv::Mat image, int flag, int overlap) {

	cv::Mat polar_out;
	cv::Size dst_size(int(this->radius), int(arcLength()));
	cv::warpPolar(image, polar_out, dst_size, this->center, this->radius, flag);

	cv::Mat cpyMake;
	cv::copyMakeBorder(polar_out, cpyMake, overlap, overlap, 0, 0, cv::BORDER_WRAP);

	return cpyMake;
}

cv::Point2d cvex::Circle::warpPolar(cv::Point2d pos, int rotateCode) {

	double x = this->center.x - pos.x;
	double y = this->center.y - pos.y;
	double theta = atan2(y, x);
	double PolarX = 0.;
	double PolarY = 0.;

	switch (rotateCode) {

	case cv::ROTATE_90_CLOCKWISE:
	{
		PolarX = (1 - (theta + M_PI) / (2 * M_PI)) * this->arcLength();
		PolarY = cv::norm(this->center - pos);
		break;
	}

	case cv::ROTATE_180:
	{
		PolarX = (1 - cv::norm(this->center - pos) / this->radius) * this->radius - 1;
		PolarY = (1 - (theta + M_PI) / (2 * M_PI)) * this->arcLength();
		break;
	}

	case cv::ROTATE_90_COUNTERCLOCKWISE:
	{
		PolarX = (theta + M_PI) / (2 * M_PI) * this->arcLength();
		PolarY = (1 - cv::norm(this->center - pos) / this->radius) * this->radius - 1;
		break;
	}

	default:
	{
		PolarX = cv::norm(this->center - pos);
		PolarY = (theta + M_PI) / (2 * M_PI) * this->arcLength();
		break;
	}
	}

	return cv::Point2d(PolarX, PolarY);
}

cv::Mat cvex::Circle::warpPolarInv(cv::Mat image, cv::Size dst_size, int overlap) {

	cv::Mat clip_out;
	image.rowRange(cv::Range(overlap, image.rows - overlap)).copyTo(clip_out);

	cv::Mat dst;
	int flags = cv::INTER_NEAREST + cv::WARP_FILL_OUTLIERS + cv::WARP_POLAR_LINEAR + cv::WARP_INVERSE_MAP;
	cv::warpPolar(clip_out, dst, dst_size, this->center, this->radius, flags);

	return dst;
}

cv::Point2d cvexCircle::warpPolarInv(cv::Point2d pos, int rotateCode) {

	double px = pos.x;
	double py = pos.y;

	switch (rotateCode) {

	case cv::ROTATE_90_CLOCKWISE:
	{
		double tmp = px;
		px = this->radius - py;
		py = tmp;
		break;
	}

	case cv::ROTATE_180:
	{
		px = this->radius - px;
		py = this->arcLength() - py;
		break;
	}

	case cv::ROTATE_90_COUNTERCLOCKWISE:
	{
		double tmp = px;
		px = py;
		py = this->arcLength() - tmp;
		break;
	}
	}

	double theta = py * 2 * M_PI / this->arcLength() - M_PI;
	double r = px;

	double ix = this->center.x - r * cos(theta);
	double iy = this->center.y - r * sin(theta);

	return cv::Point2d(ix, iy);
}

void cvex::Circle::draw(cv::Mat& image, cv::Scalar color, int thickness, bool use_buresenham) {

	if (use_buresenham && thickness >= 0) {
		cvex::DrawBresenhamCircle(image, this->center, this->radius, color, thickness);
	} else {
		cv::circle(image, this->center, int(this->radius), color, thickness);
	}
}

void cvex::Circle::draw(cv::Mat& image, cv::Scalar color, int thickness, double start_degree, double end_degree, bool min_arc) {

	if (start_degree >= 360) {
		while (start_degree >= 360) {
			start_degree -= 360;
		}
	} else if (start_degree < 0) {
		while (start_degree < 0) {
			start_degree += 360;
		}
	}

	if (end_degree >= 360) {
		while (end_degree >= 360) {
			end_degree -= 360;
		}
	} else if (end_degree < 0) {
		while (end_degree < 0) {
			end_degree += 360;
		}
	}

	if (start_degree > end_degree) std::swap(start_degree, end_degree);


	if (min_arc && end_degree - start_degree <= 180) {
		cv::ellipse(image, this->center, cv::Size(int(this->radius), int(this->radius)), 0., start_degree, end_degree, color, thickness);
	} else if (min_arc && end_degree - start_degree > 180) {
		cv::ellipse(image, this->center, cv::Size(int(this->radius), int(this->radius)), 0., start_degree + 360, end_degree, color, thickness);
	} else if (!min_arc && end_degree - start_degree <= 180) {
		cv::ellipse(image, this->center, cv::Size(int(this->radius), int(this->radius)), 0., start_degree + 360, end_degree, color, thickness);
	} else {
		cv::ellipse(image, this->center, cv::Size(int(this->radius), int(this->radius)), 0., start_degree, end_degree, color, thickness);
	}
}

double cvex::Circle::CalcPointToCircleLength(cv::Point2d pos) {
	return cv::norm(pos - this->center) - this->radius;
}

void cvex::Circle::MeanStdDev(vector<Point> ctr, double& Mean, double& StdDev) {

	double sum = 0.;
	double sum2 = 0.;
	for (int i = 0; i < ctr.size(); i++) {

		cv::Point2d pos = (cv::Point2d)ctr[i];
		double d = this->CalcPointToCircleLength(pos);

		sum += d;
		sum2 += d * d;
	}

	double mean = sum / (double)ctr.size();
	double mean2 = sum2 / (double)ctr.size();
	double variance = mean2 - (mean * mean);
	double sd = sqrt(variance);

	Mean = mean;
	StdDev = sd;
}

bool cvex::Circle::contains(cv::Point2d p) {
	return cv::norm(this->center - p) <= this->radius;
}

cv::Rect cvex::Circle::boundingRect() {
	return cv::Rect(this->center - cv::Point2d(this->radius, this->radius), cv::Size(2 * int(this->radius), 2 * int(this->radius)));
}

cv::Point2d cvex::Circle::CalcRotatePos(cv::Point2d pos, double theta) {

	cv::Point2d P = cvex::CalcRotatePos(pos, this->center, theta);

	cv::Point2d diff = P - this->center;

	double length = cv::norm(diff);
	double ratio = this->radius / length;

	return this->center + ratio * diff;

	//return P;
}

vector<Point2d> cvex::Circle::crossPoints(cvex::Line line) {
	return line.crossPoint(*this);
}

vector<Point2d> cvex::Circle::crossPoints(cvex::Circle C) {

	if (this->center == C.center && this->radius == C.radius) return vector<Point2d>();

	cv::Point2d c1 = this->center;
	cv::Point2d c2 = C.center;
	double d = cv::norm(c1 - c2);
	double r1 = this->radius;
	double r2 = C.radius;

	//離れている・一方の円内部
	if (d > r1 + r2 || d < abs(r1 - r2)) return vector<Point2d>();

	//外接・内接
	if (d == r1 + r2 || d == abs(r1 - r2)) {
		cvexLine line(c1, c2);
		vector<Point2d> cross_points1 = line.crossPoint(*this);
		vector<Point2d> cross_points2 = line.crossPoint(C);

		vector<pair<double, Point2d>> length;
		for (cv::Point2d p1 : cross_points1) {
			for (cv::Point2d p2 : cross_points2) {
				if (p1 == p2) return { p1 };
				length.push_back({ cv::norm(p1 - p2), p1 });
			}
		}

		return { std::min_element(length.begin(), length.end(), [](auto l1, auto l2)
			-> bool {return l1.first < l2.first; })->second };
	}


	//2点で交わる
	double x1 = c1.x;
	double x2 = c2.x;
	double y1 = c1.y;
	double y2 = c2.y;
	double a = 2 * (x2 - x1);
	double b = 2 * (y2 - y1);
	double c = (x1 * x1 - x2 * x2) + (y1 * y1 - y2 * y2) + (r2 * r2 - r1 * r1);

	//ax + by + c = 0;
	double slope;
	double intercept;
	cvexLine line;
	if (b == 0) {
		line = cvexLine(cv::Point2d(-c / a, 0), cv::Point2d(-c / a, 1));
	} else {
		slope = -a / b;
		intercept = -c / b;
		line = cvexLine(slope, intercept);
	}

	return line.crossPoint(*this);
}

void cvex::Circle::offsetX(double x) {
	this->center.x += x;
}

void cvex::Circle::offsetX(double x, cvex::Circle& C) {
	C = *this;
	C.center.x += x;
}

void cvex::Circle::offsetY(double y) {
	this->center.y += y;
}

void cvex::Circle::offsetY(double y, cvex::Circle& C) {
	C = *this;
	C.center.y += y;
}

void cvex::Circle::offsetP(cv::Point2d p) {
	this->center += p;
}

void cvex::Circle::offsetP(double x, double y) {
	this->center.x += x;
	this->center.y += y;
}

void cvex::Circle::offsetP(cv::Point2d p, cvex::Circle& C) {
	C = *this;
	C.center += p;
}

void cvex::Circle::offsetP(double x, double y, cvex::Circle& C) {
	C = *this;
	C.center.x += x;
	C.center.y += y;
}

std::ostream& cvex::operator << (ostream& os, const Circle& C) {
	return os << "[(" << C.center.x << ", " << C.center.y << "), r:" << C.radius << "]";
}

#pragma endregion


#pragma region cvex

void cvex::ShowImage(cv::Mat Image, string str) {

	PParameter pPara = PParameter(new Parameter);
	bool debug_flag = 1;

	static TParas tp = []()-> TParas {

		return tp;
	}();

	if (debug_flag) {
		static bool trackbar_flag = [&]() -> bool {

			CreateWindow(1);
			Track("show", 1, tp.show, 0, NULL);

			return true;
		}();
	}

	do {
		pPara->ss << str;
		cv::Mat Src = Image.clone();

		vector<pair<Mat, string>> ProcessImages{
			{Src, "src"},
		};

		cvex::ShowProcess(ProcessImages, tp, pPara);

		switch (cv::waitKey(1)) {

		case 's':
			debug_flag = false;
			break;

		}

	} while (debug_flag);

}

void cvex::ShowImage(vector<pair<Mat, string>> Images, string str) {

	PParameter pPara = PParameter(new Parameter);
	bool debug_flag = 1;

	static TParas tp = []()-> TParas {

		return tp;
	}();

	if (debug_flag) {

		static bool trackbar_flag = [&]() -> bool {

			CreateWindow(1);
			Track("show", 1, tp.show, 0, NULL);

			return true;
		}();
	}

	do {

		pPara->ss << str << endl;

		vector<pair<Mat, string>> ProcessImages;
		for (int i = 0; i < Images.size(); i++) {
			ProcessImages.push_back({ Images[i].first.clone(), Images[i].second });
		}

		cvex::ShowProcess(ProcessImages, tp, pPara);

		switch (cv::waitKey(1)) {

		case 's':
			debug_flag = false;
			break;

		}

	} while (debug_flag);

}

tuple<Mat, double, Point> cvex::ShowProcess(vector<pair<Mat, string>>& ProcessImages, Parameter::TrackParas& tp, PParameter pPara) {
	if (tp.show > ProcessImages.size() - 1) tp.show = int(ProcessImages.size()) - 1;
	cv::setTrackbarMax("show", "TrackbarWindow1", int(ProcessImages.size()) - 1);
	//cv::setTrackbarMax("scroll", "TrackbarWindow1", pPara->textcount * 30);

	cv::namedWindow("Process", cv::WINDOW_AUTOSIZE);
	cv::namedWindow("TextWindow", cv::WINDOW_AUTOSIZE);
	cv::resizeWindow("TextWindow", cv::Size(700, 1000));

	try {

		static cv::Rect ShowArea = pPara->Screen;
		static string adress;
		static double SizeRatio = 1.0;
		static tuple<Mat*, Rect*, string*, double*, PParameter> userdata;
		userdata = tuple<Mat*, Rect*, string*, double*, PParameter>(&ProcessImages[tp.show].first, &ShowArea, &adress, &SizeRatio, pPara);
		static tuple<int*, PParameter> txtuserdata(&tp.scroll, pPara);
		cv::setMouseCallback("Process", MouseCallBack, &userdata);
		cv::setMouseCallback("TextWindow", cvex::TextImageMouseCallBack, &txtuserdata);

		stringstream ss;
		cv::Rect roi = pPara->MousePara.MouseRect;
		cv::Point txt_pos;

		if (!roi.empty()) {
			//cv::rectangle(ProcessImages[tp.show].first, pPara->MousePara.MouseRect, cv::Scalar(0, 255, 0), 1.2);
			cvex::DrawRect(ProcessImages[tp.show].first, pPara->MousePara.MouseRect, cv::Scalar(0, 255, 0), 1, cvex::Line::TWO_DOT_DASH_LINE);
			ss << "[(" << roi.x << "," << roi.y << "), w:" << roi.width << " h:" << roi.height << "]";
			txt_pos = CalcRectTextPos(ProcessImages[tp.show].first, ss.str(), 0.5, 1, roi);
			cv::putText(ProcessImages[tp.show].first, ss.str(), txt_pos,
				cv::FONT_HERSHEY_TRIPLEX, 0.5, cv::Scalar(0, 255, 0), 1);
		}

		for (int i = 0; i < pPara->MousePara.DrawRects.size(); i++) {

			ss.str("");
			roi = pPara->MousePara.DrawRects[i];
			if (roi.height == 0 && roi.width == 0) {
				ss << "(" << roi.x << "," << roi.y << ")";
				cv::circle(ProcessImages[tp.show].first, pPara->MousePara.DrawRects[i].tl(), 2, cv::Scalar(0, 0, 255), -1);
			} else {
				ss << "[(" << roi.x << "," << roi.y << "), w:" << roi.width << " h:" << roi.height << "]";
				cv::rectangle(ProcessImages[tp.show].first, pPara->MousePara.DrawRects[i], cv::Scalar(0, 0, 255), 1);
			}
			txt_pos = CalcRectTextPos(ProcessImages[tp.show].first, ss.str(), 0.5, 1, roi);
			cv::putText(ProcessImages[tp.show].first, ss.str(), txt_pos,
				cv::FONT_HERSHEY_TRIPLEX, 0.5, cv::Scalar(0, 0, 255), 1);
		}

		cv::Mat Pallet;
		cv::resize(ProcessImages[tp.show].first, Pallet, cv::Size(), SizeRatio, SizeRatio, cv::INTER_NEAREST);

		cv::convertScaleAbs(Pallet, Pallet, 1, 0);

		ShowArea.width = pPara->Screen.width;
		ShowArea.height = pPara->Screen.height;

		if (Pallet.cols < ShowArea.br().x) {
			ShowArea.x = Pallet.cols - ShowArea.width;
			if (ShowArea.x < 0) {
				ShowArea.x = 0;
				ShowArea.width = Pallet.cols;
			}
		} else if (ShowArea.x < 0) {
			ShowArea.x = 0;
			if (Pallet.cols < ShowArea.br().x) {
				ShowArea.width = Pallet.cols;
			}
		}
		if (Pallet.rows < ShowArea.br().y) {
			ShowArea.y = Pallet.rows - ShowArea.height;
			if (ShowArea.y < 0) {
				ShowArea.y = 0;
				ShowArea.height = Pallet.rows;
			}
		} else if (ShowArea.y < 0) {
			ShowArea.y = 0;
			if (Pallet.rows < ShowArea.br().y) {
				ShowArea.height = Pallet.rows;
			}
		}

		Pallet = Pallet(ShowArea).clone();

		cv::Mat Output = Pallet.clone();

		cv::Point center = cv::Point(Pallet.size() / 2);
		//cv::circle(Pallet, center, 1, cv::Scalar(0, 0, 255), 2);

		stringstream sRatio;
		sRatio << fixed << setprecision(1) << SizeRatio;

		cv::Rect roi1 = cv::Rect(cv::Point(0, Pallet.rows - 50), cv::Size(100, 50));
		cv::Rect roi2 = cv::Rect(cv::Point(0, 0), cv::Size(100, 15));
		if (roi1.y < 0) roi1.y = 0;
		if (roi1.height > Pallet.rows) roi1.height = Pallet.rows;
		if (roi1.width > Pallet.cols) roi1.width = roi2.width = Pallet.cols;
		if (roi2.height > Pallet.rows) roi2.height = Pallet.rows;

		double brightness = double(cv::mean(Pallet(roi1))[0]);
		cv::Scalar Color1 = cv::Scalar((brightness - 128 >= 0 ? brightness - 128 : brightness + 128), 0, 255);
		brightness = double(cv::mean(Pallet(roi2))[0]);
		cv::Scalar Color2 = cv::Scalar((brightness - 128 >= 0 ? brightness - 128 : brightness + 128), 0, 255);

		cv::putText(Pallet, ProcessImages[tp.show].second, cv::Point(10, 15), cv::FONT_HERSHEY_TRIPLEX,
			0.5, Color2, 1);
		cv::putText(Pallet, pPara->MousePara.BrightStr, cv::Point(10, ShowArea.height - 5), cv::FONT_HERSHEY_TRIPLEX,
			0.5, Color1, 1);
		cv::putText(Pallet, "Adress:" + adress, cv::Point(10, ShowArea.height - 20), cv::FONT_HERSHEY_TRIPLEX,
			0.5, Color1, 1);
		cv::putText(Pallet, "Ratio:" + sRatio.str(), cv::Point(10, ShowArea.height - 35), cv::FONT_HERSHEY_TRIPLEX,
			0.5, Color1, 1);
		//cv::putText(Pallet, "frame:" + to_string(tp.frame), cv::Point(10, ShowArea.height - 50), cv::FONT_HERSHEY_TRIPLEX,
		//	0.5, Color1, 1);

		pPara->PutText(pPara->ss);

		cv::imshow("TextWindow", pPara->TextImage(cv::Rect(cv::Point(0, tp.scroll), cv::Size(pPara->TextImage.cols, 1000))));
		cv::imshow("Process", Pallet);

		pPara->TextImage.setTo(255);

		return { Output, SizeRatio , pPara->MousePara.MousePoint };

	} catch (...) {
		cout << "ShowProcess Err" << endl;
	}
	return { Mat::zeros(cv::Size(0, 0), CV_8UC1), 0 , Point(0,0) };
}

void cvex::MouseCallBack(int event, int x, int y, int flag, void* data) {

	tuple<Mat*, Rect*, string*, double*, PParameter> userdata = *(static_cast<tuple<Mat*, Rect*, string*, double*, PParameter>*>(data));
	cv::Mat* Image = std::get<0>(userdata);
	cv::Rect* ShowArea = std::get<1>(userdata);
	string* adress = std::get<2>(userdata);
	double* SizeRatio = std::get<3>(userdata);
	PParameter pPara = std::get<4>(userdata);

	static cv::Point ClickPoint, ShowAreaPoint, LButtonDownPoint;
	cv::Point DiffClickPoint, DiffShowAreaPoint;
	cv::Rect roi;

	switch (event) {

	case cv::EVENT_LBUTTONDOWN:

		LButtonDownPoint = cv::Point(int((double(ShowArea->x) + double(x)) / *SizeRatio), int((double(ShowArea->y) + double(y)) / *SizeRatio));

		break;

	case cv::EVENT_LBUTTONUP:

		pPara->MousePara.DrawRects.push_back(pPara->MousePara.MouseRect);
		pPara->MousePara.MouseRect = cv::Rect();

		break;

	case cv::EVENT_RBUTTONDOWN:

		if (pPara->MousePara.DrawRects.size() == 0) {
			break;
		}

		pPara->MousePara.DrawRects.erase(pPara->MousePara.DrawRects.end() - 1);

		break;

	case cv::EVENT_RBUTTONDBLCLK:

		pPara->MousePara.DrawRects.clear();

		break;

	case cv::EVENT_MBUTTONDOWN:

		ClickPoint = cv::Point(x, y);
		ShowAreaPoint = ShowArea->tl();

		break;

	case cv::EVENT_MOUSEWHEEL:

		double DiffSizeRatio = [=]() -> double {
			if (flag & cv::EVENT_FLAG_CTRLKEY) return 0.5;
			return 0.1;
		}();

		double previous = *SizeRatio;

		if (getMouseWheelDelta(flag) > 0) {
			(*SizeRatio += DiffSizeRatio) > pPara->MaxSizeRatio ? *SizeRatio = pPara->MaxSizeRatio : NULL;
		} else if (getMouseWheelDelta(flag) < 0) {
			(*SizeRatio -= DiffSizeRatio) < pPara->MinSizeRatio ? *SizeRatio = pPara->MinSizeRatio : NULL;

		}

		pPara->MousePara.ImgSizeRatio = *SizeRatio;
		pPara->MousePara.DeltaSizeRatio = *SizeRatio / previous;

		ShowArea->x = int((ShowArea->x + ShowArea->width / 2) * pPara->MousePara.DeltaSizeRatio - ShowArea->width / 2);
		ShowArea->y = int((ShowArea->y + ShowArea->height / 2) * pPara->MousePara.DeltaSizeRatio - ShowArea->height / 2);

		return;
	}

	switch (flag) {

	case cv::EVENT_FLAG_LBUTTON:

		pPara->MousePara.MouseRect = cv::Rect(LButtonDownPoint, cv::Point(int((double(ShowArea->x) + double(x)) / *SizeRatio), int((double(ShowArea->y) + double(y)) / *SizeRatio)));

		break;

	case cv::EVENT_FLAG_MBUTTON:

		DiffClickPoint = cv::Point(x, y) - ClickPoint;
		DiffShowAreaPoint = ShowAreaPoint - DiffClickPoint;

		ShowArea->x = DiffShowAreaPoint.x;
		ShowArea->y = DiffShowAreaPoint.y;

		if (ShowArea->x < 0) {
			ShowArea->x = 0;
		} else if (ShowArea->br().x > int(double(Image->cols) * *SizeRatio) - 1) {
			ShowArea->x = int(double(Image->cols) * *SizeRatio) - 1 - ShowArea->width;
		}
		if (ShowArea->y < 0) {
			ShowArea->y = 0;
		} else if (ShowArea->br().y > int(double(Image->rows) * *SizeRatio) - 1) {
			ShowArea->y = int(double(Image->rows) * *SizeRatio) - 1 - ShowArea->height;
		}

		break;

	}

	int ptrX = int((double(ShowArea->x) + double(x)) / *SizeRatio);
	int ptrY = int((double(ShowArea->y) + double(y)) / *SizeRatio);

	if (ptrX < 0 || ptrY < 0 || ptrX >= Image->cols || ptrY >= Image->rows) {
		return;
	} else if (Image->channels() == 1) {
		int Brightness = Image->ptr<uchar>(ptrY)[ptrX];
		pPara->MousePara.BrightStr = "Gray: " + to_string(Brightness);
	} else {
		cv::Vec3b Brightness = Image->ptr<Vec3b>(ptrY)[ptrX];
		pPara->MousePara.BrightStr = "Color: (" + to_string(Brightness[0]) + "," + to_string(Brightness[1]) + "," + to_string(Brightness[2]) + ")";
	}

	*adress = "(" + to_string(ptrX) + "," + to_string(ptrY) + ")";
	pPara->MousePara.MousePoint = cv::Point(x, y);
	pPara->MousePara.ScalePtr = cv::Point(ptrX, ptrY);
}

void cvex::TextImageMouseCallBack(int event, int x, int y, int flag, void* data) {

	tuple<int*, PParameter>* userdata = static_cast<tuple<int*, PParameter>*>(data);
	int* scroll = get<0>(*userdata);
	PParameter pPara = get<1>(*userdata);

	switch (event) {

	case cv::EVENT_MOUSEWHEEL:

		int deltaY = 100;

		if (getMouseWheelDelta(flag) > 0) {
			*scroll > deltaY ? *scroll -= deltaY : *scroll = 0;
		} else if (getMouseWheelDelta(flag) < 0) {
			*scroll < pPara->TextImage.rows - 1 - deltaY - 1000 ? *scroll += deltaY : *scroll = pPara->TextImage.rows - 1 - 1000;
		}

		//cv::setTrackbarPos("scroll", "TrackbarWindow1", *scroll);

		return;
	}
}

cv::Point cvex::CalcRectTextPos(cv::Mat image, string val, double font_scale, int thickness, cv::Rect rect) {
	cv::Size text_size = cv::getTextSize(val, cv::FONT_HERSHEY_TRIPLEX, font_scale, thickness, nullptr);

	cv::Point pos = rect.tl() - cv::Point(0, 7);

	if (pos.y - text_size.height < 0) pos.y = rect.br().y + text_size.height + 7;
	if (pos.y > image.rows) pos.y = rect.tl().y - 7;
	if (pos.x + text_size.width > image.cols) pos.x = image.cols - text_size.width;
	if (pos.x < 0) pos.x = 0;

	return pos;
}

template<typename T>
void cvex::CalcCircleParameter(vector<T> Contour, pair<Point2d, double>& CirclePara, vector<double> weight) {

	/******************************************************

	近似円 (Y - a)^2 + (X - b)^2 = R^2
	中心(a, b)
	プロット点群(x, y)

	CX = -2a
	CY = -2b
	CR = a^2 + b^2 - R^2

	A = [∑x^2 ∑xy ∑x ; ∑xy ∑y^2 ∑y ; ∑x ∑x ∑1]
	B = [-∑(x^3 + x * y^2) ; -∑(x^2 * y + y^3) ; -∑(x^2 + y^2)]
	C = [CX, CY, CR]

	C = Inv(A) * B;

	a = - CX / 2
	b = - CY / 2
	R = sqrt(-CR + a^2 + b^2)

	プロット点群の近似円を最小二乗法で求める。

	*******************************************************/

	if (weight.empty()) {
		weight = vector<double>(Contour.size());
		std::fill(weight.begin(), weight.end(), 1.0);
	}

	cv::Mat A = cv::Mat::zeros(cv::Size(3, 3), CV_64FC1);
	cv::Mat B = cv::Mat::zeros(cv::Size(1, 3), CV_64FC1);
	cv::Mat C = cv::Mat::zeros(cv::Size(1, 3), CV_64FC1);

	for (int i = 0; i < Contour.size(); i++) {

		double x = (double)Contour[i].x;
		double y = (double)Contour[i].y;
		double w = weight[i];

		A += w * (cv::Mat_<double>(cv::Size(3, 3)) <<
			pow(x, 2), x * y, x,
			x * y, pow(y, 2), y,
			x, y, 1);

		B -= w * (cv::Mat_<double>(cv::Size(1, 3)) <<
			pow(x, 3) + x * pow(y, 2),
			pow(x, 2) * y + pow(y, 3),
			pow(x, 2) + pow(y, 2));
	}

	C = A.inv(cv::DECOMP_SVD) * B;

	cv::Point2d Curvature_Center = cv::Point2d(-C.at<double>(0) / 2, -C.at<double>(1) / 2);
	double Curvature_Radius = sqrt(-C.at<double>(2) + pow(Curvature_Center.x, 2) + pow(Curvature_Center.y, 2));

	std::pair<cv::Point2d, double> CurveInfo = { Curvature_Center, Curvature_Radius };

	CirclePara = CurveInfo;
}
template void cvex::CalcCircleParameter<cv::Point>(std::vector<cv::Point>, pair<cv::Point2d, double>&, vector<double>);
template void cvex::CalcCircleParameter<cv::Point2f>(std::vector<cv::Point2f>, pair<cv::Point2d, double>&, vector<double>);
template void cvex::CalcCircleParameter<cv::Point2d>(std::vector<cv::Point2d>, pair<cv::Point2d, double>&, vector<double>);

template<typename T>
void cvex::CalcCircleParameter(std::vector<T> Contour, cv::Point2d& Center, double& Radius, int converge_times) {

	if (Contour.size() < 3) {
		Center = Point2d(0., 0.);
		Radius = 0.;
		return;
	}

	int n = int(Contour.size());
	vector<Point2d> data(n);
	for (int i = 0; i < n; i++) {
		data[i] = cv::Point2d(Contour[i]);
	}

	vector<double> weight_list(n), err_list(n);
	std::fill(weight_list.begin(), weight_list.end(), 1.0);

	pair<Point2d, double> circle_parameter;

	do {
		cvex::CalcCircleParameter(data, circle_parameter, weight_list);

		for (int i = 0; i < n; i++) {
			err_list[i] = abs(cv::norm(circle_parameter.first - data[i]) - circle_parameter.second);
		}

		vector<double> sort_err_list = err_list;
		std::sort(sort_err_list.begin(), sort_err_list.end());

		double err_threshold = sort_err_list[n / 2] * 1.25;
		if (err_threshold <= 1e-6) {
			Center = circle_parameter.first;
			Radius = circle_parameter.second;
			break;
		}

		double inv_err = 1 / err_threshold;

		for (int i = 0; i < n; i++) {
			if (err_list[i] > err_threshold) {
				weight_list[i] = 0;
			} else {
				double r = 1 - pow(err_list[i] * inv_err, 2);
				weight_list[i] = r * r;
			}
		}

		Center = circle_parameter.first;
		Radius = circle_parameter.second;

		converge_times--;
	} while (converge_times > 0);
}
template void cvex::CalcCircleParameter<cv::Point>(std::vector<cv::Point>, cv::Point2d&, double&, int);
template void cvex::CalcCircleParameter<cv::Point2f>(std::vector<cv::Point2f>, cv::Point2d&, double&, int);
template void cvex::CalcCircleParameter<cv::Point2d>(std::vector<cv::Point2d>, cv::Point2d&, double&, int);

template<typename T>
void cvex::CalcCircleParameter(std::vector<T> Contour, cvex::Circle& Circle, int converge_times) {

	cv::Point2d center;
	double radius;
	cvex::CalcCircleParameter(Contour, center, radius, converge_times);

	Circle = cvexCircle(center, radius);
}
template void cvex::CalcCircleParameter<cv::Point>(std::vector<Point>, cvex::Circle&, int);
template void cvex::CalcCircleParameter<cv::Point2f>(std::vector<Point2f>, cvex::Circle&, int);
template void cvex::CalcCircleParameter<cv::Point2d>(std::vector<Point2d>, cvex::Circle&, int);

void cvex::CalcCircleParameter(std::vector<cv::Point> Contour, pair<cv::Point2d, double>& Circle) {
	cvexCircle circle;
	cvex::CalcCircleParameter(Contour, circle);

	Circle = std::pair<cv::Point2d, double>{ circle.center, circle.radius };
}

bool cvex::CalcEllipseParameter(std::vector<cv::Point> Contour, cv::Point2d& Center, double& Lx, double& Ly, double& Degree) {

	/*********************************************************

	楕円の中心点 (X0, Y0)
	X軸方向の長さを Lx
	Y軸方向の長さを Ly
	楕円の傾きを θ

	楕円の一般式
	{(X - X0)cosθ + (Y - Y0)sinθ) / Lx}^2
		+ {-(X - X0)sinθ + (Y - Y0)cosθ) / Ly}^2 = 1

	プロット点群(x, y)としたとき

	A = [∑x^2*y^2	∑xy^3	∑x^2*y	∑xy^2	∑xy
		 ∑xy^3		∑y^4	∑xy^2	∑y^3	∑y^2
		 ∑x^2*y		∑xy^2	∑x^2	∑xy		∑x
		 ∑xy^2		∑y^3	∑xy 	∑y^2	∑y
		 ∑xy		∑y^2	∑x		∑y		∑1]

	B = [-∑x^3*y			C = [a
		 -∑x^2*y^2				 b
		 -∑x^3					 c
		 -∑x^2*y				 d
		 -∑x^2]					 e]

	C = Inv(A) * B;

	X0 = (ad - 2bc) / (4b - a^2)
	Y0 = (ac - 2d) / (4b - a^2)
	θ = atan{a / (1 - b)} / 2
	Lx = sqrt[(X0cosθ + Y0sinθ)^2 - e(cosθ)^2 - {(X0sinθ - Y0cosθ)^2 - e(sinθ)^2}(sinθ^2 - b(cosθ)^2) / (cosθ^2 - b(sinθ)^2)]
	Ly = sqrt[(X0sinθ - Y0cosθ)^2 - e(sinθ)^2 - {(X0cosθ + Y0sinθ)^2 - e(cosθ)^2}(cosθ^2 - b(sinθ)^2) / (sinθ^2 - b(cosθ)^2)]

	***********************************************************/

	cv::Mat A = cv::Mat::zeros(cv::Size(5, 5), CV_64FC1);
	cv::Mat B = cv::Mat::zeros(cv::Size(1, 5), CV_64FC1);
	cv::Mat C = cv::Mat::zeros(cv::Size(1, 5), CV_64FC1);

	for (auto pos : Contour) {

		double x = (double)pos.x;
		double y = (double)pos.y;

		A += (cv::Mat_<double>(cv::Size(5, 5)) <<
			x * x * y * y, x * y * y * y, x * x * y, x * y * y, x * y,
			x * y * y * y, y * y * y * y, x * y * y, y * y * y, y * y,
			x * x * y, x * y * y, x * x, x * y, x,
			x * y * y, y * y * y, x * y, y * y, y,
			x * y, y * y, x, y, 1);

		B -= (cv::Mat_<double>(cv::Size(1, 5)) <<
			x * x * x * y,
			x * x * y * y,
			x * x * x,
			x * x * y,
			x * x);
	}

	C = A.inv(cv::DECOMP_SVD) * B;

	double a = C.ptr<double>(0)[0];
	double b = C.ptr<double>(1)[0];
	double c = C.ptr<double>(2)[0];
	double d = C.ptr<double>(3)[0];
	double e = C.ptr<double>(4)[0];

	double X0 = (a * d - 2 * b * c) / (4 * b - a * a);
	double Y0 = (a * c - 2 * d) / (4 * b - a * a);
	double theta = atan(a / (1 - b)) / 2;

	double alpha = pow(X0 * cos(theta) + Y0 * sin(theta), 2)
		- e * pow(cos(theta), 2)
		- (pow(X0 * sin(theta) - Y0 * cos(theta), 2) - e * pow(sin(theta), 2))
		* (pow(sin(theta), 2) - b * pow(cos(theta), 2))
		/ (pow(cos(theta), 2) - b * pow(sin(theta), 2));

	double beta = pow(X0 * sin(theta) - Y0 * cos(theta), 2)
		- e * pow(sin(theta), 2)
		- (pow(X0 * cos(theta) + Y0 * sin(theta), 2) - e * pow(cos(theta), 2))
		* (pow(cos(theta), 2) - b * pow(sin(theta), 2))
		/ (pow(sin(theta), 2) - b * pow(cos(theta), 2));

	if (alpha <= 0 || beta <= 0) {
		return false;
	}

	double lx = sqrt(alpha);
	double ly = sqrt(beta);

	Center = cv::Point2d(X0, Y0);
	Lx = lx;
	Ly = ly;
	Degree = theta / M_PI * 180;

	return true;
}

bool cvex::CalcEllipseParameter(std::vector<cv::Point> Contour, cv::RotatedRect& Rect) {

	cv::Point2d center;
	double Lx, Ly, angle;
	if (!cvex::CalcEllipseParameter(Contour, center, Lx, Ly, angle)) {
		return false;
	};

	Rect = cv::RotatedRect(center, cv::Size(int(2 * Lx), int(2 * Ly)), float(angle));
	return true;
}

void cvex::CalcLineParameter(vector<Point> Contour, pair<double, double>& LinePara) {

	/*********************************************************

	近似直線 Y = aX + b
	プロット点群(x, y)

	A = [∑x^2 ∑x ; ∑x ∑1]
	B = [∑xy ; ∑y]
	C = [a ; b]

	C = Inv(A) * B

	Y軸方向での最小距離を求める（最小二乗法）
	プロット点群の傾きがY軸方向に近くなると、イメージ直線と違ってくる

	***********************************************************/

	cv::Mat A = cv::Mat::zeros(cv::Size(2, 2), CV_64FC1);
	cv::Mat B = cv::Mat::zeros(cv::Size(1, 2), CV_64FC1);
	cv::Mat C = cv::Mat::zeros(cv::Size(1, 2), CV_64FC1);

	for (auto Point : Contour) {

		double x = double(Point.x);
		double y = double(Point.y);

		A += (cv::Mat_<double>(cv::Size(2, 2)) <<
			pow(x, 2), x,
			x, 1);
		B += (cv::Mat_<double>(cv::Size(1, 2)) <<
			x * y,
			y);
	}

	C = A.inv(cv::DECOMP_SVD) * B;

	LinePara = { C.at<double>(0,0), C.at<double>(1, 0) };
}

template<typename T>
void cvex::CalcMinLengthLineParameter(vector<T> Contour, pair<double, double>& LinePara, vector<double> weight) {

	/*********************************************************

	近似直線　Y = aX + b
	プロット点(xi, yi)

	近似直線の傾き a = 1 / (2I) * {-K ± sqrt(K^2 + 4I^2)}
	近似直線の切片 b = 1 / n {sum(yi) - a * sum(xi)};

	ただし I = sum(xi) * sum(yi) - n * sum(xi * yi);
	　　　 K = n * {sum(y^2) - sum(x^2)} + sum(x)^2 - sum(y)^2;

	得られる直線：「求めたい直線」と「その法線」

	***********************************************************/

	if (weight.size() == 0) {
		weight = vector<double>(Contour.size());
		std::fill(weight.begin(), weight.end(), 1.0);
	}

	double Sum_x, Sum_y, Sum_xy, Sum_x2, Sum_y2, N;
	Sum_x = Sum_y = Sum_xy = Sum_x2 = Sum_y2 = N = 0.0;

	for (int i = 0; i < Contour.size(); i++) {

		double x = double(Contour[i].x);
		double y = double(Contour[i].y);
		double w = weight[i];

		N += w;
		Sum_x += w * x;
		Sum_y += w * y;
		Sum_xy += w * x * y;
		Sum_x2 += w * pow(x, 2);
		Sum_y2 += w * pow(y, 2);

	}

	double I = Sum_x * Sum_y - N * Sum_xy + 1e-7;
	double K = N * (Sum_y2 - Sum_x2) + pow(Sum_x, 2) - pow(Sum_y, 2);

	double a = 1 / (2 * I) * (-K - sqrt(pow(K, 2) + 4 * pow(I, 2)));
	double b = 1 / N * (Sum_y - a * Sum_x);

	LinePara = { a, b };
}
template void cvex::CalcMinLengthLineParameter<Point>(vector<Point>, pair<double, double>&, vector<double>);
template void cvex::CalcMinLengthLineParameter<Point2f>(vector<Point2f>, pair<double, double>&, vector<double>);
template void cvex::CalcMinLengthLineParameter<Point2d>(vector<Point2d>, pair<double, double>&, vector<double>);

template<typename T>
void cvex::CalcMinLengthLineParameter(vector<T> Contour, double& slope, double& intercept, int converge_times) {

	if (Contour.size() == 0) {
		slope = 0;
		intercept = 0;
		return;
	}

	int n = int(Contour.size());
	vector<Point2d> data(n);
	for (int i = 0; i < n; i++) {
		data[i] = cv::Point2d(Contour[i]);
	}

	vector<double> weight_list(n), err_list(n);
	std::fill(weight_list.begin(), weight_list.end(), 1.0);
	pair<double, double> line_parameter;
	do {
		CalcMinLengthLineParameter(data, line_parameter, weight_list);

		for (int i = 0; i < n; i++) {
			err_list[i] = CalcPointToLineLength(data[i], line_parameter);
		}

		vector<double> sort_err_list = err_list;
		std::sort(sort_err_list.begin(), sort_err_list.end());

		double err_threshold = sort_err_list[n / 2] * 1.25;
		if (err_threshold <= 1e-6) {
			slope = line_parameter.first;
			intercept = line_parameter.second;
			break;
		}

		double inv_err = 1 / err_threshold;

		for (int i = 0; i < n; i++) {
			if (err_list[i] > err_threshold) {
				weight_list[i] = 0;
			} else {
				double r = 1 - pow(err_list[i] * inv_err, 2);
				weight_list[i] = r * r;
			}
		}

		slope = line_parameter.first;
		intercept = line_parameter.second;

		converge_times--;
	} while (converge_times > 0);
}
template void cvex::CalcMinLengthLineParameter<Point>(vector<Point>, double&, double&, int);
template void cvex::CalcMinLengthLineParameter<Point2f>(vector<Point2f>, double&, double&, int);
template void cvex::CalcMinLengthLineParameter<Point2d>(vector<Point2d>, double&, double&, int);

template<typename T>
void cvex::CalcMinLengthLineParameter(std::vector<T> Contour, cvex::Line& line, int converge_times) {
	cvex::CalcMinLengthLineParameter(Contour, line.slope, line.intercept, converge_times);
}
template void cvex::CalcMinLengthLineParameter<Point>(vector<Point>, cvex::Line&, int);
template void cvex::CalcMinLengthLineParameter<Point2f>(vector<Point2f>, cvex::Line&, int);
template void cvex::CalcMinLengthLineParameter<Point2d>(vector<Point2d>, cvex::Line&, int);


double cvex::CalcPointToLineLength(cv::Point2d Pos, pair<Point2d, Point2d> LinePos) {

	/****************************************************

	  2点(x1, y1), (x2, y2)からなる直線
	  x1 != x2 のとき  y - y1 = (y2 - y1)/(x2 - x1) * (x - x1)

	  点(x0, y0) と直線 ax + by + c = 0 との距離 H
	  H = |ax0 + by0 + c| / sqrt(a^2 + b^2)

	  a = (y2 - y1)/(x2 - x1)
	  b = -1
	  c = -ax1 + y1

	 ***************************************************/

	double x0, x1, x2, y0, y1, y2;
	x0 = Pos.x;
	y0 = Pos.y;
	x1 = LinePos.first.x;
	y1 = LinePos.first.y;
	x2 = LinePos.second.x;
	y2 = LinePos.second.y;

	double H = 0.0;
	if (x1 == x2) {
		H = abs(x0 - x1);
	} else {
		double a = (y2 - y1) / (x2 - x1);
		double b = -1;
		double c = -a * x1 + y1;

		H = abs(a * x0 + b * y0 + c) / sqrt(a * a + b * b);
	}

	return H;
};

double cvex::CalcPointToLineLength(Point2d Pos, pair<double, double> LinePara) {

	/****************************************************

	直線：y = ax + b  と　点P：(x0, y0)

	 ***************************************************/

	double x0 = Pos.x;
	double y0 = Pos.y;
	double a = LinePara.first;
	double b = -1;
	double c = LinePara.second;
	double H;

	if (a == 0. && b == 0.) {
		H = y0;
	} else {
		H = abs(a * x0 + b * y0 + c) / sqrt(a * a + b * b);
	}

	return H;
}

double cvex::CalcPointToLineLength(cv::Point2d Pos, cvex::Line Line) {
	return CalcPointToLineLength(Pos, std::pair<double, double>{Line.slope, Line.intercept });
}

double cvex::CalcPointToLineSegmentLength(Point2d Pos, pair<Point2d, Point2d> LinePara) {

	/*******************************************************

	P1(x1, y1)とP2(x2, y2)からなる線分L と　点P0(x0, y0) との最短距離

	線分Lと点P0との最短距離Hを求めると
	H = sqrt{a*(y1-y0)-b*(x1-x0)}^2 / (a^2 + b^2)}

	ただし、点P0を通る線分Lの垂線と線分Lとの交点が線分内に存在しない場合を考慮する必要があるため
	t = -{a*(x1-x0)+b*(y1-y0)}
	a = x2-x1
	b = y2-y1
	とすると

	t < 0 の場合
	H = sqrt{(x1-x0)^2 + (y1-y0)^2}

	t > a^2 + b^2 の場合
	H = sqrt{(x2-x0)^2 + (y2-y0)^2}

	********************************************************/

	Point2d p0, p1, p2;
	p0 = Pos;
	p1 = LinePara.first;
	p2 = LinePara.second;

	double a = p2.x - p1.x;
	double b = p2.y - p1.y;
	double a2 = a * a;
	double b2 = b * b;
	double r2 = a2 + b2;
	double tt = -(a * (p1.x - p0.x) + b * (p1.y - p0.y));

	if (tt < 0.) {
		return sqrt((p1.x - p0.x) * (p1.x - p0.x) + (p1.y - p0.y) * (p1.y - p0.y));
	} else if (tt > r2) {
		return sqrt((p2.x - p0.x) * (p2.x - p0.x) + (p2.y - p0.y) * (p2.y - p0.y));
	} else if (a == 0 && b == 0) {
		return sqrt((p1.x - p0.x) * (p1.x - p0.x) + (p1.y - p0.y) * (p1.y - p0.y));
	}

	double f1 = a * (p1.y - p0.y) - b * (p1.x - p0.x);
	return sqrt((f1 * f1) / r2);
}

double cvex::CalcMinDisFromRects(Rect rect1, Rect rect2, int marge_side) {

	vector<Point2d> pts1{
		Point2d(rect1.tl()),					//左上
		Point2d(rect1.br().x, rect1.tl().y),	//右上
		Point2d(rect1.br()),					//右下
		Point2d(rect1.tl().x, rect1.br().y)		//左下
	};

	vector<Point2d> pts2{
		Point2d(rect2.tl()),					//左上
		Point2d(rect2.br().x, rect2.tl().y),	//右上
		Point2d(rect2.br()),					//右下
		Point2d(rect2.tl().x, rect2.br().y)		//左下
	};

	return CalcMinDisFromRects(pts1, pts2, marge_side);
}

double cvex::CalcMinDisFromRects(RotatedRect rect1, RotatedRect rect2, int marge_side) {

	cv::Point2f pts1[4], pts2[4];

	rect1.points(pts1);
	rect2.points(pts2);

	vector<Point2d> ptsd1{
		Point2d(pts1[0]),
		Point2d(pts1[1]),
		Point2d(pts1[2]),
		Point2d(pts1[3])
	};

	vector<Point2d> ptsd2{
		Point2d(pts2[0]),
		Point2d(pts2[1]),
		Point2d(pts2[2]),
		Point2d(pts2[3])
	};

	return CalcMinDisFromRects(ptsd1, ptsd2, marge_side);
}

double cvex::CalcMinDisFromRects(vector<Point2d> points1, vector<Point2d> points2, int marge_side) {

	vector<double> result;
	vector<pair<double, int>> segments;

	//rect2から見たrect1の距離
	for (int i = 0; i < points1.size(); i++) {
		pair<Point2d, Point2d> LinePara({ points1[i], points1[(i + 1) % points1.size() == 0 ? 0 : i + 1] });
		for (int j = 0; j < points2.size(); j++) {
			segments.push_back({ cv::norm(LinePara.first - LinePara.second), i * points2.size() + j });
			double min_dis = cvex::CalcPointToLineSegmentLength(points2[j], LinePara);
			result.push_back(min_dis);
		}
	}

	//結合したい辺(width or height)の指定があった場合
	if (marge_side != MARGE_SIDE_ALL && segments.size() == 16) {

		std::sort(segments.begin(), segments.end(), [](pair<double, int>& seg1, pair<double, int>& seg2)
			-> bool {return seg1.first < seg2.first; });

		if (marge_side == MARGE_SIDE_HEIGHT) {
			for (int i = 0; i < 8; i++) {
				result[segments[i].second] = 1e7;
			}
		} else if (marge_side == MARGE_SIDE_WIDTH) {
			for (int i = 8; i < 16; i++) {
				result[segments[i].second] = 1e7;
			}
		}
	}

	vector<double> distances;
	segments.clear();

	//rect1からみたrect2の距離
	for (int i = 0; i < points2.size(); i++) {
		pair<Point2d, Point2d> LinePara({ points2[i], points2[(i + 1) % points2.size() == 0 ? 0 : i + 1] });
		for (int j = 0; j < points1.size(); j++) {
			segments.push_back({ cv::norm(LinePara.first - LinePara.second), i * points1.size() + j });
			double min_dis = cvex::CalcPointToLineSegmentLength(points1[j], LinePara);

			distances.push_back(min_dis);
		}
	}

	//結合したい辺(width or height)の指定があった場合
	if (marge_side != MARGE_SIDE_ALL && segments.size() == 16) {

		std::sort(segments.begin(), segments.end(), [](pair<double, int>& seg1, pair<double, int>& seg2)
			-> bool {return seg1.first < seg2.first; });

		if (marge_side == MARGE_SIDE_HEIGHT) {
			for (int i = 0; i < 8; i++) {
				distances[segments[i].second] = 1e7;
			}
		} else if (marge_side == MARGE_SIDE_WIDTH) {
			for (int i = 8; i < 16; i++) {
				distances[segments[i].second] = 1e7;
			}
		}
	}

	std::copy(distances.begin(), distances.end(), std::back_inserter(result));

	return *min_element(result.begin(), result.end());
}

cv::Rect cvex::CalcBoundingBoxFromTwoRects(Rect rect1, Rect rect2) {

	std::vector<cv::Point> points = {
		rect1.tl(),/*左上座標*/
		rect1.br(),/*右下座標*/
		cv::Point(rect1.x + rect1.width, rect1.y),/*右上座標*/
		cv::Point(rect1.x, rect1.y + rect1.height),/*左下座標*/

		rect2.tl(),/*左上座標*/
		rect2.br(),/*右下座標*/
		cv::Point(rect2.x + rect2.width, rect2.y),/*右上座標*/
		cv::Point(rect2.x, rect2.y + rect2.height)/*左下座標*/
	};

	cv::Rect r = cv::boundingRect(points);
	r.width -= 1;
	r.height -= 1;

	return r;
}

cv::RotatedRect cvex::CalcBoundingBoxFromTwoRects(RotatedRect rect1, RotatedRect rect2) {

	cv::Point2f pts1[4], pts2[4];

	rect1.points(pts1);
	rect2.points(pts2);

	vector<Point2f> ptsf{
		pts1[0],pts1[1],pts1[2],pts1[3],
		pts2[0],pts2[1],pts2[2],pts2[3]
	};

	return cv::minAreaRect(ptsf);;
}

std::vector<cv::Rect> cvex::MargeBoundingBoxes(std::vector<cv::Rect> rects, double length_threshold, int marge_side) {

	if (rects.size() == 0) return rects;

	function <void(
		vector<cv::Rect>&,
		int&,
		vector<cv::Rect>&,
		int&,
		int&
		)> _marge = [](
		vector<Rect>& marge_rects, int& marge_num,
		vector<Rect>& erase_rects, int& erase_num,
		int& count
		)
		->void {
		marge_rects[marge_num] = cvex::CalcBoundingBoxFromTwoRects(marge_rects[marge_num], erase_rects[erase_num]);
		erase_rects.erase(erase_rects.begin() + erase_num);
		erase_num--;
		count++;
	};

	vector<Rect> marge_rects;
	int count = 0;

	do {
		count = 0;
		if (marge_rects.size() == 0) {
			for (int i = 0; i < rects.size(); i++) {
				bool marge_flag = false;

				for (int j = i + 1; j < rects.size(); j++) {
					double dis = cvex::CalcMinDisFromRects(rects[i], rects[j], marge_side);

					if (dis < length_threshold) {
						_marge(rects, i, rects, j, count);
						marge_flag = true;
					}
				}

				if (marge_flag) {
					marge_rects.push_back(rects[i]);
					rects.erase(rects.begin() + i);
					i--;
				}
			}
		} else {
			for (int i = 0; i < marge_rects.size(); i++) {
				for (int j = 0; j < rects.size(); j++) {
					double dis = cvex::CalcMinDisFromRects(marge_rects[i], rects[j], marge_side);

					if (dis < length_threshold) {
						_marge(marge_rects, i, rects, j, count);
					}
				}

				for (int j = i + 1; j < marge_rects.size(); j++) {
					double dis = cvex::CalcMinDisFromRects(marge_rects[i], marge_rects[j], marge_side);

					if (dis < length_threshold) {
						_marge(marge_rects, i, marge_rects, j, count);
					}
				}
			}
		}
	} while (count != 0);

	std::copy(marge_rects.begin(), marge_rects.end(), std::back_inserter(rects));

	return rects;
}

std::vector<cv::RotatedRect> cvex::MargeBoundingBoxes(std::vector<cv::RotatedRect> rects, double length_threshold, int marge_side) {

	if (rects.size() == 0) return rects;

	function <void(
		vector<cv::RotatedRect>&,
		int&,
		vector<cv::RotatedRect>&,
		int&,
		int&
		)> _marge = [](
		vector<RotatedRect>& marge_rects, int& marge_num,
		vector<RotatedRect>& erase_rects, int& erase_num,
		int& count)
		-> void {
		marge_rects[marge_num] = cvex::CalcBoundingBoxFromTwoRects(marge_rects[marge_num], erase_rects[erase_num]);
		erase_rects.erase(erase_rects.begin() + erase_num);
		erase_num--;
		count++;
	};

	vector<RotatedRect> marge_rects;
	int count = 0;

	do {
		count = 0;
		if (marge_rects.size() == 0) {
			for (int i = 0; i < rects.size(); i++) {
				bool marge_flag = false;

				for (int j = i + 1; j < rects.size(); j++) {
					double dis = cvex::CalcMinDisFromRects(rects[i], rects[j], marge_side);

					if (dis < length_threshold) {
						_marge(rects, i, rects, j, count);
						marge_flag = true;
					}
				}

				if (marge_flag) {
					marge_rects.push_back(rects[i]);
					rects.erase(rects.begin() + i);
					i--;
				}
			}
		} else {
			for (int i = 0; i < marge_rects.size(); i++) {
				for (int j = 0; j < rects.size(); j++) {
					double dis = cvex::CalcMinDisFromRects(marge_rects[i], rects[j], marge_side);

					if (dis < length_threshold) {
						_marge(marge_rects, i, rects, j, count);
					}
				}

				for (int j = i + 1; j < marge_rects.size(); j++) {
					double dis = cvex::CalcMinDisFromRects(marge_rects[i], marge_rects[j], marge_side);

					if (dis < length_threshold) {
						_marge(marge_rects, i, marge_rects, j, count);
					}
				}
			}
		}
	} while (count != 0);

	std::copy(marge_rects.begin(), marge_rects.end(), std::back_inserter(rects));

	return rects;
}

std::vector<cv::RotatedRect> cvex::MargeNearContours(std::vector<std::vector<cv::Point>>& contours, double length_threshold, int marge_side, bool re_create_minAreaRect) {

	if (contours.size() == 0)
		return vector<RotatedRect>();

	vector<RotatedRect> rects(contours.size());
	for (int i = 0; i < contours.size(); i++) {
		rects[i] = cv::minAreaRect(contours[i]);
	}

	function <void(
		vector<cv::RotatedRect>&,
		vector<vector<cv::Point>>&,
		int&,
		vector<cv::RotatedRect>&,
		vector<vector<cv::Point>>&,
		int&,
		int&
		)>_marge = [=](
		vector<RotatedRect>& marge_rects, vector<vector<Point>>& marge_contours, int& marge_num,
		vector<RotatedRect>& erase_rects, vector<vector<Point>>& erase_contours, int& erase_num,
		int& count)
		-> void {
		std::copy(erase_contours[erase_num].begin(), erase_contours[erase_num].end(), std::back_inserter(marge_contours[marge_num]));

		if (!re_create_minAreaRect) marge_rects[marge_num] = cvex::CalcBoundingBoxFromTwoRects(marge_rects[marge_num], erase_rects[erase_num]);
		else marge_rects[marge_num] = cv::minAreaRect(marge_contours[marge_num]);

		erase_rects.erase(erase_rects.begin() + erase_num);
		erase_contours.erase(erase_contours.begin() + erase_num);
		erase_num--;
		count++;
	};

	vector<RotatedRect> marge_rects;
	vector<vector<Point>> marge_contours;

	int count = 0;
	do {
		count = 0;
		if (marge_rects.size() == 0) {
			for (int i = 0; i < rects.size(); i++) {
				bool marge_flag = false;

				for (int j = i + 1; j < rects.size(); j++) {
					double dis = cvex::CalcMinDisFromRects(rects[i], rects[j], marge_side);

					if (dis < length_threshold) {
						_marge(rects, contours, i, rects, contours, j, count);
						marge_flag = true;
					}
				}

				if (marge_flag) {
					marge_rects.push_back(rects[i]);
					marge_contours.push_back(contours[i]);
					rects.erase(rects.begin() + i);
					contours.erase(contours.begin() + i);
					i--;
				}
			}
		} else {
			for (int i = 0; i < marge_rects.size(); i++) {
				for (int j = 0; j < rects.size(); j++) {
					double dis = cvex::CalcMinDisFromRects(marge_rects[i], rects[j], marge_side);

					if (dis < length_threshold) {
						_marge(marge_rects, marge_contours, i, rects, contours, j, count);
					}
				}

				for (int j = i + 1; j < marge_rects.size(); j++) {
					double dis = cvex::CalcMinDisFromRects(marge_rects[i], marge_rects[j], marge_side);

					if (dis < length_threshold) {
						_marge(marge_rects, marge_contours, i, marge_rects, marge_contours, j, count);
					}
				}
			}
		}
	} while (count != 0);

	std::copy(marge_rects.begin(), marge_rects.end(), std::back_inserter(rects));
	std::copy(marge_contours.begin(), marge_contours.end(), std::back_inserter(contours));

	return rects;
}

void cvex::ImagesShow(string WindowName, vector<pair<Mat, string>> ProcessImages) {

	static int count = -1;
	static int show[10] = { 0 }, previous[10] = { 0 };
	static vector<vector<pair<Mat, string>>> Processes;
	static vector<string> WindowNames;
	count++;
	Processes.push_back(ProcessImages);
	WindowNames.push_back(WindowName);

	cv::namedWindow(WindowName, cv::WINDOW_AUTOSIZE);
	cv::createTrackbar("show", WindowName, &show[count], int(ProcessImages.size()) - 1, [](int pos, void* data) -> void {
		int LabelNo = 0;
		for (int i = 0; i <= count; i++) {
			if (show[i] != previous[i]) {
				LabelNo = i;
			}
		}

		cv::Mat Pallet = Processes[LabelNo][show[LabelNo]].first;
		cv::putText(Pallet, Processes[LabelNo][show[LabelNo]].second, cv::Point(10, 30),
			cv::FONT_HERSHEY_TRIPLEX, 0.5, cv::Scalar(255, 255, 255), 1);
		cv::imshow(WindowNames[LabelNo], Pallet);

		previous[LabelNo] = pos;
		});

}

void cvex::VideoWrite(string video_name, vector<Mat>& images, bool isColor) {
	int fourcc;
	if (isColor) {
		fourcc = cv::VideoWriter::fourcc('I', '4', '2', '0');
	} else {
		fourcc = cv::VideoWriter::fourcc('F', 'F', 'V', '1');
	}

	cv::VideoWriter VideoOut;
	VideoOut.open(video_name, fourcc, 60, images[0].size(), isColor);

	if (!VideoOut.isOpened()) {
		std::cout << "Cannot Open VideoWriter" << std::endl;
	}

	for (int i = 0; i < images.size(); i++) {
		cv::Mat Src = images[i].clone();

		if (isColor && images[0].channels() == 1) {
			cv::cvtColor(Src, Src, cv::COLOR_GRAY2BGR);
		} else if (!isColor && images[0].channels() == 3) {
			cv::cvtColor(Src, Src, cv::COLOR_BGR2GRAY);
		}

		VideoOut << Src;
	}

	VideoOut.release();
}

void cvex::VideoWrite(string video_name, vector<Mat>& images, double fps, bool isColor) {
	int fourcc;
	if (isColor) {
		fourcc = cv::VideoWriter::fourcc('I', '4', '2', '0');
	} else {
		fourcc = cv::VideoWriter::fourcc('F', 'F', 'V', '1');
	}

	cv::VideoWriter VideoOut;
	VideoOut.open(video_name, fourcc, fps, images[0].size(), isColor);

	if (!VideoOut.isOpened()) {
		std::cout << "Cannot Open VideoWriter" << std::endl;
	}

	for (int i = 0; i < images.size(); i++) {
		cout << "frame:" << i << endl;
		cv::Mat Src = images[i].clone();

		if (isColor && images[0].channels() == 1) {
			cv::cvtColor(Src, Src, cv::COLOR_GRAY2BGR);
		} else if (!isColor && images[0].channels() == 3) {
			cv::cvtColor(Src, Src, cv::COLOR_BGR2GRAY);
		}

		VideoOut << Src;
	}

	VideoOut.release();
}

std::wstring StringToWString(std::string oString) {
	// SJIS → wstring
	int iBufferSize = MultiByteToWideChar(CP_ACP, 0, oString.c_str(), -1, (wchar_t*)NULL, 0);
	// バッファの取得
	wchar_t* cpUCS2 = new wchar_t[iBufferSize];
	// SJIS → wstring
	MultiByteToWideChar(CP_ACP, 0, oString.c_str(), -1, cpUCS2, iBufferSize);
	// stringの生成
	std::wstring oRet(cpUCS2, cpUCS2 + iBufferSize - 1);
	// バッファの破棄
	delete[] cpUCS2;
	// 変換結果を返す
	return(oRet);
}
std::string TWStringToString(std::wstring arg_wstr) {
	//// 文字数
	//size_t length = arg_wstr.size();
	//// 変換後文字数
	//size_t convLength = 0;
	//// 仮の受け皿を用意（文字数×2ありゃ十分でしょう）
	//char* c = (char*)malloc(sizeof(char) * length * 2);
	//// 変換
	//wcstombs_s(&convLength, c, sizeof(char) * length * 2, arg_wstr.c_str(), length * 2);
	//// 返り値へ保存
	//std::string result(c);
	//// 仮の受け皿を破棄
	//free(c);
	size_t length = arg_wstr.size();
	size_t convLength = 0;
	string result;
	result.resize(length * 2);
	wcstombs_s(&convLength, &result[0], result.size(), arg_wstr.c_str(), length * 2);
	result.resize(convLength - 1);

	return(result);

};
// INPUT_FOLDER_NAMEのフォルダ内にある画像名を取得する
//vector<string> cvex::getImageName(string dir_name) {
//	HANDLE hFind;
//	WIN32_FIND_DATA win32fd;
//	std::vector<std::string> file_names;
//
//	// png,jpg,bmpの拡張子のファイルのみを読み込む
//	std::string extension[3] = { "bmp", "jpg", "avi" };
//
//	for (int i = 0; i < 3; i++) {
//
//		string search_name = dir_name + "*." + extension[i];
//		//wstring Wsearch_name = StringToWString(search_name);
//
//		hFind = FindFirstFile(search_name.c_str(), &win32fd);
//
//		if (hFind == INVALID_HANDLE_VALUE) {
//			continue;
//		}
//		do {
//			if (win32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
//			} else {
//				//file_names.push_back(TWStringToString(win32fd.cFileName));
//				file_names.push_back(win32fd.cFileName);
//			}
//		} while (FindNextFile(hFind, &win32fd));
//
//		FindClose(hFind);
//	}
//	return file_names;
//}
//
//vector<string> cvex::getFileNamesAll(string dir_name, vector<string> extension) {
//
//	vector<string> FileNames;
//
//	bool all_flag = std::find_if(extension.begin(), extension.end(), [](string ext)
//		-> bool {return ext == "ALL" || ext == "all"; }) != extension.end();
//
//	for (const auto& file : std::filesystem::recursive_directory_iterator(dir_name)) {
//		string FileName = file.path().string();
//
//		if (!all_flag) {
//			if (!file.is_directory()) {
//				for (string ext : extension) {
//					if (FileName.substr(FileName.size() - ext.size(), ext.size()) == ext) {
//						FileNames.push_back(FileName);
//					}
//				}
//			}
//		} else {
//			if (!file.is_directory()) {
//				FileNames.push_back(FileName);
//			}
//		}
//	}
//
//	return FileNames;
//}
//
//vector<string> cvex::getDirNameList(string dir_name) {
//
//	vector<string> dir_list;
//
//	for (const std::filesystem::directory_entry& file : std::filesystem::directory_iterator(dir_name)) {
//		if (file.is_directory()) {
//			dir_list.push_back(file.path().filename().string());
//		}
//	}
//
//	return dir_list;
//}

//void cvex::WriteImages(vector<pair<Mat, string>> ProcessImages, string FilePath) {
//
//	if (ProcessImages.size() == 0) {
//		cout << "ProcessImages.size == 0" << endl;
//		return;
//	}
//
//	std::string WriteProcessImagesPath = FilePath;
//	if (FilePath.substr(FilePath.size() - 1, 1) == "/") {
//		WriteProcessImagesPath += "ProcessImages";
//	} else {
//		WriteProcessImagesPath += "/ProcessImages";
//	}
//	bool CreateFlag = std::filesystem::create_directories(WriteProcessImagesPath);
//
//	for (int i = 0; i < ProcessImages.size(); i++) {
//
//		if (ProcessImages[i].second == "") {
//			ProcessImages[i].second = to_string(i) + "NoName";
//		}
//
//		cv::imwrite(WriteProcessImagesPath + "/" + to_string(i) + ProcessImages[i].second + ".bmp", ProcessImages[i].first);
//	}
//
//}

void cvex::ToStripe(cv::Mat Src, cv::Mat& Dst, bool horizontal, bool vertical, int StripeMode, int StripeRange) {

	if (horizontal == vertical) {
		cout << "ToStripe Err" << endl;
		Dst = Src;
		return;
	}

	switch (StripeMode) {

	case STRIPE_AVERAGE:

		if (horizontal) {
			cv::Mat reduce_out;
			cv::reduce(Src, reduce_out, 1, cv::REDUCE_AVG);
			cv::repeat(reduce_out, 1, Src.cols, Dst);
		} else {
			cv::Mat reduce_out;
			cv::reduce(Src, reduce_out, 0, cv::REDUCE_AVG);
			cv::repeat(reduce_out, Src.rows, 1, Dst);
		}
		break;

	case STRIPE_MAX:

		if (horizontal) {
			cv::Mat reduce_out;
			cv::reduce(Src, reduce_out, 1, cv::REDUCE_MAX);
			cv::repeat(reduce_out, 1, Src.cols, Dst);
		} else {
			cv::Mat reduce_out;
			cv::reduce(Src, reduce_out, 0, cv::REDUCE_MAX);
			cv::repeat(reduce_out, Src.rows, 1, Dst);
		}
		break;

	case STRIPE_MIN:

		if (horizontal) {
			cv::Mat reduce_out;
			cv::reduce(Src, reduce_out, 1, cv::REDUCE_MIN);
			cv::repeat(reduce_out, 1, Src.cols, Dst);
		} else {
			cv::Mat reduce_out;
			cv::reduce(Src, reduce_out, 0, cv::REDUCE_MIN);
			cv::repeat(reduce_out, Src.rows, 1, Dst);
		}
		break;

	case STRIPE_MEDIAN:

		if (horizontal) {
			cv::Mat sort_out;
			cv::sort(Src, sort_out, cv::SORT_ASCENDING | cv::SORT_EVERY_ROW);
			cv::Mat reduce_out = sort_out.col(sort_out.cols / 2).clone();
			cv::repeat(reduce_out, 1, Src.cols, Dst);
		} else {
			cv::Mat sort_out;
			cv::sort(Src, sort_out, cv::SORT_ASCENDING | cv::SORT_EVERY_COLUMN);
			cv::Mat reduce_out = sort_out.row(sort_out.rows / 2).clone();
			cv::repeat(reduce_out, Src.rows, 1, Dst);
		}
	}
}

void cvex::StripeHist(cv::Mat Src, cv::Mat& Dst, bool horizontal, bool vertical, int brightness, cv::InputArray Mask) {

	if (horizontal == vertical) {
		cout << "ToStripe Err" << endl;
		Dst = Src;
		return;
	} else if (Src.channels() != 1) {
		cout << "Src.Channel Err. Src.Channel is " << Src.channels() << endl;
		Dst = Src;
		return;
	}

	cv::Mat mask;
	if (Mask.empty()) {
		mask = cv::Mat(Src.size(), CV_8UC1, 255);
	} else {
		mask = Mask.getMat();
	}

	Src.copyTo(Dst);

	if (horizontal) {

		for (int y = 0; y < Src.rows; y++) {
			float brightmean = float(cv::mean(Src.row(y), mask.row(y))[0]);
			cv::convertScaleAbs(Src.row(y), Dst.row(y), brightness / brightmean, 0);
		};

	} else {

		for (int x = 0; x < Src.cols; x++) {
			float brightmean = float(cv::mean(Src.col(x), mask.col(x))[0]);
			cv::convertScaleAbs(Src.col(x), Dst.col(x), brightness / brightmean, 0);
		};

	}
}

void cvex::StripeHistP(cv::Mat Src, cv::Mat& Dst, int Margin, bool horizontal, bool vertical, int borderType, int brightness) {
	
	if (horizontal == vertical) {
		cout << "Drection Err" << endl;
		Dst = Src;
		return;
	} else if (Src.channels() != 1) {
		cv::cvtColor(Src, Src, cv::COLOR_BGR2GRAY);
	}

	if (horizontal) {
		cv::Mat cpyMake;
		cv::copyMakeBorder(Src, cpyMake, 0, 0, Margin, Margin, borderType);

		cv::Mat stripe_hist = cv::Mat::zeros(cpyMake.size(), CV_8UC1);
		for (int x = Margin; x < cpyMake.cols - Margin; x++) {
			cv::Mat pallet;
			cvex::StripeHist(cpyMake.colRange(x - Margin, x + Margin + 1), pallet, horizontal, vertical, brightness);
			pallet.col(Margin).copyTo(stripe_hist.col(x));
		}

		stripe_hist.colRange(Margin, stripe_hist.cols - Margin).copyTo(Dst);
	} else {
		cv::Mat cpyMake;
		cv::copyMakeBorder(Src, cpyMake, Margin, Margin, 0, 0, borderType);

		cv::Mat stripe_hist = cv::Mat::zeros(cpyMake.size(), CV_8UC1);
		for (int y = Margin; y < cpyMake.rows - Margin; y++) {
			cv::Mat pallet;
			cvex::StripeHist(cpyMake.rowRange(y - Margin, y + Margin + 1), pallet, horizontal, vertical, brightness);
			pallet.row(Margin).copyTo(stripe_hist.row(y));
		}

		stripe_hist.rowRange(Margin, stripe_hist.rows - Margin).copyTo(Dst);
	}
}

void cvex::CorrectMean(cv::Mat Src, cv::Mat& Dst, int MeanVal, cv::InputArray Mask) {
	if (Src.empty()) return;

	if (Src.channels() == 3) {
		cv::cvtColor(Src, Src, cv::COLOR_BGR2GRAY);
	}

	cv::Mat mask = Mask.getMat();
	if (mask.empty()) {
		mask = cv::Mat(Src.size(), CV_8UC1, 255);
	} else if (mask.channels() == 3) {
		cv::cvtColor(mask, mask, cv::COLOR_BGR2GRAY);
	}

	double mean = cv::mean(Src, mask)[0];
	cv::convertScaleAbs(Src, Dst, MeanVal / mean, 0);
}

void cvex::CorrectMeanP(cv::Mat Src, cv::Mat& Dst, int Margin, bool TargetIsWidth,
	int brightness, int border_type, cv::InputArray Mask) {
	if (Src.empty()) return;

	if (Src.channels() == 3) {
		cv::cvtColor(Src, Src, cv::COLOR_BGR2GRAY);
	}

	cv::Mat mask = Mask.getMat();
	if (mask.empty()) {
		mask = cv::Mat(Src.size(), CV_8UC1, 255);
	} else if (mask.channels() == 3) {
		cv::cvtColor(mask, mask, cv::COLOR_BGR2GRAY);
	}

	cv::Mat normalize_out;
	if (TargetIsWidth) {
		cv::Mat cpyMake, cpyMakeMask;
		cv::copyMakeBorder(Src, cpyMake, 0, 0, Margin, Margin, border_type);
		cv::copyMakeBorder(mask, cpyMakeMask, 0, 0, Margin, Margin, border_type);

		normalize_out = cv::Mat::zeros(cpyMake.size(), CV_8UC1);
		for (int x = Margin; x < normalize_out.cols - Margin; x++) {
			double mean = cv::mean(cpyMake.colRange(x - Margin, x + Margin + 1), cpyMakeMask.colRange(x - Margin, x + Margin + 1))[0];
			cv::convertScaleAbs(cpyMake.col(x), normalize_out.col(x), 128 / mean, 0);
		}

		normalize_out.colRange(Margin, normalize_out.cols - Margin).copyTo(normalize_out);
	} else {
		cv::Mat cpyMake, cpyMakeMask;
		cv::copyMakeBorder(Src, cpyMake, Margin, Margin, 0, 0, cv::BORDER_WRAP);
		cv::copyMakeBorder(mask, cpyMakeMask, Margin, Margin, 0, 0, cv::BORDER_WRAP);

		normalize_out = cv::Mat::zeros(cpyMake.size(), CV_8UC1);
		for (int y = Margin; y < normalize_out.rows - Margin; y++) {
			double mean = cv::mean(cpyMake.rowRange(y - Margin, y + Margin + 1), cpyMakeMask.rowRange(y - Margin, y + Margin + 1))[0];
			cv::convertScaleAbs(cpyMake.row(y), normalize_out.row(y), 128 / mean, 0);
		}

		normalize_out.rowRange(Margin, normalize_out.cols - Margin).copyTo(normalize_out);
	}

	normalize_out.copyTo(Dst);
}

void cvex::CreateBrightnessGraph(cv::Mat Src, cv::Mat& Dst, bool horizontal, bool vertical) {

	if (horizontal == vertical) {
		cout << "Drection Err" << endl;
		Dst = Src;
		return;
	} else if (Src.channels() != 1) {
		cv::cvtColor(Src, Src, cv::COLOR_BGR2GRAY);
	}

	if (horizontal) {

		Dst = cv::Mat::zeros(cv::Size(256, Src.rows), CV_8UC1);

		for(int y=0; y<Src.rows;y++){
			float brightmean = float(cv::mean(Src.row(y))[0]);
			cv::line(Dst, cv::Point(0, y), cv::Point(int(brightmean), y), 255, 1);
			};

	} else {

		Dst = cv::Mat::zeros(cv::Size(Src.cols, 256), CV_8UC1);

		for(int x=0; x<Src.cols; x++) {
			float brightmean = float(cv::mean(Src.col(x))[0]);
			cv::line(Dst, cv::Point(x, 255), cv::Point(x, 255 - int(brightmean)), 255, 1);
			};
	}

}

void cvex::BilateralRoop(cv::Mat Src, cv::Mat& Dst, int d, double Color, double Space, int Roop) {

	if (Src.empty()) {
		cout << "Src is empty" << endl;
		Dst = cv::Mat::zeros(cv::Size(100, 100), CV_8UC1);
	}

	cv::bilateralFilter(Src, Dst, d, Color, Space);
	for (int i = 0; i < Roop; i++) {
		cv::bilateralFilter(Dst.clone(), Dst, d, Color, Space);
	}

}

void cvex::DFTConvert(cv::Mat Src, cv::Mat& comp_img, cv::Mat& pow_spc) {

	//Srcと同じサイズで、要素の値が全てゼロの配列を用いる
	cv::Mat planes[] = { cv::Mat_<float>(Src), cv::Mat::zeros(Src.size(), CV_32F) };

	//2つの配列planesを結合し、複素数の配列com_imgを作る。
	cv::merge(planes, 2, comp_img);

	//フーリエ変換
	cv::dft(comp_img, comp_img);

	//複素数配列を分解 palnes[0] = 実部(comp_img), planes[1] = 虚数部(comp_img)
	cv::split(comp_img, planes);

	//振幅を算出し、パワースペクトルの変数(pow_spc)に代入
	cv::magnitude(planes[0], planes[1], planes[0]);
	pow_spc = planes[0].clone();

	//パワースペクトルの値を対数スケールに変換
	// => log(1 + sqrt(Re(DFT(I))^2 + Im(DFT(I))^2
	pow_spc += cv::Scalar::all(1);

	//pow_spcの画像をlogスケールに変換する
	cv::log(pow_spc, pow_spc);

	//高額フーリエ変換のパワースペクトルに変換
	int cx = pow_spc.cols / 2;
	int cy = pow_spc.rows / 2;

	cv::Mat q0 = cv::Mat(pow_spc, cv::Rect(0, 0, cx, cy));
	cv::Mat q1 = cv::Mat(pow_spc, cv::Rect(cx, 0, cx, cy));
	cv::Mat q2 = cv::Mat(pow_spc, cv::Rect(0, cy, cx, cy));
	cv::Mat q3 = cv::Mat(pow_spc, cv::Rect(cx, cy, cx, cy));

	cv::Mat tmp;
	//各象限の値を入れ替える
	q0.copyTo(tmp);
	q3.copyTo(q0);
	tmp.copyTo(q3);

	q1.copyTo(tmp);
	q2.copyTo(q1);
	tmp.copyTo(q2);
}

void cvex::IDFTConvert(cv::Mat Src, cv::Mat& Dst, cv::Mat SpectrumMask) {

	if (SpectrumMask.empty()) {
		cv::idft(Src, Dst, cv::DFT_SCALE | cv::DFT_REAL_OUTPUT);
		Dst.convertTo(Dst, CV_8UC1);
		return;
	}

	if (Src.channels() != 2) {
		cout << "Src Channels != 2" << endl;
		return;
	} else if (Src.cols != SpectrumMask.cols || Src.rows != SpectrumMask.rows) {
		cout << "Src.size != SpectrumMask.size" << endl;
		return;
	}

	cv::Mat spectrum_mask = SpectrumMask.clone();

	int cx = spectrum_mask.cols / 2;
	int cy = spectrum_mask.rows / 2;

	cv::Mat q0 = cv::Mat(spectrum_mask, cv::Rect(0, 0, cx, cy));
	cv::Mat q1 = cv::Mat(spectrum_mask, cv::Rect(cx, 0, cx, cy));
	cv::Mat q2 = cv::Mat(spectrum_mask, cv::Rect(0, cy, cx, cy));
	cv::Mat q3 = cv::Mat(spectrum_mask, cv::Rect(cx, cy, cx, cy));

	cv::Mat tmp;

	q0.copyTo(tmp);
	q3.copyTo(q0);
	tmp.copyTo(q3);

	q1.copyTo(tmp);
	q2.copyTo(q1);
	tmp.copyTo(q2);

	cv::Mat Image;
	Src.copyTo(Image, spectrum_mask);

	// 逆 DFT 
	cv::idft(Image, Dst, cv::DFT_INVERSE | cv::DFT_SCALE | cv::DFT_REAL_OUTPUT);
	Dst.convertTo(Dst, CV_8UC1);
}

cv::Mat cvex::ReviewDFT(cv::Mat Src, bool DebugFlag, string FilePath) {

	PParameter pPara = PParameter(new Parameter());

	static Parameter::TrackParas tp2, tp = []() -> Parameter::TrackParas {

		tp.LineX = 30;
		tp.LineY = 0;
		tp.brightness = 255;

		return tp;
	}();

	static double ReBrightness = -29043.12891;
	static double ImBrightness = -100556;

	if (DebugFlag) {

		cv::namedWindow("TrackbarWindow1", cv::WINDOW_AUTOSIZE);
		cv::resizeWindow("TrackbarWindow1", cv::Size(700, 1000));

		Track("LineX", 1, tp.LineX, Src.cols - 1, NULL);
		Track("LineY", 1, tp.LineY, Src.rows - 1, NULL);

		Track("scroll", 1, tp.scroll, 0, NULL);
		Track("show", 1, tp.show, 0, NULL);
	}

	cv::Mat iDFTOut;

	do {

		cv::Mat Pallet = cv::Mat::zeros(Src.size(), CV_32FC2);
		cv::Mat Pallet2 = cv::Mat::zeros(Src.size(), CV_8UC3);

		Pallet.ptr<Vec2f>(tp.LineY)[tp.LineX] = { float(ReBrightness), float(ImBrightness) };
		Pallet.ptr<Vec2f>(tp.LineY)[Src.cols - tp.LineX] = { float(ReBrightness), float(ImBrightness) };

		cv::circle(Pallet2, cv::Point(tp.LineX, tp.LineY), 2, cv::Scalar(255, 255, 255), -1);
		cv::circle(Pallet2, cv::Point(Src.cols - tp.LineX, tp.LineY), 2, cv::Scalar(255, 255, 255), -1);

		// 逆 DFT 
		cv::idft(Pallet, iDFTOut, cv::DFT_INVERSE | cv::DFT_REAL_OUTPUT);  // 逆DFT計算

		cv::Mat VStripe, HStripe;
		VStripe = cv::Mat::zeros(cv::Size(iDFTOut.cols, 510), CV_8UC1);
		HStripe = cv::Mat::zeros(cv::Size(510, iDFTOut.rows), CV_8UC1);

		for (int y = 0; y < iDFTOut.rows; y++) {
			cv::line(HStripe, cv::Point(0, y), cv::Point(int(iDFTOut.ptr<float>(y)[0] * 255) + 255, y), 255, 1);
			pPara->PutText("y:" + to_string(y) + ", brightness:" + to_string(iDFTOut.ptr<float>(y)[0]));
		}
		for (int x = 0; x < iDFTOut.cols; x++) {
			cv::line(VStripe, cv::Point(x, 0), cv::Point(x, int(iDFTOut.ptr<float>(0)[x] * 255) + 255), 255, 1);
		}

		cv::imshow("HStripe", HStripe);
		cv::imshow("VStripe", VStripe);

		if (DebugFlag) {
			vector<pair<Mat, string>> ProcessImages;
			ProcessImages.push_back({ Pallet2, "Pallet2" });
			ProcessImages.push_back({ iDFTOut, "iDFTOut" });
			ProcessImages.push_back({ Src, "Src" });

			cvex::ShowProcess(ProcessImages, tp, pPara);

			switch (cv::waitKey(1)) {

			case 's':
				DebugFlag = false;
				break;

			/*case 'w':

				cvex::WriteImages(ProcessImages, FilePath + "ReviewDFT/");*/

				break;

			}
		}

	} while (DebugFlag);

	return iDFTOut;
}

void cvex::CalcFrequency(string FileName) {

	vector<float> data;
	cv::Mat DataMat, DftOut, planes[2], SumSq, Spectrum, Filter, FilterOut, FilterOutPlanes[2], IDFTOut;

	//CSVファイル読込み、dataに格納
	FileSystem::read_csv(FileName, data);

	//実部, 虚部の２チャンネルfloat型のMat定義
	DataMat = cv::Mat(cv::Size(int(data.size()), 1), CV_32FC2);

	//データをMatに格納
	for (int i = 0; i < int(data.size()); i++) {
		DataMat.ptr<Vec2f>(0)[i] = cv::Vec2f(data[i], 0.0);
	}

	//フーリエ変換
	cv::dft(DataMat, DftOut);

	//実部、虚部のMatに分離
	cv::split(DftOut, planes);

	//スペクトル算出
	cv::magnitude(planes[0], planes[1], SumSq);
	SumSq += cv::Scalar::all(1);
	cv::log(SumSq, Spectrum);

	//直流成分の削除
	cv::Mat Spectrum2 = Spectrum.clone();
	Spectrum.ptr<float>(0)[0] = 0.0;

	//直流成分を除く最大スペクトルの抽出
	cv::Point MaxPos;
	cv::minMaxLoc(Spectrum, NULL, NULL, NULL, &MaxPos);

	//最大スペクトルのみ抽出するフィルターの作成
	Filter = cv::Mat::zeros(Spectrum.size(), CV_8UC1);
	Filter.at<uchar>(MaxPos) = 255;
	Filter.at<uchar>(cv::Point(Filter.cols - MaxPos.x, 0)) = 255;

	//マスク処理
	DftOut.copyTo(FilterOut, Filter);

	//CSV保存用にフィルタ後の実部、虚部Matを分離
	cv::split(FilterOut, FilterOutPlanes);

	//フーリエ逆変換
	cv::idft(FilterOut, IDFTOut, cv::DFT_SCALE | cv::DFT_REAL_OUTPUT);

	//格納データの作成
	vector<string> Labels = { "Value", "Re", "Im", "SumSq", "Spectorum", "Filter", "ORe", "OIm", "IDFT" };
	vector<tuple<float, float, float, float, float, int, float, float, float>> Data;

	for (int x = 0; x < DataMat.cols; x++) {

		Data.push_back({
			data[x],
			planes[0].ptr<float>(0)[x],
			planes[1].ptr<float>(0)[x],
			SumSq.ptr<float>(0)[x],
			Spectrum2.ptr<float>(0)[x],
			int(Filter.ptr<uchar>(0)[x] / 255),
			FilterOutPlanes[0].ptr<float>(0)[x],
			FilterOutPlanes[1].ptr<float>(0)[x],
			IDFTOut.ptr<float>(0)[x]
			});
	}

	//CSVファイルの保存
	FileSystem::write_csv("../../Sample/OutTest2", Labels, Data);

}

vector<vector<Point>> cvex::ExtractContours(vector<vector<Point>> Contours, int extractmode, bool MaxVal) {

	if (Contours.size() == 0) {
		return Contours;
	}

	decltype(Contours)::iterator itr;

	switch (extractmode) {

	case Extract_Area:
		if (MaxVal) {
			itr = std::max_element(Contours.begin(), Contours.end(), [](vector<Point>& contour1, vector<Point>& contour2)->bool {
				return cv::contourArea(contour1) < cv::contourArea(contour2);
				});
		} else {
			itr = std::min_element(Contours.begin(), Contours.end(), [](vector<Point>& contour1, vector<Point>& contour2)->bool {
				return cv::contourArea(contour1) < cv::contourArea(contour2);
				});
		}
		break;

	case Extract_R:
		if (MaxVal) {
			itr = std::max_element(Contours.begin(), Contours.end(), [](vector<Point>& contour1, vector<Point>& contour2)->bool {
				cv::Point2f center[2];
				float radius[2];
				cv::minEnclosingCircle(contour1, center[0], radius[0]);
				cv::minEnclosingCircle(contour2, center[1], radius[1]);
				return radius[0] < radius[1];
				});
		} else {
			itr = std::min_element(Contours.begin(), Contours.end(), [](vector<Point>& contour1, vector<Point>& contour2)->bool {
				cv::Point2f center[2];
				float radius[2];
				cv::minEnclosingCircle(contour1, center[0], radius[0]);
				cv::minEnclosingCircle(contour2, center[1], radius[1]);
				return radius[0] < radius[1];
				});
		}
		break;

	case Extract_arcLength:
		if (MaxVal) {
			itr = std::max_element(Contours.begin(), Contours.end(), [](vector<Point>& contour1, vector<Point>& contour2)->bool {
				return cv::arcLength(contour1, true) < cv::arcLength(contour2, true);
				});
		} else {
			itr = std::min_element(Contours.begin(), Contours.end(), [](vector<Point>& contour1, vector<Point>& contour2)->bool {
				return cv::arcLength(contour1, true) < cv::arcLength(contour2, true);
				});
		}
		break;

	case Extract_RRectHeight:
		if (MaxVal) {
			itr = std::max_element(Contours.begin(), Contours.end(), [](vector<Point>& contour1, vector<Point>& contour2)->bool {
				cv::RotatedRect rect0 = cv::minAreaRect(contour1);
				cv::RotatedRect rect1 = cv::minAreaRect(contour2);

				return max(rect0.size.height, rect0.size.width) < max(rect1.size.height, rect1.size.width);
				});
		} else {
			itr = std::min_element(Contours.begin(), Contours.end(), [](vector<Point>& contour1, vector<Point>& contour2)->bool {
				cv::RotatedRect rect0 = cv::minAreaRect(contour1);
				cv::RotatedRect rect1 = cv::minAreaRect(contour2);

				return max(rect0.size.height, rect0.size.width) < max(rect1.size.height, rect1.size.width);
				});
		}
		break;

	case Extract_CenterX:
		if (MaxVal) {
			itr = std::max_element(Contours.begin(), Contours.end(), [](vector<Point>& contour1, vector<Point>& contour2)->bool {
				return cv::minAreaRect(contour1).center.x < cv::minAreaRect(contour2).center.x;
				});
		} else {
			itr = std::min_element(Contours.begin(), Contours.end(), [](vector<Point>& contour1, vector<Point>& contour2)->bool {
				return cv::minAreaRect(contour1).center.x < cv::minAreaRect(contour2).center.x;
				});
		}
		break;

	case Extract_CenterY:
		if (MaxVal) {
			itr = std::max_element(Contours.begin(), Contours.end(), [](vector<Point>& contour1, vector<Point>& contour2)->bool {
				return cv::minAreaRect(contour1).center.y < cv::minAreaRect(contour2).center.y;
				});
		} else {
			itr = std::min_element(Contours.begin(), Contours.end(), [](vector<Point>& contour1, vector<Point>& contour2)->bool {
				return cv::minAreaRect(contour1).center.y < cv::minAreaRect(contour2).center.y;
				});
		}
		break;
	}

	vector<vector<Point>> ExContours{ *itr };

	return ExContours;
}

cv::Mat cvex::CreateHistGram(cv::Mat Src) {

	if (Src.empty()) {
		return cv::Mat::zeros(Src.size(), CV_8UC1);
	}

	if (Src.channels() == 1) {

		cv::Mat image_hist;
		cv::Mat Brightness;

		int hist_size = 256;
		float range[] = { 0, 256 };
		const float* hist_range = range;

		/* 画素数を数える */
		cv::calcHist(&Src, 1, 0, cv::Mat(), Brightness, 1, &hist_size, &hist_range);

		Brightness.at<float>(128) = 10;

		/* 正規化 */
		cv::normalize(Brightness, Brightness, 0.0, 1.0, NORM_MINMAX, -1, Mat());

		/* ヒストグラム生成用の画像を作成 */
		image_hist = cv::Mat(cv::Size(276, 320), CV_8UC1, 255);

		/* 背景を描画（見やすくするためにヒストグラムの部分の背景をグレーにする */
		cv::rectangle(image_hist, cv::Point(10, 10), cv::Point(265, 100), 230, -1);

		/* ヒストグラムを描画 */
		for (int i = 0; i < 256; i++) {
			cv::line(image_hist, cv::Point(10 + i, 100), cv::Point(10 + i, 100 - int(Brightness.at<float>(i) * 80)), 0, 1);

			//横軸10ずつラインを引く
			if (i % 10 == 0) {
				cv::line(image_hist, Point(10 + i, 100), cv::Point(10 + i, 10), 170, 1);
			}

			//横軸50ずつラインを引く
			if (i % 50 == 0) {
				cv::line(image_hist, Point(10 + i, 100), cv::Point(10 + i, 10), 50, 1);
			}

		}

		return image_hist;

	} else if (Src.channels() == 3) {

		Mat channels[3]; // チャンネル毎に分けた後に代入する変数
		split(Src, channels); // 入力画像を3チャンネルに分解

		Mat image_hist;
		Mat R, G, B;
		int hist_size = 256;
		float range[] = { 0, 256 };
		const float* hist_range = range;

		/* 画素数を数える */
		calcHist(&channels[0], 1, 0, Mat(), R, 1, &hist_size, &hist_range);
		calcHist(&channels[1], 1, 0, Mat(), G, 1, &hist_size, &hist_range);
		calcHist(&channels[2], 1, 0, Mat(), B, 1, &hist_size, &hist_range);

		/* 正規化 */
		normalize(R, R, 0.0, 1.0, NORM_MINMAX, -1, Mat());
		normalize(G, G, 0.0, 1.0, NORM_MINMAX, -1, Mat());
		normalize(B, B, 0.0, 1.0, NORM_MINMAX, -1, Mat());

		/* ヒストグラム生成用の画像を作成 */
		image_hist = Mat(Size(276, 320), CV_8UC3, Scalar(255, 255, 255));

		/* 背景を描画（見やすくするためにヒストグラム部分の背景をグレーにする） */
		for (int i = 0; i < 3; i++) {
			rectangle(image_hist, Point(10, 10 + 100 * i),
				Point(265, 100 + 100 * i), Scalar(230, 230, 230), -1);
		}

		/* ヒストグラムを描画 */
		for (int i = 0; i < 256; i++) {
			// それぞれのヒストグラムを描画
			line(image_hist, Point(10 + i, 100),
				Point(10 + i, 100 - (int)(R.at<float>(i) * 80)),
				Scalar(0, 0, 255), 1, 8, 0);

			line(image_hist, Point(10 + i, 200),
				Point(10 + i, 200 - (int)(G.at<float>(i) * 80)),
				Scalar(0, 255, 0), 1, 8, 0);

			line(image_hist, Point(10 + i, 300),
				Point(10 + i, 300 - (int)(G.at<float>(i) * 80)),
				Scalar(255, 0, 0), 1, 8, 0);


			// 横軸10ずつラインを引く
			if (i % 10 == 0) {
				line(image_hist, Point(10 + i, 100), Point(10 + i, 10),
					Scalar(170, 170, 170), 1, 8, 0);
				line(image_hist, Point(10 + i, 200), Point(10 + i, 110),
					Scalar(170, 170, 170), 1, 8, 0);
				line(image_hist, Point(10 + i, 300), Point(10 + i, 210),
					Scalar(170, 170, 170), 1, 8, 0);

				// 横軸50ずつ濃いラインを引く
				if (i % 50 == 0) {
					line(image_hist, Point(10 + i, 100), Point(10 + i, 10),
						Scalar(50, 50, 50), 1, 8, 0);
					line(image_hist, Point(10 + i, 200), Point(10 + i, 110),
						Scalar(50, 50, 50), 1, 8, 0);
					line(image_hist, Point(10 + i, 300), Point(10 + i, 210),
						Scalar(50, 50, 50), 1, 8, 0);
				}
			}
		}
		return image_hist;


	}
	else {
		return cv::Mat::zeros(Src.size(), CV_8UC1);
	}

}

void cvex::sauvolaFast(const cv::Mat& src, cv::Mat& dst, int kernelSize, double k, double r) {
	dst.create(src.size(), src.type());

	cv::Mat srcWithBorder;
	int borderSize = kernelSize / 2 + 1;
	int kernelPixels = kernelSize * kernelSize;
	cv::copyMakeBorder(src, srcWithBorder, borderSize, borderSize,
		borderSize, borderSize, cv::BORDER_REPLICATE);

	cv::Mat sum, sqSum;
	cv::integral(srcWithBorder, sum, sqSum);

	for(int y=0; y<src.rows; y++) {
		for (int x = 0; x < src.cols; x++) {
			int kx = x + kernelSize;
			int ky = y + kernelSize;
			double sumVal = sum.ptr<int>(ky)[kx]
				- sum.ptr<int>(ky)[x]
				- sum.ptr<int>(y)[kx]
				+ sum.ptr<int>(y)[x];
			double sqSumVal = sqSum.ptr<double>(ky)[kx]
				- sqSum.ptr<double>(ky)[x]
				- sqSum.ptr<double>(y)[kx]
				+ sqSum.ptr<double>(y)[x];

			double mean = sumVal / kernelPixels;
			double var = (sqSumVal / kernelPixels) - (mean * mean);
			if (var < 0.0)
				var = 0.0;
			double stddev = sqrt(var);
			double threshold = mean * (1 + k * (stddev / r - 1));

			if (src.ptr<uchar>(y)[x] < threshold)
				dst.ptr<uchar>(y)[x] = 0;
			else
				dst.ptr<uchar>(y)[x] = 255;
		}
		};
}

void cvex::GammaCorrect(cv::Mat Src, cv::Mat& Dst, double Gamma) {

	if (Src.empty()) {
		cout << "Src is empty" << endl;
		return;
	}

	Src.copyTo(Dst);

	if (Dst.channels() == 1) {

		Dst.forEach<uchar>([Gamma](uchar& p, const int position[2]) -> void {
			p = static_cast<uchar>(pow(double(p) / 255.0, Gamma) * 255.0);
			});

	} else {

		vector<Mat> planes;
		cv::split(Dst, planes);

		for (int i = 0; i < planes.size(); i++) {

			planes[i].forEach<uchar>([Gamma](uchar& p, const int position[2]) -> void {
				p = static_cast<uchar>(pow(double(p) / 255.0, Gamma) * 255.0);
				});

		}

		cv::merge(planes, Dst);

	}

}

void cvex::DoGFilter(cv::Mat Src, cv::Mat& Dst, cv::Size GaussSize, double SigmaX, double SigmaY, double SigmaRatio) {

	if (Src.empty()) {
		cout << "Src is emtpy" << endl;
		Src.copyTo(Dst);
		return;
	}

	cv::Mat GaussianOut, GaussianOut2;
	cv::GaussianBlur(Src, GaussianOut, GaussSize, SigmaX, SigmaY);
	cv::GaussianBlur(Src, GaussianOut2, GaussSize, SigmaX * SigmaRatio, SigmaY * SigmaRatio);

	Dst = GaussianOut - GaussianOut2 + 128;
}

void cvex::DoGFilter(cv::Mat Src, cv::Mat& Dst, int GaussianSize, double Sigma) {

	cv::Mat gaussian_out;
	cv::Size block = cv::Size(GaussianSize, GaussianSize);
	cv::GaussianBlur(Src, gaussian_out, block, Sigma, Sigma);
	cv::GaussianBlur(gaussian_out, gaussian_out, block, Sigma, Sigma);
	cv::GaussianBlur(gaussian_out, gaussian_out, block, Sigma, Sigma);
	cv::GaussianBlur(gaussian_out, gaussian_out, block, Sigma, Sigma);
	Dst = cv::Mat::zeros(Src.size(), CV_8UC1);

	for (int y = 0; y < gaussian_out.rows; y++) {
		for (int x = 0; x < gaussian_out.cols; x++) {
			int p = (int)Src.ptr<uchar>(y)[x] - (int)gaussian_out.ptr<uchar>(y)[x] + 128;
			if (p < 0) {
				p = 0;
			} else if (p > 255) {
				p = 255;
			}
			Dst.ptr<uchar>(y)[x] = saturate_cast<uchar>(p);
		}
	}
}

cv::Point2f cvex::CalcMoments(vector<Point>& Contour) {

	Moments mu = moments(Contour);
	cv::Point2f mc = cv::Point2f(float(mu.m10 / mu.m00), float(mu.m01 / mu.m00));

	return mc;
}

double cvex::CalcCircleLevel(vector<Point>& Contour) {

	double Length = cv::arcLength(Contour, true);
	double Area = cv::contourArea(Contour);

	return 4 * M_PI * Area / (Length * Length);
}

void cvex::DrawDimensionLine(cv::Mat& Src, cv::Point pos1, cv::Point pos2, int Horizontal, int Vertical, int index, double correction) {

	if (Vertical) {



	} else if (Horizontal) {

		int DiffPix = abs(pos1.x - pos2.x);
		double Length = double(DiffPix) * correction;

		cv::line(Src, pos1, cv::Point(pos1.x, index - Sign(pos1.y - index) * 5), cv::Scalar(0, 0, 255), 1);
		cv::line(Src, pos2, cv::Point(pos2.x, index - Sign(pos2.y - index) * 5), cv::Scalar(0, 0, 255), 1);
		cv::line(Src, cv::Point(pos1.x, index), cv::Point(pos2.x, index), cv::Scalar(0, 0, 255), 1);

		cv::arrowedLine(Src, cv::Point(pos1.x - Sign(DiffPix - 50) * 50, index), cv::Point(pos1.x, index), cv::Scalar(0, 0, 255));
		cv::arrowedLine(Src, cv::Point(pos2.x + Sign(DiffPix - 50) * 50, index), cv::Point(pos2.x, index), cv::Scalar(0, 0, 255));

	}
}

void cvex::Standardization(cv::Mat Src, cv::Mat& Dst, double designed_mean, double designated_std_dev, cv::InputArray Mask) {
	
	cv::Mat mask = Mask.getMat();
	if (mask.empty()) {
		mask = cv::Mat(Src.size(), CV_8UC1, 255);
	}
	
	cv::Scalar mean, std_dev;
	cv::meanStdDev(Src, mean, std_dev, mask);
	Src.copyTo(Dst);

	Dst.forEach<uchar>([mean, std_dev, designed_mean, designated_std_dev](uchar& p, const int position[2]) -> void {
		p = cv::saturate_cast<uchar>((p - mean[0]) / std_dev[0] * designated_std_dev + designed_mean);
		});
}

void cvex::RangeNormalization(cv::Mat Src, cv::Mat& Dst, int min, int max) {

	if (min >= max) {
		min = max - 1;
	}

	double dAlpha = 255 / (double)(255 - 50);
	double dBeta = -255 * 50 / (double)(255 - 50);
	cv::Mat matTemp;
	Src.convertTo(matTemp, CV_32F, dAlpha, dBeta);
	cv::threshold(matTemp, matTemp, 0, 0, cv::THRESH_TOZERO);
	cv::threshold(matTemp, matTemp, 255, 255, cv::THRESH_TRUNC);
	
	matTemp.convertTo(Dst, Src.type());

	//Dst = cv::Mat::zeros(Src.size(), Src.type());
	//int point1, point;

	//for (int x = 0; x < Src.cols; x++) {//src.GetWidth()
	//	for (int y = 0; y < Src.rows; y++) {//src.GetHeight()

	//		point1 = int(255 * (Src.at<uchar>(cv::Point(x, y)) - min) / (max - min));
	//		if (point1 <= 0) {
	//			point = 0;
	//		} else if (point1 >= 255) {
	//			point = 255;
	//		} else {
	//			point = point1;
	//		}
	//		Dst.at<uchar>(cv::Point(x, y)) = point;
	//	}
	//}
}

cv::Rect cvex::MakeSuitableRect(cv::Rect rect, cv::Mat Src, bool IsConstSize, bool IsShiftable) {

	/**********************************************
	*
	* 矩形を画像内に納める
	*
	* 可変矩形 or 固定矩形
	* 移動可能 or 固定位置
	*
	* に分けて行う(可変矩形は固定位置のみ)
	*
	***********************************************/

	cv::Rect src_rect(0, 0, Src.cols, Src.rows);

	if (rect.width <= 0 || rect.height <= 0) return cv::Rect();

	if (!IsConstSize) {

		//可変サイズ矩形

		if (rect.x > src_rect.br().x ||
			rect.y > src_rect.br().y ||
			rect.br().x < 0 ||
			rect.br().y < 0) {
			return cv::Rect();
		}

		if (rect.x < 0) {
			rect = cv::Rect(cv::Point(0, rect.y), rect.br());
		}

		if (rect.y < 0) {
			rect = cv::Rect(cv::Point(rect.x, 0), rect.br());
		}

		if (rect.br().x > src_rect.br().x) {
			rect = cv::Rect(rect.tl(), cv::Point(src_rect.br().x, rect.br().y));
		}

		if (rect.br().y > src_rect.br().y) {
			rect = cv::Rect(rect.tl(), cv::Point(rect.br().x, src_rect.br().y));
		}

		if (rect.area() == 0) return cv::Rect();

	} else if (!IsShiftable) {

		//固定サイズ矩形かつ移動も不可

		if (!src_rect.contains(rect.tl())) return cv::Rect();
		if (!src_rect.contains(cv::Point(rect.br().x, rect.y))) return cv::Rect();
		if (!src_rect.contains(rect.br())) return cv::Rect();
		if (!src_rect.contains(cv::Point(rect.x, rect.br().y))) return cv::Rect();

	} else {

		//固定サイズ矩形かつ移動は可能

		if (rect.width > src_rect.width ||
			rect.height > src_rect.height ||
			rect.x > src_rect.br().x ||
			rect.y > src_rect.br().y ||
			rect.br().x < 0 ||
			rect.br().y < 0) {
			return cv::Rect();
		}

		if (rect.x < 0) rect.x = 0;
		if (rect.y < 0)	rect.y = 0;
		if (rect.br().x > src_rect.br().x) {
			rect.x -= rect.br().x - src_rect.br().x;
		}
		if (rect.br().y > src_rect.br().y) {
			rect.y -= rect.br().y - src_rect.br().y;
		}
	}

	return rect;
}

void cvex::MakeSuitableRect(vector<Rect>& rects, cv::Mat Src, bool IsConstSize, bool IsShiftable) {

	vector<Rect> result;

	for (auto rect : rects) {
		cv::Rect suitable_rect = MakeSuitableRect(rect, Src, IsConstSize, IsShiftable);

		if (!suitable_rect.empty()) {
			result.push_back(suitable_rect);
		}

	}

	rects = result;
}

cv::Rect cvex::ConvertRect(cv::Rect rect, cv::Mat Src) {

	if (rect.width < rect.height) {
		int diff = rect.height - rect.width;
		rect.width = rect.height;
		rect.x -= diff / 2;
	} else {
		int diff = rect.width - rect.height;
		rect.height = rect.width;
		rect.y -= diff / 2;
	}

	//画像を超える矩形ならば合わせる
	if (rect.width > Src.cols - 10) {
		rect.width = Src.cols - 10;
		rect.height = Src.cols - 10;
	}

	if (rect.x < 0) {
		rect.x = 0;
	} else if (rect.br().x >= Src.cols) {
		rect.x -= rect.br().x - Src.cols;
	}

	if (rect.y < 0) {
		rect.y = 0;
	} else if (rect.br().y >= Src.rows) {
		rect.y -= rect.br().y - Src.rows;
	}

	return rect;
}

vector<cv::Rect> cvex::ConvertRects(vector<cv::Rect> rects, cv::Mat Src) {

	vector<Rect> squares;

	for (int i = 0; i < rects.size(); i++) {

		cv::Rect rect = rects[i];

		rect = cvex::ConvertRect(rect, Src);

		squares.push_back(rect);
	}

	return squares;
}

cv::Mat cvex::GaussianFilter(cv::Mat src, int block_size, double sigma) {

	cv::Mat result;

	if (block_size % 2 == 0) {
		block_size++;
	}
	cv::GaussianBlur(src, result, cv::Size(block_size, block_size), sigma, sigma);

	return result;
}

void cvex::DrawRect(cv::Mat& image, cv::Rect rect, cv::Scalar color, int thickness, int line_type, int gap, int linewidth) {
	switch (line_type) {
	case cvexLine::SOLID_LINE:
		cv::rectangle(image, rect, color, thickness);
		break;

	default:
		if (thickness <= 0) break;

		vector<Point> poses{
			{ rect.tl() },
			{ rect.br().x - 1, rect.tl().y },
			{ rect.br() - cv::Point(1, 1)},
			{ rect.tl().x, rect.br().y - 1 },
		};

		for (int i = 0; i < 4; i++) {
			cv::Point2d start = poses[i];
			cv::Point2d end = poses[i + 1 == 4 ? 0 : i + 1];
			cvexLine l(start, end);
			l.draw(image, start, end, color, thickness, line_type, gap, linewidth);
		}
		break;
	}
}

void cvex::DrawRotatedRect(cv::Mat& src, cv::RotatedRect rect, cv::Scalar color, int thickness, int line_type, int gap, int linewidth) {

	cv::Point2f pos[4];
	rect.points(pos);

	switch (line_type) {
	case cvexLine::SOLID_LINE:
	{
		if (thickness >= 0) {
			if (thickness == 0) thickness++;
			for (int i = 0; i < 4; i++) {
				cv::line(src, pos[i], pos[(i + 1) == 4 ? 0 : i + 1], color, thickness);
			}
		} else {
			vector<Point> points{ pos[0], pos[1], pos[2], pos[3] };
			cv::fillConvexPoly(src, points, color);
		}
		break;
	}

	default:
	{
		for (int i = 0; i < 4; i++) {
			cv::Point2d start = pos[i];
			cv::Point2d end = pos[i + 1 == 4 ? 0 : i + 1];
			cvexLine l(start, end);
			l.draw(src, start, end, color, thickness, line_type, gap, linewidth);
		}
	}
	}
}

cv::Point2d cvex::CalcRotatePos(cv::Point2d pos, cv::Point2d center, double degree) {

	double a = center.x;
	double b = center.y;
	double x = pos.x;
	double y = pos.y;
	double theta = degree * M_PI / 180;

	cv::Mat A = (cv::Mat_<double>(2, 2) <<
		cos(theta), -sin(theta),
		sin(theta), cos(theta));
	cv::Mat B = (cv::Mat_<double>(2, 1) <<
		x - a,
		y - b);
	cv::Mat C = (cv::Mat_<double>(2, 1) <<
		a,
		b);
	cv::Mat D = cv::Mat::zeros(2, 1, CV_64FC1);

	D = A * B + C;

	return cv::Point2d(D.at<double>(cv::Point(0, 0)), D.at<double>(cv::Point(0, 1)));
}

void cvex::SobelFilter(cv::Mat src, cv::Mat& dst, int size) {
	if (src.channels() != 1) {
		src.copyTo(dst);
		return;
	}

	cv::Mat sobel1, sobel2;
	cv::Sobel(src, sobel1, CV_64FC1, 1, 0, size);
	cv::Sobel(src, sobel2, CV_64FC1, 0, 1, size);

	sobel1.forEach<double>([=](double& p, const int pos[2]) {
		double p1 = p;
		double p2 = sobel2.ptr<double>(pos[0])[pos[1]];
		p = sqrt(p1 * p1 + p2 * p2) / sqrt(2);
		});
	cv::convertScaleAbs(sobel1, sobel1, 1, 0);

	sobel1.copyTo(dst);
}

void cvex::DrawBresenhamCircle(cv::Mat& src, cv::Point2d center, double radius, cv::Scalar color, int thickness) {

	cv::Mat Pallet = cv::Mat::zeros(src.size(), CV_8UC1);
	cv::Rect image_rect(0, 0, Pallet.cols, Pallet.rows);

	double cx = 0;
	double cy = radius;
	double d = 2 - 2 * radius;

	function<void(int,int)> draw_point = [&](int x, int y) {
		if (image_rect.contains(cv::Point(x, y))) {
			Pallet.ptr<uchar>(y)[x] = 255;
		}
	};

	draw_point((int)cx + int(center.x), (int)cy + int(center.y));
	draw_point((int)cx + int(center.x), -(int)cy + int(center.y));
	draw_point((int)cy + int(center.x), (int)cx + int(center.y));
	draw_point(-(int)cy + int(center.x), -(int)cx + int(center.y));

	while (1) {

		if (d > -cy) {
			--cy;
			d += 1 - 2 * cy;
		}

		if (d <= cx) {
			++cx;
			d += 1 + 2 * cx;
		}

		if (!cy) break;

		//描画
		draw_point((int)cx + int(center.x), (int)cy + int(center.y));
		draw_point(-(int)cx + int(center.x), (int)cy + int(center.y));
		draw_point(-(int)cx + int(center.x), -(int)cy + int(center.y));
		draw_point((int)cx + int(center.x), -(int)cy + int(center.y));
	}

	cv::Mat kernel = (cv::Mat_<uchar>(3, 3) <<
		0, 1, 0,
		1, 1, 1,
		0, 1, 0);
	cv::dilate(Pallet, Pallet, kernel, cv::Point(-1, -1), thickness - 1);

	if (src.channels() == 1) {
		src.forEach<uchar>([=](uchar& p, const int pos[2]) {
			if (Pallet.ptr<uchar>(pos[0])[pos[1]] == 255)
				p = uchar(color[0]);
			});
	} else if (src.channels() == 3) {
		src.forEach<Vec3b>([=](Vec3b& p, const int pos[2]) {
			if (Pallet.ptr<uchar>(pos[0])[pos[1]] == 255)
				p = Vec3b(uchar(color[0]), uchar(color[1]), uchar(color[2]));
			});
	}
}

string cvex::getDateTimeStr() {

	time_t t = std::time(nullptr);
	struct tm localTime;
	localtime_s(&localTime, &t);
	stringstream s;
	s << "20" << localTime.tm_year - 100;
	s << setw(2) << setfill('0') << localTime.tm_mon + 1;
	s << setw(2) << setfill('0') << localTime.tm_mday;
	s << setw(2) << setfill('0') << localTime.tm_hour;
	s << setw(2) << setfill('0') << localTime.tm_min;
	s << setw(2) << setfill('0') << localTime.tm_sec;

	return s.str();
}

double cvex::SSIM(cv::Mat src1, cv::Mat src2) {
	double mean_x = 0., mean_y = 0.;
	double var_x = 0., var_y = 0.;
	double covar_xy = 0.;
	double ssim = 0.;
	double c1 = (0.01 * 255) * (0.01 * 255);
	double c2 = (0.03 * 255) * (0.03 * 255);

	if (src1.channels() == 1 && src2.channels() == 1 && src1.size() == src2.size()) {
		double sum_x = 0., sum_x2 = 0., sum_y = 0., sum_y2 = 0., sum_xy = 0.;
		for (int y = 0; y < src1.rows; y++) {
			for (int x = 0; x < src1.cols; x++) {
				double val_x = (double)src1.ptr<uchar>(y)[x];
				double val_y = (double)src2.ptr<uchar>(y)[x];

				sum_x += val_x;
				sum_y += val_y;
				sum_x2 += val_x * val_x;
				sum_y2 += val_y * val_y;
				sum_xy += val_x * val_y;
			}
		}

		double area = (double)src1.rows * (double)src1.cols;
		mean_x = sum_x / area;
		mean_y = sum_y / area;
		var_x = sqrt(sum_x2 / area - mean_x * mean_x);
		var_y = sqrt(sum_y2 / area - mean_y * mean_y);
		covar_xy = sum_xy / area - mean_x * mean_y;

		ssim = ((2 * mean_x * mean_y + c1) * (2 * covar_xy + c2)) / ((mean_x * mean_x + mean_y * mean_y + c1) * (var_x * var_x + var_y * var_y + c2));
	}

	return ssim;
}

void cvex::MSSIM(cv::Mat src1, cv::Mat src2, int blocksize, double& score, cv::Mat& heat_map) {

	assert(src1.size() == src2.size());
	assert(src1.channels() == 1);
	assert(src2.channels() == 1);

	if (blocksize < 3) blocksize = 3;

	cv::Mat block = cv::Mat::zeros(cv::Size(src1.cols - blocksize + 1, src1.rows - blocksize + 1), CV_64FC1);

	score = 0.;
	int count = 0;
	for (int y = 0; y < src1.rows - (blocksize - 1); y++) {
		for (int x = 0; x < src1.cols - (blocksize - 1); x++) {
			double ssim = SSIM(src1(cv::Rect(x, y, blocksize, blocksize)), src2(cv::Rect(x, y, blocksize, blocksize)));
			block.at<double>(y, x) = (1 - ssim) * 255;
			score += ssim;
			count++;
		}
	}

	score /= double(count);
	cv::convertScaleAbs(block, heat_map, 1, 0);
}

cv::Mat cvex::CreateColorMask(cv::Mat gray_img, cv::Mat binary_img, ColorRange range) {
	cv::Mat result;
	cv::merge(vector<Mat>{
		range.blue ? gray_img * 0.7 + binary_img * 0.3 : gray_img * 0.7,
			range.green ? gray_img * 0.7 + binary_img * 0.3 : gray_img * 0.7,
			range.red ? gray_img * 0.7 + binary_img * 0.3 : gray_img * 0.7},
		result);

	return result;
}

cv::Mat cvex::CreateColorMask(cv::Mat gray_img, std::vector<cv::Mat> bin_imgs, std::vector<ColorRange> ranges) {
	cv::Mat result;
	cv::cvtColor(gray_img, result, cv::COLOR_GRAY2BGR);

	if (bin_imgs.size() != ranges.size()) {
		std::cout << "bin_imgs.size != ranges.size" << std::endl;
	} else {
		for (int i = 0; i < bin_imgs.size(); i++) {
			cv::Mat bin_img = bin_imgs[i].clone();
			ColorRange range = ranges[i];
			cv::Mat pallet = CreateColorMask(gray_img, bin_img, range);
			pallet.copyTo(result, bin_img);
		}
	}

	return result;
}

#pragma endregion


#pragma region FileSystem

void FileSystem::read_csv(string FileName, vector<float>& data) {

	string extension = FileName.substr(FileName.size() - 3, 3);
	if (!(extension == "csv") && !(extension == "CSV")) {
		cout << "File Extension is not csv" << endl;
	}

	std::string str_buf;
	std::string str_conma_buf;
	std::string input_csv_file_path = FileName;

	// 読み込むcsvファイルを開く(std::ifstreamのコンストラクタで開く)
	std::ifstream ifs_csv_file(input_csv_file_path);

	// getline関数で1行ずつ読み込む(読み込んだ内容はstr_bufに格納)
	while (getline(ifs_csv_file, str_buf)) {
		// 「,」区切りごとにデータを読み込むためにistringstream型にする
		std::istringstream i_stream(str_buf);

		// 「,」区切りごとにデータを読み込む
		while (getline(i_stream, str_conma_buf, ',')) {
			data.push_back(strtof(str_conma_buf.c_str(), nullptr));
		}
	}
}

void FileSystem::read_csv(string FileName, vector<vector<float>>& data) {

	string extension = FileName.substr(FileName.size() - 3, 3);
	if (!(extension == "csv") && !(extension == "CSV")) {
		cout << "File Extension is not csv" << endl;
	}

	std::string str_buf;
	std::string str_conma_buf;
	std::string input_csv_file_path = FileName;

	// 読み込むcsvファイルを開く(std::ifstreamのコンストラクタで開く)
	std::ifstream ifs_csv_file(input_csv_file_path);

	// getline関数で1行ずつ読み込む(読み込んだ内容はstr_bufに格納)
	while (getline(ifs_csv_file, str_buf)) {

		vector<float> col_data;

		// 「,」区切りごとにデータを読み込むためにistringstream型にする
		std::istringstream i_stream(str_buf);

		// 「,」区切りごとにデータを読み込む
		while (getline(i_stream, str_conma_buf, ',')) {
			col_data.push_back(strtof(str_conma_buf.c_str(), nullptr));
		}

		data.push_back(col_data);
	}
}

void FileSystem::write_csv(string file_name, vector<string> Labels, cv::Mat1d scores) {

	if (file_name.substr(file_name.size() - 4, 4) != ".csv") {
		file_name += ".csv";
	}

	std::ofstream save(file_name);

	double output;

	save << ",";

	for (int x = 0; x < Labels.size(); x++) {
		save << Labels[x] << ",";
	}

	save << std::endl;

	for (int y = 0; y < scores.rows; y++) {

		save << std::dec << y << ",";

		for (int x = 0; x < scores.cols; x++) {
			output = scores.at<double>(y, x);
			save << std::dec << output << ",";
		}
		save << std::endl;
	}

	save.close();

}

#pragma endregion