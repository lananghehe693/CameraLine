#pragma once
#include <iostream>
#include <fstream>
#include <opencv2/opencv.hpp>
//#include <opencv2/ximgproc.hpp>
#include <Windows.h>
#include <filesystem>
#include <thread>
#define _USE_MATH_DEFINES
#include <math.h>

#undef CreateWindow
#define CreateWindow(WindowNo) {\
	cv::namedWindow("TrackbarWindow" + std::to_string(WindowNo), cv::WINDOW_AUTOSIZE);\
	cv::resizeWindow("TrackbarWindow" + std::to_string(WindowNo), cv::Size(700, 1000));\
}

#define DEPLOYMENT(a) a
#define TRACK_SELECTOR( _1, _2, _3, _4, _5, _6, SELECT, ...) SELECT
#define Track1(Name, WindowNo, Variable, Value) cv::createTrackbar(Name, "TrackbarWindow" + to_string(WindowNo), &Variable, Value)
#define Track2(Name, WindowNo, Variable, Value, Func) cv::createTrackbar(Name, "TrackbarWindow" + to_string(WindowNo), &Variable, Value, Func)
#define Track3(Name, WindowNo, Variable, Value, Func, userdata) cv::createTrackbar(Name, "TrackbarWindow" + to_string(WindowNo), &Variable, Value, Func, &userdata)
#define Track(...) DEPLOYMENT(TRACK_SELECTOR(__VA_ARGS__, Track3, Track2, Track1)(__VA_ARGS__))

#define OddSize(Value) [](int, void*){\
	if(Value % 2 == 0) Value++;\
	}
#define Debug(func) if(debug_flag) {func}

using namespace std;
using namespace cv;

class Parameter
{
public:

	Parameter();

	cv::Mat InputImage;
	cv::VideoCapture capture;

	std::stringstream ss;

	int textcount = 0;
	cv::Mat TextImage = cv::Mat(cv::Size(700, 10000), CV_8UC1, 255);

	double ScreenSizeRatio = 0.8;
	double MaxSizeRatio = 30.0;
	double MinSizeRatio = 0.1;
	cv::Rect Screen = cv::Rect(cv::Point(0, 0), cv::Size(int(GetSystemMetrics(SM_CXSCREEN) * ScreenSizeRatio),int(GetSystemMetrics(SM_CYSCREEN) * ScreenSizeRatio)));

	/// <summary>
	/// テキストイメージ表示
	/// </summary>
	/// <param name="str">テキストイメージに書きたい文字</param>
	void PutText(std::string str);
	void PutText(std::stringstream& ss);

	/// <summary>
	/// マウスコールバック時に渡すパラメータ
	/// </summary>
	struct MouseCallBackParas {

		std::string BrightStr;
		double DeltaSizeRatio = 0.0;
		double ImgSizeRatio = 1.0;
		cv::Point MousePoint;
		cv::Point ScalePtr;
		std::vector<cv::Rect> DrawRects;
		cv::Rect MouseRect;

	} MousePara;

	/// <summary>
	/// トラックバーで使用するパラメータ
	/// </summary>
	struct TrackParas {

		//フィルタパラメータ
		int BlurX = 0;				//Blur用
		int BlurY = 0;				//Blur用
		int GaussianSize = 0;		//Gaussianblur用
		int SigmaX = 0;				//Gaussianblur用
		int SigmaY = 0;				//GaussianBlur用
		int SigmaRatio = 0;
		int MedianSize = 0;			//MedianFilter用
		int SobelSize = 0;			//Sobel用
		int d = 0;					//bilateralFilter用
		int Color = 0;				//bilateralFilter用
		int Space = 0;				//bilateralFilter用
		int Roop = 0;				//ループ処理用
		int AdaptiveSize = 0;		//adaptivethreshold用
		int AdaptiveC = 0;			//adaptivethreshold用
		int g_thresh = 0;			//threshold用
		int brightness = 0;			//輝度値調整用
		int BrightRatio = 0;		//輝度値調整用
		int gamma = 0;				//ガンマ値
		int gain = 0;				//ゲイン値
		int normalize = 0;			//正規化用
		int filter = 0;				//
		int Canny1 = 0;				//Canny用
		int Canny2 = 0;				//Canny用
		int HoughThresh = 0;		//Houg変換用
		int MaxLineGap = 0;			//Houg変換用
		int MinLineLength = 0;		//Houg変換用
		int MorphCount = 0;			//Morphology変換
		int Epsilon = 0;			//approxPolyDP用
		int SauvolaSize = 0;		//sauvolaFast用
		int SauvolaK = 0;			//sauvolaFast用
		int SauvolaR = 0;			//sauvolaFast用
		int LaplacianSize = 0;		//Laplacian用
		int StandardMean = 0;		//Standardization用
		int StandardDiv = 0;		//Standardization用
		int margin = 0;				//余白用
		int RangeMin = 0;			//RangeNormalize用
		int RangeMax = 0;			//RangeNormalize用

		//閾値
		int AreaLimit = 0;			//面積の閾値
		int RadiusLimit = 0;		//半径の閾値
		int LengthLimit = 0;		//長さの閾値
		int BrightnessLimit = 0;	//輝度値の閾値
		int MargeDistance = 0;		//結合する間隔
		int StdLimit = 0;			//偏差の閾値
		int DefectLimit = 0;		//凸包の閾値
		int EraseArea = 0;			//ノイズ除去用
		int EraseLength = 0;		//ノイズ除去用

		//座標パラメータ
		int angle = 0;				//画像回転角度
		int RectX = 0;				//roi抽出Rect
		int RectY = 0;				//roi抽出Rect
		int RectWidth = 0;			//roi抽出Rect
		int RectHeight = 0;			//roi抽出Rect
		int LineX = 0;				//X座標1
		int LineX2 = 0;				//X座標2
		int LineY = 0;				//Y座標1
		int LineY2 = 0;				//Y座標2
		int Radius = 0;				//円の半径
		int MojiPosX = 0;			//描画文字の位置調整用
		int MojiPosY = 0;			//描画文字の位置調整用

		//表示パラメータ
		int frame = 0;			//動画フレーム
		int frameend = 0;		//動画終了フレーム
		int Iscroll = 0;		//画像表示スクロール
		int IscrollX = 0;		//画像表示スクロールX
		int IscrollY = 0;		//画像表示スクロールY
		int Thickness = 1;		//描写太さ
		int Weight = 0;			//AddWeightedの重み
		int scroll = 0;			//TextImageスクロール
		int show = 0;			//画像表示No

	};
};

typedef Parameter::TrackParas TParas;
using PParameter = std::shared_ptr<Parameter>;

#define BilateralTrack {\
Track("d", 1, tp.d, 100);\
Track("Color", 1, tp.Color, 1000);\
Track("Space", 1, tp.Space, 100);\
}

#define AdaptiveTrack {\
Track("AdapSize", 1, tp.AdaptiveSize, 1000, [](int, void*){\
if(tp.AdaptiveSize % 2 == 0)\
tp.AdaptiveSize++;\
if(tp.AdaptiveSize < 3)\
tp.AdaptiveSize = 3;});\
Track("AdapC", 1, tp.AdaptiveC, 255);\
}

