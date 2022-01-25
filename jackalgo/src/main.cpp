#include <opencv2/core.hpp>
#include "opencv2/imgproc.hpp"
#include <opencv2/highgui.hpp>
#include <opencv2/photo.hpp>

#include <iostream>

using namespace cv;
using namespace std;

const bool red = false;

void process(Mat* input, Mat* output) {
    if(red) {
        *input = ~*input; // invert color space to allow const
        cvtColor(*input,*output, COLOR_BGR2HSV);
        inRange(*output, Scalar(43, 18, 234), Scalar(110, 172, 255), *output);
    } else {
        cvtColor(*input, *output, COLOR_BGR2HSV);
        inRange(*output, Scalar(90, 50, 70), Scalar(128, 255, 255), *output);
    }
    medianBlur(*output, *output, 17);
    erode(*output, *output, Mat(), Point(-1, -1), 3);
    dilate(*output, *output, Mat(), Point(-1, -1), 5);
    morphologyEx(*output, *output, MORPH_HITMISS, Mat());

    vector<Vec3f> circles;
    HoughCircles(*output, circles, HOUGH_GRADIENT, 1, 25, 30, 15, 50, 0);
    Vec3f biggest;
    int bigrad = 0;
    for( size_t i = 0; i < circles.size(); i++ ) {
        if(circles[i][2] > bigrad) {
            bigrad = circles[i][2];
            biggest = circles[i];
        }
    }
    Point center = Point(biggest[0], biggest[1]);
    // circle center
    circle( *input, center, 1, Scalar(0,100,100), 3, LINE_AA);
    // circle outline
//    int radius = biggest[2];
//    circle( *input, center, radius, Scalar(255,0,255), 3, LINE_AA);
    //
//    Moments m = moments(*output);
//    double hu[7];
//    HuMoments(m, hu);
//    int cX = m.m10 / m.m00;
//    int cY = m.m01 / m.m00;
//    circle(*output, Point(cX, cY), 5, (255, 255, 255), -1);
}

int main() {
  int id = 0;
  VideoCapture capture;
  capture.open(id);

  while(true) {
	Mat frame;
    Mat output;
    capture.read(frame);
    if(frame.empty()) {
        printf("Frame not empty!\n");
        break;
    }


    process(&frame, &output);


    imshow("input", frame);
    imshow("output", output);

    waitKey(1);
  }

  return 0;
}
