#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <stdio.h>
#include <iostream>
#include <cstdio>
#include <Windows.h>
#define THRESH 100
#define INT_INFINITY 2147438648
#define BIAS 40
using namespace cv;
using namespace std;

int main() {
	VideoCapture vc("moving_ball.avi");
	Mat img, img_hsv;
	Point prev_center = Point(0,0);
	double prev_radius = 0;

	while(1) {
		vc >> img;
		if(img.empty()) break;
		//imshow("video", img);
		if(waitKey(10) == 27) break;
		
		/*
		Logics
		1. ������ ������ ���� (�׷���) -> ã�� �ʹ�.
		2. R G B
			=> �̰� ���ִ� �� R> 80 && G B < 50 ������ ������
		3. Mat parameter RGB ������
		4. ó���� �̹������� �Ͼ�� �κ��� ��ǥ������ ���� ��� �ڷᱸ���� �־�
		5. ��輱�� ã��
		6. ����� ã���ش� -> ��輱 �̰� ���Ϳ� ����� �ٵ�? ���Ϳ� ��� ����ǳ�
		���Ϳ� ������ �ƾ� -> ������ ��
		7. ������ �𸣸� ���� ���� ����Ʈ�� ĥ�ؾ��ϴµ� ��� ĥ��? Point <- <Point>
		Contour�� ������� ��輱�� �ƴ϶� �Ͼ�� �κ��� 
		��� ã������ �߽��� ã��
		��迡 ĥ��
		�߽���� �� radius �׸���ǰڳ�
		*/

		cvtColor(img, img_hsv, CV_BGR2HSV);
		Mat bg, fc;
		inRange(img_hsv, Scalar(0, 100, 100), Scalar(10, 255, 255), bg);
		inRange(img_hsv, Scalar(160, 100, 100), Scalar(179, 255, 255), fc);
      
		Mat red;
		addWeighted(bg, 1.0, fc, 1.0, 0.0, red);
		GaussianBlur(red, red, cv::Size(9, 9), 2, 2);
		
		Mat canny_output;
		vector<vector<Point>> contours;
		vector<Vec4i> hierarchy;

		// Canny�� edge���� �̾Ƴ���. (Thresh ���� ���� �͵��� edge�� �޾Ƶ��δ�)
		Canny(red, canny_output, THRESH, THRESH * 2, 3);
		//THRESH�� �� down threshold, upper threshold
		//Gradient (3 * 3 ��� ����ؼ� ���͸� -> x�� y�� ��� �ɰ��ϰ� �ؼ� edge�� ���ϴ� ���)
		
		// Canny�κ��� �̾Ƴ� edge���� contour�� ��ȯ�Ѵ�.
		findContours(canny_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
		//printf("(%d, %d)\n",contours[0][0].x,contours[0][0].y);
		
		

		int min_x = INT_INFINITY;
		int min_y = INT_INFINITY;
		int max_x = 0;
		int max_y = 0;

		int prev_x = 0;
		int prev_y = 0;


		for(int i = 0 ; i < contours.size() ; i++) {
			for ( int j = 0; j < contours[i].size() ; j++) {
				boolean validPoint = false;
				int x = contours[i][j].x;
				int y = contours[i][j].y;

				//prev center�� ���ؼ� ���� �з��� Valid!
				if ( prev_center.x != 0 && prev_center.y != 0 ) { //ó���� �ƴ� ���
							
					double distance =  std::sqrt( (prev_center.x-x)*(prev_center.x-x) + (prev_center.y-y)*(prev_center.y-y) );
					//printf("(%d, %d) prev_center\n", prev_center.x, prev_center.y);
					//printf("distance : %lf\n", distance);
					//printf("prev_radius : %lf\n", prev_radius);
					if ( distance <= prev_radius + BIAS ) {
						validPoint = true;
					} 
				} else {
					validPoint = true;
				}

				//min ~ max
				if(validPoint) { //valid point�� ��쿡�� �ջ�
					if( i == 0 && j == 0 ) {
						//init min & max
						min_x = contours[i][j].x;
						min_y = contours[i][j].y;
						max_x = contours[i][j].x;
						max_y = contours[i][j].y;

					} else {
						if(contours[i][j].x < min_x)
							min_x = contours[i][j].x;
						if(contours[i][j].y < min_y)
							min_y = contours[i][j].y;
						if(contours[i][j].x > max_x)
							max_x = contours[i][j].x;
						if(contours[i][j].y > max_y)
							max_y = contours[i][j].y;
					} //if - else
				}
			} //for j
		} //for i

		Point new_center = Point(0,0);
		double new_radius = 0;
		//center point
		new_center.x = (min_x + max_x) / 2;
		new_center.y = (min_y + max_y) / 2;
		new_radius = (max_x-min_x) / 2;

		circle(img, new_center, 3, Scalar(0, 255, 0), -1, 8, 0);
		circle(img, new_center, new_radius, Scalar(0, 255, 0), 3, 8, 0);
		imshow("video", img);
		/*
		//puts("===================================");
		//printf("(%d, %d) center\n", new_center.x, new_center.y);
		//printf("new_radius : %lf", new_radius);
		//puts("===================================");
		*/
		prev_center.x = new_center.x;
		prev_center.y = new_center.y;
		prev_radius = new_radius;

	}//while
	/*
	�׸���
	circle(img, center, 3, Scalar(0, 255, 0), -1, 8, 0);
	���������� ����°�� thickness�� ���ϴµ�, �����̸� ä���� ���� �׸�, 8�� linetype�̴�. �Ǽ��� �ǹ��ϴ� ��, ������ 0�� fraction bits
	circle(img, center, ������, Scalar(0, 255, 0), 3, 8, 0);
	*/
	//�߽��� x,y�� �ִٰ� �� ��

	cv::destroyAllWindows();
	vc.release();
	return 0;
}