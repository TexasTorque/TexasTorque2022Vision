#include <iostream>
#include <ctime>
#include <chrono>

#include "opencv2/objdetect.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"

time_t timeNow() {
    using std::chrono::system_clock;
    return system_clock::to_time_t(system_clock::now());
}

class StopWatch { 
  private: time_t start;

  public:
    StopWatch(void) { restart(); }
    void restart(void) { start = timeNow(); }
    int elapsed(void) { return timeNow() - start; }
    double fps(int f) { if (int e = elapsed() > 0) return f / (double) e; else return 0; }
};

void fatal(const std::string& msg) {
    std::cerr << msg << std::endl;
    exit(1);
}

enum Alliance { RED, BLUE };

class Bound { 
    public:
    cv::Scalar upper, lower; 
    Bound(cv::Scalar u, cv::Scalar l) : upper(u), lower(l) {}
};


Bound detectionRange(Alliance alliance) {
    if (alliance == RED)
        return Bound(cv::Scalar(170, 70, 50), cv::Scalar(180, 255, 255));
    else if (alliance == BLUE)
        return Bound(cv::Scalar(90, 50, 70), cv::Scalar(128, 255, 255));
    else
        fatal("Invalid alliance");
    return Bound(cv::Scalar(0, 0, 0), cv::Scalar(0, 0, 0));
}

void update(cv::Mat* input, cv::Mat* output, Alliance alliance) {
        std::vector<cv::Point> points;
        cv::Mat ellipse = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
        Bound bound = detectionRange(alliance);

        // 50% resize leads to 50-75% performance improvement
        cv::resize(*input, *input, cv::Size(), .5, .5, cv::INTER_LANCZOS4);

        // Convert color to Hue-Saturation-Value scale
        cv::cvtColor(*input, *output, cv::COLOR_BGR2HSV);

        // Write the mask matrix from pixels within color bounds
        cv::inRange(*output, bound.upper, bound.lower, *output);


        // Apply morphological opening to remove noise
        cv::erode(*output, *output, ellipse);
        cv::dilate(*output, *output, ellipse);
        cv::dilate(*output, *output, ellipse);
        cv::erode(*output, *output, ellipse);

        // Find all identified points in the mask
        //cv::findNonZero(*output, points);
         
}

int main(int argc, char** argv) {

    int cameraDevice = 0;
	cv::VideoCapture capture;
    capture.open(cameraDevice);
    if (!capture.isOpened()) 
        std::printf("[ERROR] Cannot open video capture!\n");

    cv::Mat frame, mask, ellipse;

    StopWatch timer = StopWatch();

    // Initialize program loop while reading 
    // frames and incrementing frame counter
    for (int fc = 0; capture.read(frame); fc++) {
        if (frame.empty()) {
            printf("[ERROR] Frame is empty!\n");
            break;
        }

        // Update the mask
        update(&frame, &mask, BLUE);

        // Output log and frame while checking for keyboard break 
        cv::imshow("[INPUT]", frame);
        cv::imshow("[OUTPUT]", mask);
        if (cv::waitKey(10) == 27) break;
    }

    return 0;
}