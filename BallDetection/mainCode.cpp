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
		1. 빨간색 계통의 색깔 (그룹핑) -> 찾고 싶다.
		2. R G B
			=> 이거 해주는 거 R> 80 && G B < 50 나머지 없애줘
		3. Mat parameter RGB 값들이
		4. 처리된 이미지에서 하얀색 부분의 좌표값들을 모아 자료구조에 넣어
		5. 경계선을 찾음
		6. 등고선을 찾아준다 -> 경계선 이게 벡터에 저장돼 근데? 벡터에 어떻게 저장되냐
		벡터에 저장이 됐어 -> 구조는 모름
		7. 구조를 모르면 여기 위에 페인트를 칠해야하는데 어떻게 칠함? Point <- <Point>
		Contour에 저장돼있 경계선이 아니라 하얀색 부분의 
		경계 찾은다음 중심점 찾고
		경계에 칠함
		중심찍어 놓 radius 그리면되겠네
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

		// Canny로 edge들을 뽑아낸다. (Thresh 범위 안의 것들을 edge로 받아들인다)
		Canny(red, canny_output, THRESH, THRESH * 2, 3);
		//THRESH의 값 down threshold, upper threshold
		//Gradient (3 * 3 행렬 사용해서 필터링 -> x와 y의 대비를 심각하게 해서 edge를 구하는 방법)
		
		// Canny로부터 뽑아낸 edge들을 contour로 변환한다.
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

				//prev center와 비교해서 같은 분류면 Valid!
				if ( prev_center.x != 0 && prev_center.y != 0 ) { //처음이 아닌 경우
							
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
				if(validPoint) { //valid point인 경우에만 합산
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
	그리는
	circle(img, center, 3, Scalar(0, 255, 0), -1, 8, 0);
	마지막에서 세번째는 thickness를 뜻하는데, 음수이면 채워진 원을 그림, 8은 linetype이다. 실선을 의미하는 등, 마지막 0은 fraction bits
	circle(img, center, 반지름, Scalar(0, 255, 0), 3, 8, 0);
	*/
	//중심점 x,y가 있다고 할 때

	cv::destroyAllWindows();
	vc.release();
	return 0;
}
