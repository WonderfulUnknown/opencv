﻿#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include "math.h"

using namespace std;
using namespace cv;

#define size 35//卷积核大小
#define sigma 10//sigma越大，平滑效果越明显

double gaus[size][size];

void GetgaussianKernel()
{
	const double PI = 4.0*atan(1.0);//π
	int center = size / 2;
	double sum = 0;

	for (int i = 0; i < size; i++)
	{
		for (int j = 0; j < size; j++)
		{
			//忽略公式中的(1 / (2 * PI*sigma*sigma))*，不影响结果
			gaus[i][j] = exp(-((i - center)*(i - center) + (j - center)*(j - center)) / (2 * sigma*sigma));
			sum += gaus[i][j];
		}
	}
	//cout << "低通滤波矩阵：" << endl << endl;
	for (int i = 0; i < size; i++)
	{
		for (int j = 0; j < size; j++)
		{
			gaus[i][j] /= sum;
			//cout << gaus[i][j] << "  ";
		}
		//cout << endl << endl;
	}
}

Mat conv(Mat src, Mat kern)
{
	Mat out = src.clone();
	int row1 = src.rows;
	int col1 = src.cols;
	int row2 = kern.rows;
	int col2 = kern.cols;
	for (int k = 0; k < 3; k++)
	{
		for (int i = 0; i < row1 - row2; i++)
		{
			for (int j = 0; j < col1 - col2; j++)
			{
				int temp = 0;
				for (int ix = 0; ix < row2; ix++)
					for (int iy = 0; iy < col2; iy++)
						temp += src.at<Vec3b>(i + ix, j + iy)[k] * kern.at<double>(ix, iy);
				if (temp > 255)
					temp = 255;
				if (temp < 0)
					temp = 0;
				out.at<Vec3b>(i, j)[k] = temp;
			}
		}
	}
	return out;
}

Mat add(Mat mat1, Mat mat2, double alpha)
{
	Mat out = mat1.clone();
	int row = mat1.rows;
	int col = mat1.cols;
	int temp = 0;
	for (int k = 0; k < 3; k++)
	{
		for (int i = 1; i < row - 1; i++)
		{
			for (int j = 1; j < col - 1; j++)
			{
				temp = alpha * out.at<Vec3b>(i, j)[k] + (1.0 - alpha)*mat2.at<Vec3b>(i, j)[k];
				if (temp > 255)
					temp = 255;
				else if (temp < 0)
					temp = 0;
				out.at<Vec3b>(i, j)[k] = temp;
			}
		}
	}
	return out;
}

int main()
{
	Mat src_image1 = imread("dog.bmp");
	Mat src_image2 = imread("cat.bmp");
	//判断图像是否加载成功
	if (src_image1.data && src_image2.data)
		cout << "图像加载成功!" << endl << endl;
	else
	{
		cout << "图像加载失败!" << endl << endl;
		system("pause");
		return -1;
	}
	namedWindow("src_image1", WINDOW_AUTOSIZE);
	namedWindow("src_image2", WINDOW_AUTOSIZE);
	imshow("src_image1", src_image1);
	imshow("src_image2", src_image2);

	GetgaussianKernel();//生成高斯卷积核矩阵
	Mat gaus_kern = Mat(size, size, CV_64F, gaus);
	//Mat lap_kern = (Mat_<double>(3, 3) <<
	//	1, 1, 1,
	//	1, -8, 1,
	//	1, 1, 1);
	Mat dst_low, dst_high, dst;

	filter2D(src_image1, dst_low, src_image1.depth(), gaus_kern);
	//dst_low = conv(src_image1, gaus_kern);
	namedWindow("low_pass", WINDOW_AUTOSIZE);
	imshow("low_pass", dst_low);
	
	Mat dst_temp;
	//filter2D(src_image2, dst_high, src_image2.depth(), lap_kern);
	//dst_high = conv(src_image2, lap_kern);
	filter2D(src_image2, dst_temp, src_image2.depth(), gaus_kern);
	addWeighted(src_image2, 1, dst_temp, -1, 0, dst_high);
	namedWindow("high_pass", WINDOW_AUTOSIZE);
	imshow("high_pass", dst_high);

	namedWindow("hybrid_image", WINDOW_AUTOSIZE);
	//addWeighted(dst_low, 1, dst_high, 1, 0, dst);
	dst = add(dst_low, dst_high, 0.5);
	imshow("hybrid_image", dst);

	waitKey(0);
	return 0;
}