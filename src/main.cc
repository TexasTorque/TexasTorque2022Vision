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

// Am maybe going to replace above w/ below
/*
typedef struct {
	cv::Scalar upper, lower;
} Bound;

Bound newBound(cv::Scalar l, cv::Scalar u) {
	return { .lower = l , .upper = u }:
}
*/

time_t fetchTimeNow() {
	using std::chrono::system_clock;
	return system_clock::to_time_t(system_clock::now());
}

class StopWatch {
	private:
	time_t start;

  	public: 
	StopWatch(void) { restart(); }
	void restart(void) { this->start = fetchTimeNow(); }
	long elapsed(void) { return fetchTimeNow() - this->start; };
	long calculateFPS(long frame) {
		long span = elapsed();
		if (span > 0) return frame / span;
		return 0;
	}
};

// NetworkTable Spec:

// Table:       ball_detection
//   Entry:     alliance_color 
//     Type:    string
//     Value:   "blue"
//     Value:   "red"
//     Default: "none"
//   Entry:     ball_horizontal_position
//     Type:    double
//     Value:   -1/2w <= x <= 1/2w
//     Default: 0.0
//   Entry: 	frames_per_second
//     Type:    double
//     Value: 	0 <= x
//     Default: -1.0

NetworkTablePointer initializeNetworkTable(std::string identifier) {
	//return nt::NetworkTableInstance::GetDefault().GetTable(identifier);
	auto instance = nt::NetworkTableInstance::GetDefault();
	instance.StartClientTeam(1477);
	//instance.StartDSClient();
	while (!instance.IsConnected());
		//std::printf("NOT CONNECTED\n");
	return instance.GetTable(identifier);
}

Bound fetchDetectionBounds(NetworkTablePointer tablePointer) {
    //std::string color = PTR(tablePointer).GetEntry("alliance_color").GetString("none");
	std::string color = tablePointer->GetEntry("alliance_color").GetString("none");

    if (color == "none") throw std::runtime_error("Alliance color could not read from network tables");
    else if (color == "red") return Bound(RED);
    else if (color == "blue") return Bound(BLUE);
    else throw std::runtime_error("Alliance color could not be parsed");
}

cv::VideoCapture initializeVideoCapture(int cameraDevice) {
    cv::VideoCapture capture;
    capture.open(cameraDevice);
    if (!capture.isOpened()) throw std::runtime_error("Could not open video capture");
	return capture;
}

void checkForFrameEmpty(cv::Mat frame) {
	if (frame.empty())
		throw std::runtime_error("Frame is empty");
}

// cv::Vec3f fetchBiggestCircle(std::vector<cv::Vec3f> circles) {
//     cv::Vec3f biggest;
//     int bigrad = 0;
//     for (size_t i = 0; i < circles.size(); i++) {
//         if(circles[i][2] > bigrad) {
//             bigrad = circles[i][2];
//             biggest = circles[i];
//         }
//     }
// 	return biggest;
// }

cv::Vec3f fetchBiggestCircle(std::vector<cv::Vec3f> circles) {
	return *std::max_element(circles.begin(), circles.end(),
			[](const cv::Vec3f& a, const cv::Vec3f& b) { // this is a C++ lambda - >25% /g/ approved!
		return a[2] < b[2];
	});
}	

cv::Point processCenter(cv::Mat* input, cv::Mat* output, Bound bounds) {
    if (bounds.color == RED) *input = ~*input; // invert color space to allow const
    cvtColor(*input, *output, cv::COLOR_BGR2HSV);
    inRange(*output, bounds.lower, bounds.upper, *output);

    medianBlur(*output, *output, 17);
    erode(*output, *output, cv::Mat(), cv::Point(-1, -1), 3);
    dilate(*output, *output, cv::Mat(), cv::Point(-1, -1), 5);
    morphologyEx(*output, *output, cv::MORPH_HITMISS, cv::Mat());

    std::vector<cv::Vec3f> circles;
    HoughCircles(*output, circles, cv::HOUGH_GRADIENT, 1, 25, 30, 15, 50, 0);
	cv::Vec3f biggest = fetchBiggestCircle(circles);

    cv::Point center = cv::Point(biggest[0], biggest[1]);
    // circle center
    cv::circle(*input, center, 1, cv::Scalar(0,100,100), 3, cv::LINE_AA);

    // circle outline
	//int radius = biggest[2];
	//circle(*input, center, radius, cv::Scalar(255,0,255), 3, cv::LINE_AA);
    //
	//cv::Moments m = cv::moments(*output);
	//double hu[7];
	//cv::HuMoments(m, hu);
	//int cX = m.m10 / m.m00;
	//int cY = m.m01 / m.m00;
	//cv::circle(*output, cv::Point(cX, cY), 5, (255, 255, 255), -1);

	return center;
}

int main(int argc, char** argv) {
	
	NetworkTablePointer programTablePointer = initializeNetworkTable("ball_detection");

    Bound detectionBounds = fetchDetectionBounds(programTablePointer);
	
    //cv::VideoCapture capture = initializeVideoCapture(0);

	frc::CameraServer* cameraServer = frc::CameraServer::GetInstance();
	cs::CvSink cvSink = cameraServer->GetVideo("USB Camera 0");
	cs::CvSource cvSource = cameraServer->PutVideo("USB Cam", 320, 240);
	
	// Allocating buffers - plz allocate as many buffers as you can to save on meory allocation
    cv::Mat frame, output, ellipse;

    nt::NetworkTableEntry ballEntryX = programTablePointer->GetEntry("x");
    nt::NetworkTableEntry ballEntryR = programTablePointer->GetEntry("r");
    nt::NetworkTableEntry fpsEntry = programTablePointer->GetEntry("frames_per_second");
	
	StopWatch timer = StopWatch();
    // Initialize program loop while reading
    // frames and incrementing frame counter

	std::string color = programTablePointer->GetEntry("alliance_color").GetString("none");
        printf("hello\n");

    //for (long count = 0; capture.read(frame); count++) {
	//for (long count = 0; long _ = cvSink.GrabFrame(frame); count++) {
	long count = 0;
	while (true) {
		long c = cvSink.GrabFrame(frame);
		if(c == 0) continue;
		//checkForFrameEmpty(frame);
		double fps = timer.calculateFPS(count);	

		cv::Point center = processCenter(&frame, &output, detectionBounds);

		fpsEntry.SetDouble(fps);
                ballEntryX.SetDouble(center.x);
		//if (count % 25 == 0) std::printf("FPS: %d : %f", fps, 0.);
		//detectionBounds.lower[0]);

		cvSource.PutFrame(output);

        //ballEntry.SetDouble(count);
		if (count % 150 == 0) timer.restart();
		count++;
    }
}