#include <iostream>
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

enum Alliance { RED, BLUE };

Alliance getAllianceFromNT(int nt) {
    // return RED;
    return BLUE;
}

Bound detectionRange(Alliance alliance) {
    if (alliance == RED)
        return Bound(cv::Scalar(170, 70, 50), cv::Scalar(180, 255, 255));
    else if (alliance == BLUE)
        return Bound(cv::Scalar(90, 50, 70), cv::Scalar(128, 255, 255));
    std::printf("Invalid alliance\n");
    exit(-1);
    return Bound(cv::Scalar(0, 0, 0), cv::Scalar(0, 0, 0));
}

int main(int argc, char** argv) {
    nt::NetworkTableInstance ntInstance =
            nt::NetworkTableInstance::GetDefault();

    // We can probably use a single table for everything.
    // Do we really have enough entries to warrent it?

    std::shared_ptr<nt::NetworkTable> allianceColorTable =
            ntInstance.GetTable("AllianceColor");
    bool isRed =
            (*allianceColorTable.get()).GetEntry("RedBlue").GetBoolean(false);

    // Possible one liner
    // (*ntInstance.GetTable("AllianceColor").get()).GetEntry("RedBlue").GetBoolean(false);

    int cameraDevice = 0;
    cv::VideoCapture capture;
    capture.open(cameraDevice);
    if (!capture.isOpened())
        std::printf("[ERROR] Cannot open video capture!\n");

    cv::Mat frame, mask, ellipse;

    Alliance alliance = getAllianceFromNT(0);

    std::shared_ptr<nt::NetworkTable> ballTable =
            ntInstance.GetTable("BallTable");
    nt::NetworkTableEntry ballEntry = (*ballTable.get()).GetEntry("ballentry");

    // Initialize program loop while reading
    // frames and incrementing frame counter
    for (int fc = 0; capture.read(frame); fc++) {
        if (frame.empty()) {
            std::printf("[ERROR] Frame is empty!\n");
            break;
        }

        // Update the mask
        std::printf(isRed ? "Red" : "Blue");

        ballEntry.SetDouble(-1);

        // Output log and frame while checking for keyboard break
    }
}
