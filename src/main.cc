//
// Copyright (c) Texas Torque 2022
//
// Authors: Justus, Jacob, Omar, Jack
//

#include <chrono>
#include <ctime>
#include <iostream>
#include <stdexcept>

#include <networktables/NetworkTable.h>
#include <networktables/NetworkTableInstance.h>

#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/objdetect.hpp"
#include "opencv2/videoio.hpp"

#define PTR(sharedPtr) (*sharedPtr.get()) // Very unsafe - for use by C++ Professionals(TM) only!
typedef std::shared_ptr<nt::NetworkTable> NetworkTablePointer;

class Bound { public:
    cv::Scalar upper, lower;
    Bound(cv::Scalar u, cv::Scalar l) : upper(u), lower(l) {}
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
	return nt::NetworkTableInstance::GetDefault().GetTable(identifier);
}

Bound fetchDetectionBounds(NetworkTablePointer tablePointer) {
    std::string color = PTR(tablePointer).GetEntry("alliance_color").GetString("none");
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
  	
	NetworkTablePointer programTablePointer = initializeNetworkTable("ball_detection");

    //Bound detectionBounds = fetchDetectionBounds(programTablePointer);

    cv::VideoCapture capture = initializeVideoCapture(0);
	
	// Allocating buffers - plz allocate as many buffers as you can to save on meory allocation
    cv::Mat frame, mask, ellipse;

    nt::NetworkTableEntry ballEntry = PTR(programTablePointer).GetEntry("ball_horizontal_position");
    nt::NetworkTableEntry fpsEntry = PTR(programTablePointer).GetEntry("frames_per_second");
	
	StopWatch timer = StopWatch();
    // Initialize program loop while reading
    // frames and incrementing frame counter

	std::string color = PTR(programTablePointer).GetEntry("alliance_color").GetString("none");


    for (long count = 0; capture.read(frame); count++) {
		checkForFrameEmpty(frame);
		double fps = timer.calculateFPS(count);	

		fpsEntry.SetDouble(fps);

		//if (count % 25 == 0) std::printf("FPS: %d : %f", fps, 0.);
		//detectionBounds.lower[0]);
		
        ballEntry.SetDouble(count);
    }
}