#define CannyTrack {\
Track("Canny1", 1, tp.Canny1, 1000);\
Track("Canny2", 1, tp.Canny2, 1000);\
}

#define SauvolaTrack {\
Track("SauvolaSize", 1, tp.SauvolaSize, 1000, OddSize(tp.SauvolaSize));\
Track("SauvolaK", 1, tp.SauvolaK, 1000);\
Track("SauvolaR", 1, tp.SauvolaR, 1000);\
}

#define GaussianTrack {\
Track("GaussSize", 1, tp.GaussianSize, 100, OddSize(tp.GaussianSize));\
Track("SigmaX", 1, tp.SigmaX, 100);\
Track("SigmaY", 1, tp.SigmaY, 100);\
}

#define RangeTrack {\
Track("RangeMax", 1, tp.RangeMax, 255);\
Track("RangeMin", 1, tp.RangeMin, 255);\
}

#define RoiTrack(image){\
int x = 0; std::swap(x, tp.RectX);\
int y = 0; std::swap(y, tp.RectY);\
Track("RectX", 1, tp.RectX, image.cols - tp.RectWidth, [](int pos, void* data) -> void {\
	cv::Mat* Image = static_cast<cv::Mat*>(data);\
	cv::setTrackbarMax("RectWidth", "TrackbarWindow1", Image->cols - pos);\
	}, image);\
Track("RectY", 1, tp.RectY, image.rows - tp.RectHeight, [](int pos, void* data) -> void {\
	cv::Mat* Image = static_cast<cv::Mat*>(data);\
	cv::setTrackbarMax("RectHeight", "TrackbarWindow1", Image->rows - pos);\
	}, image);\
Track("RectWidth", 1, tp.RectWidth, image.cols - tp.RectX, [](int pos, void* data) -> void {\
	cv::Mat* Image = static_cast<cv::Mat*>(data);\
	cv::setTrackbarMax("RectX", "TrackbarWindow1", Image->cols - pos);\
	}, image);\
Track("RectHeight", 1, tp.RectHeight, image.rows - tp.RectY, [](int pos, void* data) -> void {\
	cv::Mat* Image = static_cast<cv::Mat*>(data);\
	cv::setTrackbarMax("RectY", "TrackbarWindow1", Image->rows - pos);\
	}, image);\
std::swap(x, tp.RectX); cv::setTrackbarPos("RectX", "TrackbarWindow1", tp.RectX);\
std::swap(y, tp.RectY); cv::setTrackbarPos("RectY", "TrackbarWindow1", tp.RectY);\
}

#define StandardTrack {\
Track("Mean", 1, tp.StandardMean, 255);\
Track("Div", 1, tp.StandardDiv, 1000);\
}

class SampleInspection {
protected:

	static std::string FilePath;
	static std::string FileName;
	static std::string WriteFilePath;
	static int SampleNo;
	static int N;

	PParameter pPara;

	cv::VideoCapture Capture;
	cv::Mat InputImage;

public:
	SampleInspection();

	bool ReadFile();
	void SetFilePath(std::string Path, std::string Name);
	virtual void StartInspection();
};

using PSample = std::shared_ptr<SampleInspection>;

/// <summary>
/// 汎用関数
/// </summary>
namespace cvex {

	class Circle;

	class Line {
	public:

		enum SLOPE_TYPE {
			GRADIENT,
			DEGREE,
			RADIAN
		};

		/// <summary>
		/// コストラクタ
		/// </summary>
		Line();
		/// <summary>
		/// 2点で直線を定義
		/// </summary>
		/// <param name="p1">点1</param>
		/// <param name="p2">点2</param>
		Line(cv::Point2d p1, cv::Point2d p2);
		/// <summary>
		/// 傾きと点で直線を定義
		/// </summary>
		/// <param name="slope">傾き</param>
		/// <param name="p">点</param>
		/// <param name="slope_type">傾きの種類</param>
		/// <param name="">　GRADIENT：比率</param>
		/// <param name="">　DEGREE：角度</param>
		/// <param name="">　RADIAN：ラジアン</param>
		Line(double slope, cv::Point2d p, int slope_type = GRADIENT);
		/// <summary>
		/// 傾きと切片で直線を定義
		/// </summary>
		/// <param name="slope">傾き</param>
		/// <param name="intercept">切片</param>
		/// <param name="slope_type">傾きの種類</param>
		/// <param name="">　GRADIENT：比率</param>
		/// <param name="">　DEGREE：角度</param>
		/// <param name="">　RADIAN：ラジアン</param>
		Line(double slope, double intercept, int slope_type = GRADIENT);
		/// <summary>
		/// 点群から最小二乗法で直線を定義（ロバスト補正付）
		/// </summary>
		/// <param name="ctr">点群</param>
		/// <param name="roop">ループ回数</param>
		Line(vector<Point>& ctr, int roop = 1);

