#include <opencv2\core.hpp>
#include <opencv2\imgproc.hpp>
#include <opencv2\highgui.hpp>

using namespace cv;
using namespace std;

#include <stdlib.h>
#include <stdio.h>
#include <iostream>

Mat combineMats(Mat im1, Mat im2)
{
	Size sz1 = im1.size();
	Size sz2 = im2.size();

	Mat combineImage(sz1.height, sz1.width + sz2.width, CV_8UC3);

	Mat left(combineImage, Rect(0, 0, sz1.width, sz1.height));
	im1.copyTo(left);
	
	Mat right(combineImage, Rect(sz1.width, 0, sz2.width, sz2.height));
	im2.copyTo(right);
	
	return combineImage;
}

Mat makeLeftMat(Mat img) 
{
	Mat transform(img.size().height, img.size().width, CV_8UC3);
	img.copyTo(transform);			//скопировали оригинальное изображение

	for (int i = 0; i < img.size().width; i++)					//ищем нужные пиксели и перекрашиваем
	{
		for (int j = 0; j < img.size().height; j++)
		{
			Vec3b pixel = transform.at<Vec3b>(j, i);			//разбивка на каналы
			uchar b = pixel[0];	//b,g,r
			uchar g = pixel[1];	//b,g,r
			uchar r = pixel[2];	//b,g,r

			int result = (g > (int)r+30 && g > (int)b+30) ? 255 : 0;
			transform.at<Vec3b>(j, i) = Vec3b(result, result, result);
		}
	}

	Mat transformGray;
	cvtColor(transform, transformGray, CV_BGR2GRAY);
	blur(transformGray, transformGray, Size(3, 3));

	Mat threshold_output;
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

	threshold(transformGray, threshold_output, 100, 255, THRESH_BINARY);	  // Detect edges using Threshold

	findContours(threshold_output, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));	  // Find contours
	
	// Approximate contours to polygons + get bounding rects
	vector<vector<Point>> contours_poly(contours.size());
	vector<Rect> boundRect(contours.size());

	for (int i = 0; i < contours.size(); i++)
	{
		approxPolyDP(Mat(contours[i]), contours_poly[i], 3, true);
		boundRect[i] = boundingRect(Mat(contours_poly[i]));
	}
	
	/// Draw polygonal contour + bonding rects + circles
	for (int i = 0; i < contours.size(); i++)
	{
		if (boundRect[i].width > 5 && boundRect[i].height > 5) {
			rectangle(img, boundRect[i].tl(), boundRect[i].br(), Scalar(0, 0, 255), 2, 8, 0);
			rectangle(transform, boundRect[i].tl(), boundRect[i].br(), Scalar(0, 0, 255), 2, 8, 0);
		}
	}

	return transform;
}

void captureFromCamera(){
	VideoCapture cap(0); // open the video camera no. 0

	if (!cap.isOpened())  // if not success, exit program 
	{
		cout << "Cannot open the video cam" << endl;
		return;
	}

	double dWidth = cap.get(CV_CAP_PROP_FRAME_WIDTH); //get the width of frames of the video
	double dHeight = cap.get(CV_CAP_PROP_FRAME_HEIGHT); //get the height of frames of the video

	cout << "Frame size : " << dWidth << " x " << dHeight << endl;

	namedWindow("MyVideo", CV_WINDOW_AUTOSIZE); //create a window called "MyVideo"

	while (1)
	{
		Mat frame;
		bool bSuccess = cap.read(frame); // read a new frame from video

		if (!bSuccess) //if not success, break loop
		{
			cout << "Cannot read a frame from video stream" << endl;
			break;
		}

		Mat sideBySide = combineMats(frame, makeLeftMat(frame));			//комбинируем два результирующих изображения вместе
		
		imshow("MyVideo", sideBySide);

		if (waitKey(30) == 27) //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
		{
			cout << "esc key is pressed by user" << endl;
			break;
		}
	}

}

void captureFromFile(string filepath){
	VideoCapture cap(filepath); // open the video file for reading

	if (!cap.isOpened())  // if not success, exit program
	{
		cout << "Cannot open the video file" << endl;
		return;
	}

	//cap.set(CV_CAP_PROP_POS_MSEC, 300); //start the video at 300ms

	double fps = cap.get(CV_CAP_PROP_FPS); //get the frames per seconds of the video

	cout << "Frame per seconds : " << fps << endl;

	namedWindow("MyVideo", CV_WINDOW_AUTOSIZE); //create a window called "MyVideo"

	while (1)
	{
		Mat frame;

		bool bSuccess = cap.read(frame); // read a new frame from video

		if (!bSuccess) //if not success, break loop
		{
			cout << "Cannot read the frame from video file" << endl;
			break;
		}

		imshow("MyVideo", frame); //show the frame in "MyVideo" window

		if (waitKey(30) == 27) //wait for 'esc' key press for 30 ms. If 'esc' key is pressed, break loop
		{
			cout << "esc key is pressed by user" << endl;
			break;
		}
	}
}

int main(int argc, char* argv[])
{
	captureFromCamera();
	return 0;
}