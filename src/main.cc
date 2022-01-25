//
// Copyright (c) Texas Torque 2022
//
// Authors: Justus, Jacob, Omar, Jack
//

#include <algorithm>
#include <chrono>
#include <ctime>
#include <iostream>
#include <stdexcept>

#include "networktables/NetworkTable.h"
#include "networktables/NetworkTableInstance.h"
#include "cameraserver/CameraServer.h"

#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/objdetect.hpp"
#include "opencv2/videoio.hpp"

#define PTR(sharedPtr) (*sharedPtr.get()) // Very unsafe - for use by C++ Professionals(TM) only!
typedef std::shared_ptr<nt::NetworkTable> NetworkTablePointer;

enum Color { RED, BLUE };

class Bound { 
public:
	Color color;
    cv::Scalar upper, lower;
    Bound(Color color) : color(color) {
		if (color == RED) {
			upper = cv::Scalar(43, 18, 234);
			lower = cv::Scalar(110, 172, 255);
		} else if (color == BLUE) {
    		        upper = cv::Scalar(90, 50, 70);
			lower = cv::Scalar(128, 255, 255);
		} else
			throw std::runtime_error("Unknown color");
	}
};

// NetworkTable Spec:

// Table:       ball_detection
//   Entry:     alliance_color 
//     Type:    string
//     Value:   "blue"
//     Value:   "red"
//     Default: "none"
//   Entry:     x
//     Type:    double
//     Value:   0 <= x <= 640
//     Default: 0.0
//   Entry: 	r
//     Type:    double
//     Value: 	0 <= 640
//     Default: 0.0

NetworkTablePointer initializeNetworkTable(std::string identifier) {
	auto instance = nt::NetworkTableInstance::GetDefault();
	instance.StartClientTeam(1477);
	return instance.GetTable(identifier);
}

Bound fetchDetectionBounds(NetworkTablePointer tablePointer, std::string color) {
    if (color == "none") throw std::runtime_error("Alliance color could not read from network tables");
    else if (color == "red") return Bound(RED);
    else if (color == "blue") return Bound(BLUE);
    else throw std::runtime_error("Alliance color could not be parsed");
}


void checkForFrameEmpty(cv::Mat frame) {
	if (frame.empty())
		throw std::runtime_error("Frame is empty");
}

 cv::Vec3f fetchBiggestCircle(std::vector<cv::Vec3f> circles) {
     cv::Vec3f biggest;
     int bigrad = 0;
     for (size_t i = 0; i < circles.size(); i++) {
         if(circles[i][2] > bigrad) {
             bigrad = circles[i][2];
             biggest = circles[i];
         }
     }
 	return biggest;
 }

void processCenter(cv::Mat* input, Bound bounds, nt::NetworkTableEntry* xEntry, nt::NetworkTableEntry* rEntry) {
    if (bounds.color == RED) *input = ~*input; // invert color space to allow const
    // convert to HSV color space
    cvtColor(*input, *input, cv::COLOR_BGR2HSV);
    // remove non-colored range
    inRange(*input, bounds.lower, bounds.upper, *input);

    // blur together features
    medianBlur(*input, *input, 17);

    // erode extraneous data
    erode(*input, *input, cv::Mat(), cv::Point(-1, -1), 3);

    // dilate good data
    dilate(*input, *input, cv::Mat(), cv::Point(-1, -1), 5);

    // Remove small details
    morphologyEx(*input, *input, cv::MORPH_HITMISS, cv::Mat());

    // get circles using hough-circles
    std::vector<cv::Vec3f> circles;
    HoughCircles(*input, circles, cv::HOUGH_GRADIENT, 1, 25, 30, 15, 50, 0);

    // if there is a circle
    if(circles.size() > 0) {
        //get the largest one and send
        cv::Vec3f biggest = fetchBiggestCircle(circles);

        cv::Point center = cv::Point(biggest[0], biggest[1]);

        // draw circle center
        cv::circle(*input, center, 1, cv::Scalar(0, 100, 100), 3, cv::LINE_AA);

        // set x-value (px) and radius (px)
        xEntry->SetDouble(biggest[0]);
        rEntry->SetDouble(biggest[2]);
    } else {
        // if none are found, set default value (0)
        xEntry->SetDouble(0);
        rEntry->SetDouble(0);
    }
}

int main(int argc, char** argv) {
    NetworkTablePointer programTablePointer = initializeNetworkTable("ball_detection");

    std::string color = programTablePointer->GetEntry("alliance_color").GetString("none");

    Bound detectionBounds = fetchDetectionBounds(programTablePointer, color);

    cs::CvSink cvSink = frc::CameraServer::GetInstance()->GetVideo();
    cs::CvSource cvSource = frc::CameraServer::GetInstance()->PutVideo("VP front", 640, 480);
	
    cv::Mat frame;

    nt::NetworkTableEntry ballEntryX = programTablePointer->GetEntry("x");
    nt::NetworkTableEntry ballEntryR = programTablePointer->GetEntry("r");


    // Initialize program loop while reading
    while (true) {
            if(cvSink.GrabFrame(frame) == 0) {
                cvSource.NotifyError(cvSink.GetError());
            }
            processCenter(&frame, detectionBounds, &ballEntryX, &ballEntryR);

            cvSource.PutFrame(frame);
    }
}