		/// <summary>
		/// y座標からx座標を取得
		/// </summary>
		/// <param name="">y座標</param>
		/// <returns>x座標</returns>
		double getX(double y);
		/// <summary>
		/// x座標からy座標を取得
		/// </summary>
		/// <param name="">x座標</param>
		/// <returns>y座標</returns>
		double getY(double x);
		/// <summary>
		/// x座標から点のアドレスを取得
		/// </summary>
		/// <param name="">x座標</param>
		/// <returns>x座標のアドレス</returns>
		cv::Point2d getXPoint(double x);
		/// <summary>
		/// y座標から点のアドレスを取得
		/// </summary>
		/// <param name="">y座標</param>
		/// <returns>y座標のアドレス</returns>
		cv::Point2d getYPoint(double y);
		/// <summary>
		/// x軸方向にオフセット
		/// </summary>
		/// <param name="x">：オフセット値</param>
		void offsetX(double x);
		/// <summary>
		/// x軸方向にオフセットして代入
		/// </summary>
		/// <param name="x">：オフセット値</param>
		/// <param name="l">：代入する直線</param>
		void offsetX(double x, cvex::Line& l);
		/// <summary>
		/// y軸方向にオフセット
		/// </summary>
		/// <param name="y">：オフセット値</param>
		void offsetY(double y);
		/// <summary>
		/// y軸方向にオフセットして代入
		/// </summary>
		/// <param name="y">：オフセット値</param>
		/// <param name="l">：代入する直線</param>
		void offsetY(double y, cvex::Line& l);
		/// <summary>
		/// x軸・y軸両方でオフセット
		/// </summary>
		/// <param name="p">：オフセット値</param>
		void offsetP(cv::Point2d p);
		/// <summary>
		/// x軸・y軸両方でオフセットして代入
		/// </summary>
		/// <param name="p">オフセット値</param>
		/// <param name="l">代入する直線</param>
		void offsetP(cv::Point2d p, cvex::Line& l);
		/// <summary>
		/// 点を中心に直線を回転
		/// </summary>
		/// <param name="degree">：回転する角度</param>
		/// <param name="center">：回転中心</param>
		void offsetA(double degree, cv::Point2d center);
		/// <summary>
		/// 点を中心に直線を回転して代入
		/// </summary>
		/// <param name="degree">回転する角度</param>
		/// <param name="center">回転中心</param>
		/// <param name="l">代入する直線</param>
		void offsetA(double degree, cv::Point2d center, cvex::Line& l);
		/// <summary>
		/// 点を通る法線を定義
		/// </summary>
		/// <param name="p">：点</param>
		/// <returns>法線</returns>
		Line getNormal(cv::Point2d p);
		/// <summary>
		/// 直線と直線の交点を計算
		/// </summary>
		/// <param name="line">：直線</param>
		/// <returns>交点</returns>
		cv::Point2d crossPoint(cvex::Line line);
		/// <summary>
		/// 直線と円との交点を計算
		/// </summary>
		/// <param name="circle">：円</param>
		/// <returns>交点</returns>
		vector<Point2d> crossPoint(Circle circle);
		/// <summary>
		/// 点と直線の距離を計算
		/// </summary>
		/// <param name="p">：点</param>
		/// <returns>点と直線の距離</returns>
		double calcLength(cv::Point2d p);
		/// <summary>
		/// 直線を描画
		/// </summary>
		/// <param name="image">：描画する画像</param>
		/// <param name="color">：線の色</param>
		/// <param name="thickness">：線の太さ</param>
		/// <param name="line_type">：線の種類</param>
		/// <param name="">　SOLID_LINE：実線</param>
		/// <param name="">　DASHED_LINE：破線</param>
		/// <param name="">　DOT_DASH_LINE：一点鎖線</param>
		/// <param name="">　TWO_DOT_DASH_LINE：二点鎖線</param>
		/// <param name="gap">：線の切れ目の間隔</param>
		/// <param name="">　0の場合、thickness＊２</param>
		/// <param name="linewidth">線の区間長さ</param>
		/// <param name="">　0の場合、thickness＊４</param>
		void draw(cv::Mat& image, cv::Scalar color, int thickness, int line_type = SOLID_LINE, int gap = 0, int linewidth = 0);
		/// <summary>
		/// 線分を描画
		/// </summary>
		/// <param name="image">：描画する画像</param>
		/// <param name="start">：線の開始点</param>
		/// <param name="end">：線の終点</param>
		/// <param name="color">：線の色</param>
		/// <param name="thickness">：線の太さ</param>
		/// <param name="line_type">：線の種類</param>
		/// <param name="">　SOLID_LINE：実線</param>
		/// <param name="">　DASHED_LINE：破線</param>
		/// <param name="">　DOT_DASH_LINE：一点鎖線</param>
		/// <param name="">　TWO_DOT_DASH_LINE：二点鎖線</param>
		/// <param name="gap">：線の切れ目の間隔</param>
		/// <param name="">　0の場合、thickness＊２</param>
		/// <param name="linewidth">線の区間長さ</param>
		/// <param name="">　0の場合、thickness＊４</param>
		void draw(cv::Mat& image, cv::Point start, cv::Point end, cv::Scalar color, int thickness, int line_type = SOLID_LINE, int gap = 0, int linewidth = 0);

		enum BINARY_TYPE {
			UPPER_WHITE,
			UPPER_BLACK,
			LOWER_WHITE,
			LOWER_BLACK,
			LEFT_WHITE,
			LEFT_BLACK,
			RIGHT_WHITE,
			RIGHT_BLACK
		};

		enum LINE_TYPE
		{
			SOLID_LINE,
			DASHED_LINE,
			DOT_DASH_LINE,
			TWO_DOT_DASH_LINE,
		};

		/// <summary>
		/// 線を境に白黒を指定
		/// </summary>
		/// <param name="src">：入力画像</param>
		/// <param name="binary_type">：白黒の方法</param>
		/// <returns>出力画像</returns>
		cv::Mat BinaryBorder(cv::Mat src, int binary_type);

		double slope = 0;
		double intercept = 0;

		Line& operator=(const Line& l) {
			this->slope = l.slope;
			this->intercept = l.intercept;
			return *this;
		};

	private:

		vector<Point> create_bresenham_points(cv::Point start, cv::Point end);

	};

	class Circle {
	public:

		Circle();
		Circle(cv::Point2d center, double radius);
		Circle(cv::Point2d p1, cv::Point2d p2);
		Circle(cv::Point2d p1, cv::Point2d p2, cv::Point2d p3);
		template<typename T> Circle(vector<T>& ctr, int roop = 1);

		/// <summary>
		/// 面積を算出
		/// </summary>
		/// <returns>面積</returns>
		double area();
		/// <summary>
		/// 円周長を算出
		/// </summary>
		/// <returns>円周長</returns>
		double arcLength();
		/// <summary>
		/// 極座標変換
		/// </summary>
		/// <param name="src">入力画像</param>
		/// <param name="flag">補間手法</param>
		/// <param name="overlap">オーバーラップ</param>
		/// <returns>極座標変換画像</returns>
		cv::Mat warpPolar(cv::Mat src, int flag = cv::INTER_CUBIC, int overlap = 0);
		/// <summary>
		/// 点の極座標を算出
		/// </summary>
		/// <param name="pos">入力座標</param>
		/// <param name="rotateCode">変換後の回転</param>
		/// <returns>点の極座標</returns>
		cv::Point2d warpPolar(cv::Point2d pos, int rotateCode = -1);
		/// <summary>
		/// 極座標逆変換
		/// </summary>
		/// <param name="src">入力画像</param>
		/// <param name="dst_size">出力画像のサイズ</param>
		/// <param name="overlap">入力画像のオーバーラップ長</param>
		/// <returns>極座標逆変換画像</returns>
		cv::Mat warpPolarInv(cv::Mat src, cv::Size dst_size, int overlap = 0);
		/// <summary>
		/// 点の極座標逆変換の算出
		/// </summary>
		/// <param name="pos">入力座標</param>
		/// <param name="rotateCode">現在の回転量</param>
		/// <returns>極座標逆変換後の座標</returns>
		cv::Point2d warpPolarInv(cv::Point2d pos, int rotateCode = -1);
		/// <summary>
		/// 円の描画
		/// </summary>
		/// <param name="image">描画する画像</param>
		/// <param name="color">描画する色</param>
		/// <param name="thickness">線の幅</param>
		/// <param name="use_buresenham">ブレゼンハムの手法を使うか</param>
		void draw(cv::Mat& image, cv::Scalar color, int thickness, bool use_buresenham = false);
		/// <summary>
		/// 扇円の描画
		/// </summary>
		/// <param name="image">描画する画像</param>
		/// <param name="color">描画する色</param>
		/// <param name="thickness">線の幅</param>
		/// <param name="start_angle">扇の開始角度</param>
		/// <param name="end_angle">扇の終点角度</param>
		/// <param name="min_arc">扇の短辺を描画するか</param>
		void draw(cv::Mat& image, cv::Scalar color, int thickness, double start_angle, double end_angle, bool min_arc = true);
		/// <summary>
		/// 対象の点と円との最短距離を算出
		/// </summary>
		/// <param name="pos">対象の点</param>
		/// <returns>最短距離</returns>
		double CalcPointToCircleLength(cv::Point2d pos);
		/// <summary>
		/// 対象の輪郭と理想円とのズレ平均とズレの標準偏差を算出
		/// </summary>
		/// <param name="ctr">対象の輪郭</param>
		/// <param name="mean">理想円のズレ平均</param>
		/// <param name="dev">ズレの標準偏差</param>
		void MeanStdDev(vector<Point> ctr, double& mean, double& dev);
		/// <summary>
		/// 対象の点が円に含まれているか
		/// </summary>
		/// <param name="p">対象の点</param>
		/// <returns>含まれているか</returns>
		bool contains(cv::Point2d p);
		/// <summary>
		/// 円を囲む矩形を出力
		/// </summary>
		/// <returns>円を囲む矩形</returns>
		cv::Rect boundingRect();
		/// <summary>
		/// 対象の点について、円の中心点を中心に指定角度回転させた時の円周上の点を算出
		/// </summary>
		/// <param name="pos">対象の点</param>
		/// <param name="degree">指定角度</param>
		/// <returns>回転後の円周上の点</returns>
		cv::Point2d CalcRotatePos(cv::Point2d pos, double degree);
		/// <summary>
		/// 対象の線と円の交点を算出
		/// </summary>
		/// <param name="line">対象の線</param>
		/// <returns>交点</returns>
		vector<Point2d> crossPoints(cvex::Line line);
		/// <summary>
		/// 対象の円と円との交点を算出
		/// </summary>
		/// <param name="circle">対象の円</param>
		/// <returns>交点</returns>
		vector<Point2d> crossPoints(Circle circle);

