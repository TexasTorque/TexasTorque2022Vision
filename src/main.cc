//
// Copyright (c) Texas Torque 2022
//
// Authors: Justus, 
//

#include <iostream>
#include <stdexcept>

#include <networktables/NetworkTable.h>
#include <networktables/NetworkTableInstance.h>

#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/objdetect.hpp"
#include "opencv2/videoio.hpp"

class Bound {
  public:
    cv::Scalar upper, lower;
    Bound(cv::Scalar u, cv::Scalar l) : upper(u), lower(l) {
    }
};

// Am going to replace above w below
/*
typedef struct {
	cv::Scalar upper, lower;
} Bound;

Bound newBound(cv::Scalar l, cv::Scalar u) {
	return { .lower = l , .upper = u }:
}
*/

// NetworkTable Spec:

// Table:       ball_detection
//   Entry:     alliance_color 
//     Type:    string
//     Value:   "blue"
//     Value:   "red"
//     Default: "none"
//   Entry:     ball_horizontal_position
//     Type:    double
//     Value:   -1/2w < x < 1/2w
//     Default: 0.0

nt::NetworkTable initializeNetworkTable(std::string identifier) {
    nt::NetworkTableInstance ntInstance = nt::NetworkTableInstance::GetDefault();
    std::shared_ptr<nt::NetworkTable> programTable = ntInstance.GetTable(identifier);
    return (*programTable.get());
}

Bound fetchDetectionBounds(nt::NetworkTable tableInstance) {
    std::string color = tableInstance.GetEntry("alliance_color").GetString("none");
    if (color == "none") throw std::runtime_error("Alliance color could not read from network tables");
    else if (color == "red") return Bound(cv::Scalar(170, 70, 50), cv::Scalar(180, 255, 255));
    else if (color == "blue") return Bound(cv::Scalar(90, 50, 70), cv::Scalar(128, 255, 255));
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

int main(int argc, char** argv) {
  	
	nt:NetworkTable programTable = initializeNetworkTable("ball_detection");

    Bound detectionBounds = fetchDetectionBounds(ntInstance);

    cv::VideoCapture capture = initializeVideoCapture(0);
	
	// Allocating buffers - plz allocate as many buffers as you can to save on meory allocation
    cv::Mat frame, mask, ellipse;

    nt::NetworkTableEntry ballEntry = programTable.GetEntry("ball_horizontal_position");

    // Initialize program loop while reading
    // frames and incrementing frame counter
    for (int fc = 0; capture.read(frame); fc++) {
		checkForFrameEmpty(frame);
		
		

        ballEntry.SetDouble(-1);

        // Output log and frame while checking for keyboard break
	
    }
}
