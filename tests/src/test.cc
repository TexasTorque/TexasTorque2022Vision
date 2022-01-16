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

enum Shape { CIRCLE, TRIANGLE, SQUARE, RECTANGLE, PENTAGON };

int approxSidesOfContour(std::vector<cv::Point> points) {
    double arc = cv::arcLength(points, true);
    cv::approxPolyDP(points, points, 0.04 * arc, true);  
    return (int) points.size();
}

Shape detectShape(std::vector<cv::Point> points) {
    double arc = cv::arcLength(points, true);
    cv::approxPolyDP(points, points, 0.04 * arc, true);
    if (points.size() == 3) return TRIANGLE;
    else if (points.size() == 4) return RECTANGLE;
    else if (points.size() == 5) return PENTAGON;
    else if (points.size() == 8) return SQUARE;
    else return CIRCLE;
}

std::string getShapeString(Shape shape) {
    switch (shape) {
        case CIRCLE: return "circle";
        case TRIANGLE: return "triangle";
        case SQUARE: return "square";
        case RECTANGLE: return "rectangle";
        case PENTAGON: return "pentagon";
        default: return "unknown";
    }
}

std::vector<cv::Point> getLargestContourByArea(std::vector<std::vector<cv::Point>> contours) {
    int id = -1;
    for (double i = 0, area = 0; i < contours.size(); i++) {
        double newArea = cv::contourArea(contours.at(i));
        if (newArea <= area) continue;
        area = newArea;
        id = i;
    } 
    return contours[id];
} 

void update(cv::Mat* input, cv::Mat* output, Alliance alliance) {
        std::vector<cv::Point> points;
        cv::Mat ellipse = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
        Bound bound = detectionRange(alliance);

        cv::resize(*input, *input, cv::Size(), .5, .5, cv::INTER_LANCZOS4);

        cv::cvtColor(*input, *output, cv::COLOR_BGR2HSV);

        cv::inRange(*output, bound.upper, bound.lower, *output);

        cv::erode(*output, *output, ellipse);
        cv::dilate(*output, *output, ellipse);
        cv::dilate(*output, *output, ellipse);
        cv::erode(*output, *output, ellipse);

        std::vector<std::vector<cv::Point>> contours;
        
        cv::findNonZero(*output, points);

        cv::findContours(*output, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);

        //cv::drawContours(*input, contours, -1, cv::Scalar(255, 255, 255), 2);

        if (contours.size() <= 0) return;

        //std::sort(contours.begin(), contours.end(), cv::contourArea);
        //std::vector<cv::Point> contour = contours[0];
        std::vector<cv::Point> contour = getLargestContourByArea(contours);

        if (approxSidesOfContour(contour) <= 4) return;

        cv::Point2f _center;
        float radius;
        cv::minEnclosingCircle(contour, _center, radius);

        cv::Moments m = cv::moments(contour);

        cv::Point center = cv::Point(m.m10 / m.m00, m.m01 / m.m00);

        if (radius <= 50) return;

        cv::circle(*input, center, radius, cv::Scalar(0, 255, 0), 4);

        /*
        for (const std::vector<cv::Point>& contour : contours) {
            cv::RotatedRect rect = cv::minAreaRect(contour);
            cv::Point2f vertices[4];
            rect.points(vertices);

            if (rect.size.height < 25 || rect.size.width < 25) continue;

            if (rect.size.height * 5/4 < rect.size.width
                ||rect.size.height * 4/5 > rect.size.width )
                continue;


            for (int i = 0; i < 4; i++) {
                cv::line(*input, vertices[i], vertices[(i + 1) % 4], cv::Scalar(255, 255, 255), 2);
            } 

            //cv::putText(*input, getShapeString(detectShape(contour)), 
            //        rect.center, cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 255, 255), 2);
            
            cv::putText(*input, std::to_string(rect.size.width) + " x "
                    + std::to_string(rect.size.height),  rect.center, 
                    cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 255, 255), 2);
        }*/

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