		/// <summary>
		/// x座標のオフセット
		/// </summary>
		/// <param name="x">オフセット値</param>
		void offsetX(double x);
		/// <summary>
		/// x座標のオフセット
		/// </summary>
		/// <param name="x">オフセット値</param>
		/// <param name="circle">代入する円</param>
		void offsetX(double x, cvex::Circle& circle);
		/// <summary>
		/// y座標のオフセット
		/// </summary>
		/// <param name="y">オフセット値</param>
		void offsetY(double y);
		/// <summary>
		/// y座標のオフセット
		/// </summary>
		/// <param name="y">オフセット値</param>
		/// <param name="circle">代入する円</param>
		void offsetY(double y, cvex::Circle& circle);
		/// <summary>
		/// x,y座標のオフセット
		/// </summary>
		/// <param name="p">オフセット値</param>
		void offsetP(cv::Point2d p);
		/// <summary>
		/// x,y座標のオフセット
		/// </summary>
		/// <param name="x">x座標オフセット値</param>
		/// <param name="y">y座標オフセット値</param>
		void offsetP(double x, double y);
		/// <summary>
		/// x,y座標のオフセット
		/// </summary>
		/// <param name="p">オフセット値</param>
		/// <param name="circle">代入する円</param>
		void offsetP(cv::Point2d p, cvex::Circle& circle);
		/// <summary>
		/// x,y座標のオフセット
		/// </summary>
		/// <param name="x">x座標オフセット値</param>
		/// <param name="y">y座標オフセット値</param>
		/// <param name="circle">代入する円</param>
		void offsetP(double x, double y, cvex::Circle& circle);

		cv::Point2d center = cv::Point2d(0, 0);
		double radius = 0;

	private:
		void calc_params_from(cv::Point2d p1, cv::Point2d p2, cv::Point2d p3);
	};

	std::ostream& operator << (ostream& os, const Circle& C);
	std::ostream& operator << (ostream& os, const Line& L);

