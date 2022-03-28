#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <stdio.h>
#include <math.h>
using namespace cv;
using namespace std;

const int ROI_RAD = 6; // radius of ROI
const Scalar color[4] = {Scalar(0,255,0), Scalar(255, 0, 0), Scalar(0,0,255), Scalar(255,255,0)};
int histSize = 8; // bucket size
float valueRange[] = { 0,256 };
const float* ranges[] = { valueRange };
int channels = 0;
int dims = 1; 
Mat img[2], table;
Mat roi[2][4]; // selected matrices
Mat hist_v[2][4];
int similar[4];
int idx = 0;
Point point[2][4]; // selected point

Mat find_draw(); // match points
Mat getHist(Mat roi, int i, int j); // get histogram
Mat calc(Mat roi); // count values in roi

void onMouse(int event, int x, int y, int flags, void* param); // event function

int main(int ac, char** av) {
	// load & resize images
	img[0] = imread("1st.jpg", IMREAD_GRAYSCALE);
	img[1] = imread("2nd.jpg", IMREAD_GRAYSCALE);
	if (img[0].empty() || img[1].empty()) return -1;
	resize(img[0], img[0], Size(img[0].cols * 0.25, img[0].rows * 0.25));
	resize(img[1], img[1], Size(img[1].cols * 0.25, img[1].rows * 0.25));

	// concatenate images & set event function
	hconcat(img[0], img[1], table);
	imshow("first & second", table);
	setMouseCallback("first & second", onMouse, 0);

	while (1) {
		if (waitKey(10) == 32) {
			// calculate
			for (int i = 0; i < 2; i++) {
				for (int j = 0; j < 4; j++) {
					roi[i][j] = table(Rect(point[i][j].x - ROI_RAD,
						point[i][j].y - ROI_RAD,
						ROI_RAD << 1, ROI_RAD << 1));
					imshow("hist" + to_string(i) + to_string(j), getHist(roi[i][j], i, j));
				}
			}
			imshow("find", find_draw());
			break;
		}
	}
	waitKey(0);
	return 0;
}

Mat find_draw() {
	Mat ret;
	cvtColor(table, ret, COLOR_GRAY2RGB);
	int diff[4][4];
	int match[4];

	// sum differences of the histograms
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			Mat tmp = abs(hist_v[0][i] - hist_v[1][j]);
			diff[i][j] = sum(tmp)[0];
		}
	}

	// find matches
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			cout << "first:" << i << ", second:" << j << ", diff:" << diff[i][j] << '\n';
		}
		similar[i] = min_element(diff[i], diff[i] + 4) - diff[i];
		cout << i << " similar with " << similar[i] << '\n';
	}

	//draw circles & lines
	for (int i = 0; i < 4; i++) {
		circle(ret, point[0][i], ROI_RAD, color[i], 1, LINE_AA, 0);
		circle(ret, point[1][i], ROI_RAD, color[i], 1, LINE_AA, 0);
		line(ret, point[0][i], point[1][similar[i]], color[i], 3, FILLED, 0);
	}
	return ret;
}

Mat getHist(Mat roi, int i, int j) {
	Mat hist = calc(roi);
	Mat histImage(512, 512, CV_8U);

	normalize(hist, hist, 0, histImage.rows, NORM_MINMAX, CV_32F);
	hist.copyTo(hist_v[i][j]);
	histImage = Scalar(255);
	int binW = cvRound((double)histImage.cols / histSize);
	int X1, Y1, X2, Y2;
	for (int i = 0; i < histSize; i++) {
		X1 = i * binW;
		Y1 = histImage.rows;
		X2 = (i + 1) * binW;
		Y2 = histImage.rows - cvRound(hist.at<float>(i));
		rectangle(histImage, Point(X1, Y1), Point(X2, Y2), Scalar(0), -1);
	}

	return histImage;
}

Mat calc(Mat roi) {
	int cx = ROI_RAD;
	int cy = ROI_RAD;
	Mat hist = Mat::zeros(1, histSize, CV_8UC1);
	for (int i = 0; i < roi.rows; i++) {
		for (int j = 0; j < roi.cols; j++) {
			// ignore points outside the circle
			if ((pow(cx - i, 2) + pow(cy - j, 2)) <= ROI_RAD * ROI_RAD) {
				hist.at<uchar>(0, roi.at<uchar>(i, j) / (256 / histSize))++;
			}
		}
	}
	return hist;
}

void onMouse(int event, int x, int y, int flags, void* param) {
	switch (event) {

	case EVENT_LBUTTONDOWN:
		point[idx / 4][idx % 4].x = x;
		point[idx / 4][idx % 4].y = y;
		cout << "image" << idx / 4 << " point" << idx % 4 << '\n';
		cout <<point[idx / 4][idx % 4].x << ' ' << point[idx / 4][idx % 4].y << '\n';
		idx++;
		break;
	}
}