	/// <summary>
	/// 中規模関数の継承用
	/// </summary>
	struct Func {
		PParameter pPara = PParameter(new Parameter());
	};
	/// <summary>
	/// 単一画像の表示
	/// </summary>
	/// <param name="Image">：表示する画像</param>
	void ShowImage(cv::Mat Image, string str = "");
	void ShowImage(vector<pair<Mat, string>> ProcessImages, string str = "");
	/// <summary>
	/// 処理画像配列の表示プログラム
	/// </summary>
	/// <param name="ProcessImages">：処理画像・画像名を格納した配列</param>
	/// <param name="tp">：トラックバーのパラメータ</param>
	/// <param name="pPara">：パラメータクラス</param>
	/// <returns> tuple(Mat Image, double SizeRatio) </returns>
	std::tuple<cv::Mat, double, cv::Point> ShowProcess(std::vector<std::pair<cv::Mat, std::string>>& ProcessImages, Parameter::TrackParas& tp, PParameter pPara);
	/// <summary>
	/// マウスコールバック関数
	/// </summary>
	void MouseCallBack(int event, int x, int y, int flag, void* data);
	/// <summary>
	/// テキストイメージのマウスコールバック関数
	/// </summary>
	void TextImageMouseCallBack(int event, int x, int y, int flag, void* data);
	/// <summary>
	/// 矩形近くに文字を表示させる場合の表示位置計算
	/// </summary>
	/// <param name="image">：描画する画像</param>
	/// <param name="str">：文字</param>
	/// <param name="font_scale">：文字の大きさ</param>
	/// <param name="thickness">：文字の太さ</param>
	/// <param name="rect">：描画する矩形</param>
	/// <returns>文字を描画する位置</returns>
	cv::Point CalcRectTextPos(cv::Mat image, string str, double font_scale, int thickness, cv::Rect rect);
	/// <summary>
	/// 連続結果表示
	/// </summary>
	/// <param name="WindowName">：ウィンドウ名</param>
	/// <param name="ProcessImages">：(画像・名前)配列</param>
	void ImagesShow(std::string WindowName, std::vector<std::pair<cv::Mat, std::string>> ProcessImages);
	/// <summary>
	/// 動画保存
	/// </summary>
	/// <param name="images">：画像配列</param>
	/// <param name="video_name">：動画名</param>
	void VideoWrite(string video_name, vector<Mat>& images, bool isColor = true);
	void VideoWrite(string video_name, vector<Mat>& images, double fps, bool isColor = true);
	/// <summary>
	/// 最小二乗法による近似円計算
	/// </summary>
	/// <param name="Contour">：円近似する点群</param>
	/// <param name="CirclePara">：(近似円の中心点、近似円の半径)</param>
	template<typename T>
	void CalcCircleParameter(std::vector<T> Contour, std::pair<cv::Point2d, double>& CirclePara, vector<double> weight);
	template<typename T>
	void CalcCircleParameter(std::vector<T> Contour, cv::Point2d& Center, double& Radius, int converge_times = 1);
	template<typename T>
	void CalcCircleParameter(std::vector<T> Contour, cvex::Circle& Circle, int converge_times = 1);
	void CalcCircleParameter(std::vector<cv::Point> Contour, std::pair<cv::Point2d, double>& circle);
	/// <summary>
	/// 最小二乗法による楕円近似計算
	/// </summary>
	/// <param name="Contour">：楕円近似する点群</param>
	/// <param name="Center">：近似楕円の中心点</param>
	/// <param name="LengthX">：近似楕円の中心点からX軸方向の長さ</param>
	/// <param name="LengthY">：近似楕円の中心点からY軸方向の長さ</param>
	/// <param name="Degree">：近似楕円の傾き（度）</param>
	bool CalcEllipseParameter(std::vector<cv::Point> Contour, cv::Point2d& Center, double& LengthX, double& LengthY, double& Degree);
	bool CalcEllipseParameter(std::vector<cv::Point> Contour, cv::RotatedRect& Rect);
	/// <summary>
	/// 最小二乗法による近似直線計算
	/// </summary>
	/// <param name="Contour">：直線近似する点群</param>
	/// <param name="LinePara">：(近似直線の傾き、近似直線の切片)</param>
	void CalcLineParameter(std::vector<cv::Point> Contour, std::pair<double, double>& LinePara);
	/// <summary>
	/// 最小距離二乗法による近似直線計算
	/// </summary>
	/// <param name="Contour">：直線近似する点群</param>
	/// <param name="LinePara">：（近似直線の傾き、近似直線の切片）[2]</param>
	template<typename T>
	void CalcMinLengthLineParameter(std::vector<T> Contour, std::pair<double, double>& LinePara, vector<double> weight = {});
	template<typename T>
	void CalcMinLengthLineParameter(std::vector<T> Contour, double& slope, double& intercept, int converge_times = 1);
	template<typename T>
	void CalcMinLengthLineParameter(std::vector<T> Contour, cvex::Line& line, int converge_times = 1);
	/// <summary>
	/// 2点による直線L と 点P との距離を求める
	/// </summary>
	/// <param name="Pos">：点P</param>
	/// <param name="LinePara">：直線上の2点</param>
	/// <returns>点と直線との距離H</returns>
	double CalcPointToLineLength(cv::Point2d Pos, std::pair<cv::Point2d, cv::Point2d> LinePara);
	/// <summary>
	/// y = ax + b の直線L と 点P との距離を求める
	/// </summary>
	/// <param name="Pos">：点P</param>
	/// <param name="LinePara">：{a, b}</param>
	/// <returns>点と直線との距離H</returns>
	double CalcPointToLineLength(cv::Point2d Pos, std::pair<double, double> LinePara);
	double CalcPointToLineLength(cv::Point2d Pos, cvex::Line Line);
	/// <summary>
	/// 2点(P1, P2)による線分L と 点P0 との最小距離を求める
	/// </summary>
	/// <param name="Pos">：点P0</param>
	/// <param name="LinePara">：線分(P1, P2)</param>
	/// <returns>点と直線との距離H</returns>
	double CalcPointToLineSegmentLength(cv::Point2d Pos, std::pair<cv::Point2d, cv::Point2d> LinePara);
	enum MARGE_SIDE {
		MARGE_SIDE_WIDTH,
		MARGE_SIDE_HEIGHT,
		MARGE_SIDE_ALL
	};
	/// <summary>
	/// 2個の矩形の最短距離を求める
	/// </summary>
	/// <param name="rect1">：矩形1個目</param>
	/// <param name="rect2">：矩形2個目</param>
	/// <returns>最短距離</returns>
	double CalcMinDisFromRects(cv::Rect rect1, cv::Rect rect2, int marge_side = MARGE_SIDE_ALL);
	/// <summary>
	/// 2個の回転矩形の最短距離を求める
	/// </summary>
	/// <param name="rect1">：回転矩形1個目</param>
	/// <param name="rect2">：回転矩形2個目</param>
	/// <returns>最短距離</returns>
	double CalcMinDisFromRects(cv::RotatedRect rect1, cv::RotatedRect rect2, int marge_side = MARGE_SIDE_ALL);
	/// <summary>
	/// 4点で囲まれた矩形の最短距離を求める
	/// </summary>
	/// <param name="points1">：矩形1個目の4点</param>
	/// <param name="points2">：矩形2個目の4点</param>
	/// <returns>最短距離</returns>
	double CalcMinDisFromRects(std::vector<cv::Point2d> points1, std::vector<cv::Point2d> points2, int marge_side = MARGE_SIDE_ALL);
	/// <summary>
	/// 2個の矩形を結合する
	/// </summary>
	/// <param name="rect1">：1個目の矩形</param>
	/// <param name="rect2">：2個目の矩形</param>
	/// <returns>結合矩形</returns>
	cv::Rect CalcBoundingBoxFromTwoRects(cv::Rect rect1, cv::Rect rect2);
	/// <summary>
	/// 2個の回転矩形を結合する
	/// </summary>
	/// <param name="rect1">：1個目の回転矩形</param>
	/// <param name="rect2">：2個目の回転矩形</param>
	/// <returns>結合回転矩形</returns>
	cv::RotatedRect CalcBoundingBoxFromTwoRects(cv::RotatedRect rect1, cv::RotatedRect rect2);
	/// <summary>
	/// 矩形配列の中でlength_thresholdより近い距離の矩形同士を結合する
	/// </summary>
	/// <param name="rects">：矩形配列</param>
	/// <param name="length_threshold">：結合する矩形の最大距離</param>
	/// <returns>結合された矩形配列</returns>
	std::vector<cv::Rect> MargeBoundingBoxes(std::vector<cv::Rect> rects, double length_threshold, int marge_side = MARGE_SIDE_ALL);
	std::vector<cv::RotatedRect> MargeBoundingBoxes(std::vector<cv::RotatedRect> rects, double length_threshold, int marge_side = MARGE_SIDE_ALL);
	std::vector<cv::RotatedRect> MargeNearContours(std::vector<std::vector<cv::Point>>& contours, double length_threshold, int marge_side = MARGE_SIDE_ALL, bool re_create_minAreaRect = false);
	/// <summary>
	/// 指定フォルダ内にある画像名を全て取得する
	/// </summary>
	/// <param name="dir_name">：フォルダパス＋検索名</param>
	/// <returns>ファイル名の配列</returns>
	//std::vector<std::string> getImageName(std::string dir_name);
	/// <summary>
	/// サブフォルダ含めた指定フォルダ内にある指定拡張子のファイル名一覧を取得
	/// </summary>
	/// <param name="dir_name">：フォルダ名</param>
	/// <param name="extension">：拡張子</param>
	/// <returns>ファイル名一覧</returns>
	//std::vector<std::string> getFileNamesAll(std::string dir_name, std::vector<std::string> extension);
	/// <summary>
	/// 指定フォルダ直下にあるフォルダ名を全て取得
	/// </summary>
	/// <param name="dir_name">：指定フォルダ名</param>
	/// <returns>指定フォルダ直下のフォルダリスト</returns>
	//std::vector<std::string> getDirNameList(std::string dir_name);
	/// <summary>
	/// 指定フォルダに処理画像を全て保存する。
	/// </summary>
	/// <param name="ProcessImages">：処理画像群</param>
	/// <param name="FilePath">：保存フォルダ</param>
	//void WriteImages(std::vector<std::pair<cv::Mat, std::string>> ProcessImages, std::string FilePath);
	enum STRIPE_MODE {
		STRIPE_AVERAGE, //平均値
		STRIPE_MAX,		//最大値
		STRIPE_MIN,		//最小値
		STRIPE_MEDIAN,	//中央値
	};
	/// <summary>
	/// 画像のストライプ化
	/// </summary>
	/// <param name="Src">：入力画像</param>
	/// <param name="Dst">：出力画像</param>
	/// <param name="horizon">：横方向</param>
	/// <param name="vertical">：縦方向</param>
	/// <param name="StripeMode">：平均値、最大値、最小値、中央値</param>
	/// <param name="stripe_range"></param>
	void ToStripe(cv::Mat Src, cv::Mat& Dst, bool horizon = true, bool vertical = false, int StripeMode = STRIPE_AVERAGE, int stripe_range = 0);
	/// <summary>
	/// 行列ごとの輝度値調整
	/// </summary>
	/// <param name="Src">：入力画像</param>
	/// <param name="Dst">：出力画像</param>
	/// <param name="horizon">：横方向</param>
	/// <param name="vertical">：縦方向</param>
	void StripeHist(cv::Mat Src, cv::Mat& Dst, bool horizon = true, bool vertical = false, int brightness = 128, cv::InputArray mask = cv::noArray());
	/// <summary>
	/// ±Marginごとの細かい区間でStripeHistを行う
	/// </summary>
	/// <param name="Src">：入力画像</param>
	/// <param name="Dst">：出力画像</param>
	/// <param name="Margin">：細かい区間の幅</param>
	/// <param name="horizon">：横方向</param>
	/// <param name="vertical">：縦方向</param>
	/// <param name="borderType">：両端の拡張タイプ</param>
	/// <param name="brightness">：設定する平均輝度値</param>
	void StripeHistP(cv::Mat Src, cv::Mat& Dst, int Margin, bool horizon = true, bool vertical = false, int borderType = cv::BORDER_REFLECT_101, int brightness = 128);
	void CorrectMean(cv::Mat Src, cv::Mat& Dst, int MeanVal = 128, cv::InputArray mask = cv::noArray());
	void CorrectMeanP(cv::Mat Src, cv::Mat& Dst, int Margin, bool TragetIsWidth = true, int brightness = 128, int border_type = cv::BORDER_REFLECT_101, cv::InputArray mask = cv::noArray());
	/// <summary>
	/// 行列ごとの輝度値グラフ作成
	/// </summary>
	/// <param name="Src">：入力画像</param>
	/// <param name="Dst">：グラフ画像</param>
	/// <param name="horizon">：横方向</param>
	/// <param name="vertical">：縦方向</param>
	void CreateBrightnessGraph(cv::Mat Src, cv::Mat& Dst, bool horizon = true, bool vertical = false);
	/// <summary>
	/// 多重バイラテラル
	/// </summary>
	/// <param name="Src">：入力画像</param>
	/// <param name="Dst">：出力画像</param>
	/// <param name="d">：bilateral d</param>
	/// <param name="Color">：bilateral Color</param>
	/// <param name="Space">：bilateral Space</param>
	/// <param name="Roop">ループ数</param>
	void BilateralRoop(cv::Mat Src, cv::Mat& Dst, int d, double Color, double Space, int Roop);
	/// <summary>
	/// DFT変換。パワースペクトル画像の表示には正規化が必要
	/// </summary>
	/// <param name="Src">：入力画像</param>
	/// <param name="comp_img">：DFT変換後出力画像</param>
	/// <param name="pow_spc">：パワースペクトル画像</param>
	void DFTConvert(cv::Mat Src, cv::Mat& comp_img, cv::Mat& pow_spc);
	/// <summary>
	/// DFT逆変換
	/// </summary>
	/// <param name="Src">：DFT変換画像</param>
	/// <param name="Dst">：IDFT変換画像</param>
	/// <param name="SpectrumMask">：スペクラムのマスク画像</param>
	void IDFTConvert(cv::Mat Src, cv::Mat& Dst, cv::Mat SpectrumMask);
	/// <summary>
	/// DFT変換検討用
	/// </summary>
	/// <param name="Src">：元画像</param>
	/// <param name="DebugFlag">：デバッグフラグ</param>
	/// <param name="FilePath">：保存するフォルダ</param>
	/// <returns>DFT変換後画像</returns>
	cv::Mat ReviewDFT(cv::Mat Src, bool DebugFlag = true, std::string FilePath = "");
	/// <summary>
	/// 一次元CSVデータの周波数解析用
	/// </summary>
	/// <param name="FileName">：周波数解析するCSVファイル</param>
	void CalcFrequency(std::string CSVFileName);
	/// <summary>
	/// ExtractContoursのモード選択用
	/// </summary>
	enum ExtractMode {
		Extract_Area,		//面積
		Extract_R,			//円の半径
		Extract_arcLength,  //輪郭長さ
		Extract_RRectHeight,	//回転矩形の長辺
		Extract_CenterX,	//中心のx座標
		Extract_CenterY		//中心のy座標
	};
	/// <summary>
	/// 輪郭データから、各モードの最大or最小の輪郭のみ抽出
	/// </summary>
	std::vector<std::vector<cv::Point>> ExtractContours(std::vector<std::vector<cv::Point>> Contours, int extractmode = 0, bool MaxVal = true);
	/// <summary>
	/// ヒストグラム作成
	/// </summary>
	cv::Mat CreateHistGram(cv::Mat Src);
	/// <summary>
	/// 積分画像による適用的二値化
	/// </summary>
	void sauvolaFast(const cv::Mat& src, cv::Mat& dst, int kernelSize = 3, double k = 0.5, double r = 50);
	/// <summary>
	/// ガンマ値補正
	/// </summary>
	/// <param name="Src">：入力画像</param>
	/// <param name="Dst">：出力画像</param>
	/// <param name="Gamma">：ガンマ値</param>
	void GammaCorrect(cv::Mat Src, cv::Mat& Dst, double Gamma);
	/// <summary>
	/// Different of GaussianFilter
	/// ガウシアン同士の差分画像
	/// </summary>
	/// <param name="Src">：入力画像</param>
	/// <param name="Dst">：出力画像</param>
	/// <param name="GaussSize">：ガウシアンサイズ</param>
	/// <param name="SigmaX">：シグマX</param>
	/// <param name="SigmaY">：シグマY</param>
	/// <param name="SigmaRatio">：差分画像のシグマ倍率</param>
	void DoGFilter(cv::Mat Src, cv::Mat& Dst, cv::Size GaussSize, double SigmaX, double SigmaY, double SigmaRatio);
	void DoGFilter(cv::Mat Src, cv::Mat& Dst, int GaussSize, double Sigma);
	/// <summary>
	/// 輪郭の重心を求める
	/// </summary>
	/// <param name="Contour">：輪郭</param>
	/// <returns>重心</returns>
	cv::Point2f CalcMoments(std::vector<cv::Point>& Contour);
	/// <summary>
	/// 輪郭の円形度を求める
	/// </summary>
	/// <param name="Contour"></param>
	/// <returns></returns>
	double CalcCircleLevel(std::vector<cv::Point>& Contour);

	void DrawDimensionLine(cv::Mat& Src, cv::Point pos1, cv::Point pos2, int Horizontal, int Vertical, int index, double correction);
	/// <summary>
	/// 符号判断
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="Val"></param>
	/// <returns>-1, 0, 1</returns>
	template<typename T> T Sign(T Val) {
		return (T(0) < Val) - (T(0) > Val);
	}
	/// <summary>
	/// 平均・標準偏差を利用した画像正規化
	/// </summary>
	/// <param name="Src">：入力画像</param>
	/// <param name="Dst">：出力画像</param>
	/// <param name="designed_mean">：平均値</param>
	/// <param name="designated_std_dev">：標準偏差</param>
	void Standardization(cv::Mat Src, cv::Mat& Dst, double designed_mean, double designated_std_dev, cv::InputArray mask = cv::noArray());
	/// <summary>
	/// 指定輝度範囲を正規化する
	/// </summary>
	/// <param name="Src">：入力画像</param>
	/// <param name="Dst">：出力画像</param>
	/// <param name="min">：指定輝度下限値</param>
	/// <param name="max">：指定輝度上限値</param>
	void RangeNormalization(cv::Mat Src, cv::Mat& Dst, int min, int max);
	/// <summary>
	/// 矩形を画像に適応させる
	/// </summary>
	/// <param name="rect">：矩形</param>
	/// <param name="Src">：画像</param>
	/// <param name="IsConstSize">：サイズ固定か</param>
	/// <param name="IsShiftable">：移動可能か</param>
	/// <returns></returns>
	cv::Rect MakeSuitableRect(cv::Rect rect, cv::Mat Src, bool IsConstSize = false, bool IsShiftable = false);
	void MakeSuitableRect(vector<Rect>& rects, cv::Mat Src, bool IsConstSize = false, bool IsShiftable = false);
	/// <summary>
	/// 矩形 ➡　正方形に変換
	/// </summary>
	/// <param name="rect">：矩形</param>
	/// <param name="Src">：画像</param>
	/// <returns>正方形</returns>
	cv::Rect ConvertRect(cv::Rect rect, cv::Mat Src);
	/// <summary>
	/// 矩形群　➡　最大辺の正方形群に変換
	/// </summary>
	/// <param name="error_rects">：矩形群</param>
	/// <param name="Src">：画像</param>
	/// <returns>正方形群</returns>
	std::vector<cv::Rect> ConvertRects(std::vector<cv::Rect> error_rects, cv::Mat Src);
	/// <summary>
	/// ガウシアンフィルタの簡略表記
	/// </summary>
	/// <param name="src">：入力画像</param>
	/// <param name="block_size">：ガウシアンブロックサイズ</param>
	/// <param name="sigma">：シグマ</param>
	/// <returns>出力画像</returns>
	cv::Mat GaussianFilter(cv::Mat src, int block_size, double sigma);
	/// <summary>
	/// cv::rectangleの拡張
	/// </summary>
	/// <param name="image">：入出力画像</param>
	/// <param name="rect">：矩形</param>
	/// <param name="color">：色</param>
	/// <param name="thickness">：太さ</param>
	/// <param name="line_type">：線種（実線・破線・一点鎖線・二点鎖線）</param>
	/// <param name="gap">：線の間隔</param>
	/// <param name="linewidth">：線の長さ</param>
	void DrawRect(cv::Mat& image, cv::Rect rect, cv::Scalar color, int thickness = 1, int line_type = cvex::Line::SOLID_LINE, int gap = 0, int linewidth = 0);
	/// <summary>
	/// 回転矩形の描画
	/// </summary>
	/// <param name="src">：入出力画像</param>
	/// <param name="rect">：回転矩形</param>
	/// <param name="color">：色</param>
	/// <param name="thickness">太さ</param>
	void DrawRotatedRect(cv::Mat& src, cv::RotatedRect rect, cv::Scalar color, int thickness = 1, int line_type = cvex::Line::SOLID_LINE, int gap = 0, int linewidth = 0);
	/// <summary>
	/// ある点を中心点を軸に指定角度分回転させる
	/// </summary>
	/// <param name="pos">：対象点</param>
	/// <param name="center">：回転軸の中心点</param>
	/// <param name="degree">：回転角度</param>
	/// <returns></returns>
	cv::Point2d CalcRotatePos(cv::Point2d pos, cv::Point2d center, double degree);
	/// <summary>
	/// 2次元微分フィルタ（絶対値）
	/// </summary>
	/// <param name="src">：入力画像</param>
	/// <param name="dst">：出力画像</param>
	/// <param name="size">：ソーベルサイズ</param>
	void SobelFilter(cv::Mat src, cv::Mat& dst, int size = 3);
	/// <summary>
	/// ブレゼンハムの円描画
	/// </summary>
	/// <param name="src"></param>
	/// <param name="center"></param>
	/// <param name="radius"></param>
	void DrawBresenhamCircle(cv::Mat& src, cv::Point2d center, double radius, cv::Scalar color, int thickness = 1);
	/// <summary>
	/// 年月日時間を文字列で出力
	/// </summary>
	/// <returns></returns>
	string getDateTimeStr();
	/// <summary>
	/// SSIM算出による画像類似比較
	/// </summary>
	/// <param name="src1">：画像1</param>
	/// <param name="src2">：画像2</param>
	/// <returns>SSIM</returns>
	double SSIM(cv::Mat src1, cv::Mat src2);
	/// <summary>
	/// 局所的にSSIMを算出し、平均値(類似度）を出力
	/// </summary>
	/// <param name="src1">：画像1</param>
	/// <param name="src2">：画像2</param>
	/// <param name="blocksize">：局所ブロック長さ</param>
	/// <param name="score">：類似度</param>
	/// <param name="heat_map">：相違箇所のヒートマップ</param>
	void MSSIM(cv::Mat src1, cv::Mat src2, int blocksize, double& score, cv::Mat& heat_map);
	struct ColorRange
	{
		bool blue;
		bool green;
		bool red;
	};
	/// <summary>
	/// 二値化画像をグレースケール画像にカラーで合成する
	/// </summary>
	/// <param name="gray_img">：グレースケール画像</param>
	/// <param name="bin_img">：二値化画像</param>
	/// <param name="isBlur">：青</param>
	/// <param name="isGreen">：緑</param>
	/// <param name="isRed">：赤</param>
	/// <returns>合成画像</returns>
	cv::Mat CreateColorMask(cv::Mat gray_img, cv::Mat bin_img, ColorRange range);
	cv::Mat CreateColorMask(cv::Mat gray_img, std::vector<cv::Mat> bin_imgs, vector<ColorRange> ranges);
};

typedef cvex::Circle cvexCircle;
typedef cvex::Line cvexLine;

/// <summary>
/// ファイル保存・読み込み
/// </summary>
namespace FileSystem {

	template <size_t N = 0, typename T>
	unsigned long long iterate_tuple_binarySize(unsigned long long& Size, const T& t) {
		if constexpr (N < std::tuple_size<T>::value) {
			const auto& x = std::get<N>(t);

			Size += sizeof(x);

			stringstream ss;
			ss << typeid(x).name();
			if (ss.str() != "int" && ss.str() != "float") {
				cout << "Not int or float" << endl;
				exit(0);
			}

			return iterate_tuple_binarySize<N + 1>(Size, t);
		}

		return Size;
	}

	template <typename X>
	bool write_binary(string filepath, vector<X>& vec) {
		if (vec.size() == 0) {
			cout << "vec.size() == 0" << endl;
			return false;
		}

		stringstream ss;
		ss << typeid(vec[0]).name();
		if (ss.str() != "int" && ss.str() != "float") {
			cout << "Not int or float" << endl;
			exit(0);
		}

		ofstream fout(filepath, ios::out | ios::binary);
		fout.write((char*)&vec[0], vec.size() * sizeof(vec[0]));
		fout.close();
		return true;
	};

	template<typename X, typename Y>
	bool write_binary(string filepath, vector<pair<X, Y>>& vec) {
		if (vec.size() == 0) {
			cout << "vec.size() == 0" << endl;
			return false;
		}

		stringstream ss;
		ss << typeid(get<0>(vec[0])).name();
		if (ss.str() != "int" && ss.str() != "float") {
			cout << "Not int or float" << endl;
			exit(0);
		}

		ss.clear();
		ss << typeid(get<1>(vec[0])).name();
		if (ss.str() != "int" && ss.str() != "float") {
			cout << "Not int or float" << endl;
			exit(0);
		}

		ofstream fout(filepath, ios::out | ios::binary);
		fout.write((char*)&vec[0], vec.size() * (sizeof(vec[0].first) + sizeof(vec[0].second)));
		fout.close();
		return true;
	};

	template<typename ...Args>
	bool write_binary(string filepath, vector<tuple<Args...>>& vec) {
		if (vec.size() == 0) {
			cout << "vec.size() == 0" << endl;
			return false;
		}

		unsigned long long Size = 0;
		Size = iterate_tuple_binarySize(Size, vec[0]);

		ofstream fout(filepath, ios::out | ios::binary);
		fout.write((char*)&vec[0], vec.size() * (Size));
		fout.close();
		return true;
	}

	template <typename X>
	bool read_binary(string filepath, vector<X>& vec) {
		if (vec.size() == 0) {
			cout << "vec.size() == 0" << endl;
			return false;
		}

		stringstream ss;
		ss << typeid(vec[0]).name();
		if (ss.str() != "int" && ss.str() != "float") {
			cout << "Not int or float" << endl;
			exit(0);
		}

		std::ifstream fin(filepath, ios::in | ios::binary);
		fin.read((char*)&vec[0], vec.size() * sizeof(vec[0]));
		fin.close();
		return true;
	};

	template<typename X, typename Y>
	bool read_binary(string filepath, vector<pair<X, Y>>& vec) {
		if (vec.size() == 0) {
			cout << "vec.size() == 0" << endl;
			return false;
		}

		stringstream ss;
		ss << typeid(get<0>(vec[0])).name();
		if (ss.str() != "int" && ss.str() != "float") {
			cout << "Not int or float" << endl;
			exit(0);
		}

		ss.clear();
		ss << typeid(get<1>(vec[0])).name();
		if (ss.str() != "int" && ss.str() != "float") {
			cout << "Not int or float" << endl;
			exit(0);
		}

		std::ifstream fin(filepath, ios::in | ios::binary);
		fin.read((char*)&vec[0], vec.size() * (sizeof(vec[0].first) + sizeof(vec[0].second)));
		fin.close();
		return true;
	};

	template<typename ...Args>
	bool read_binary(string filepath, vector<tuple<Args...>>& vec) {
		if (vec.size() == 0) {
			cout << "vec.size() == 0" << endl;
			return false;
		}

		unsigned long long Size = 0;
		Size = iterate_tuple_binarySize(Size, vec[0]);

		ifstream fin(filepath, ios::in | ios::binary);
		fin.read((char*)&vec[0], vec.size() * Size);
		fin.close();

		return true;
	}

	template <size_t N = 0, typename T>
	void iterate_tuple(FILE* fp, const T& t) {
		if constexpr (N < std::tuple_size<T>::value) {
			const auto& x = std::get<N>(t);

			stringstream ss;
			ss << typeid(x).name();

			//cout << x << ", " << ss.str() << endl;

			//if (ss.str() == "int") {
			//	fprintf(fp, "%d,", x);
			//}
			//else if (ss.str() == "float") {
			//	fprintf(fp, "%f,", x);
			//}
			//else if (ss.str() == "double") {
			//	fprintf(fp, "%lf,", x);
			//}
			//else {
			//	fprintf(fp, "%s,", x);
			//}

			if constexpr (std::is_same_v<decltype(x), int>) {
				fprintf(fp, "%d,", x);
			}
			else if constexpr (std::is_same_v<decltype(x), float>) {
				fprintf(fp, "%f,", x);
			}
			else if constexpr (std::is_same_v<decltype(x), double>) {
				fprintf(fp, "%f,", x);   // fprintf tidak pakai %lf!
			}
			else if constexpr (std::is_convertible_v<decltype(x), const char*>) {
				fprintf(fp, "%s,", x);
			}
			else {
				// fallback pakai std::string
				std::string tmp = std::to_string(x);
				fprintf(fp, "%s,", tmp.c_str());
			}

			iterate_tuple<N + 1>(fp, t);
		}
	}

	template<typename... Args>
	bool write_csv(string FilePath, vector<string> Labels, const vector<tuple<Args...>>& args) {

		if (FilePath.substr(FilePath.size() - 3, 3) != "csv") {
			FilePath += ".csv";
		}

		int LabelsSize = int(Labels.size());
		int DataVecSize = int(args.size());
		if (LabelsSize == 0) {
			cout << "Labels.size() == 0" << endl;
			return false;
		}
		else if (DataVecSize == 0) {
			cout << "DataVecSize == 0" << endl;
			return false;
		}

		//FILE* fp;
		//fopen_s(&fp, (FilePath).c_str(), "w");
		//for (auto str : Labels) {
		//	fprintf(fp, "%s,", str);
		//}
		//fprintf(fp, "\n");

		//for (auto Tuple : args) {
		//	iterate_tuple(fp, Tuple);
		//	fprintf(fp, "\n");
		//}

		//fclose(fp);

		FILE* fp = nullptr;
		errno_t err = fopen_s(&fp, FilePath.c_str(), "w");
		if (err != 0 || fp == nullptr) {
			// gagal membuka file
			return false;
		}

		// tulis header (labels)
		for (const auto& str : Labels) {
			fprintf(fp, "%s,", str.c_str());
		}
		fprintf(fp, "\n");

		// tulis isi tuple per baris
		for (const auto& tup : args) {
			iterate_tuple(fp, tup);
			fprintf(fp, "\n");
		}

		fclose(fp);

		return true;
	}

	void read_csv(std::string FilePath, std::vector<float>& data);
	void read_csv(std::string FilePath, std::vector<std::vector<float>>& data);
	void write_csv(std::string FilePath, std::vector<std::string> Labels, cv::Mat1d scores);
}