//
// Copyright (c) Texas Torque 2022
//
// Authors: Justus, Jacob, Omar, Jack
//


#ifndef TEXASTORQUE_MAGAZINE
#define TEXASTORQUE_MAGAZINE

#include <algorithm>
#include <chrono>
#include <ctime>
#include <iostream>
#include <stdexcept>
#include <queue>

#include "cameraserver/CameraServer.h"
#include "networktables/NetworkTable.h"
#include "networktables/NetworkTableInstance.h"
#include "wpi/StringRef.h"
#include "wpi/json.h"
#include "wpi/raw_istream.h"
#include "wpi/raw_ostream.h"
#include "vision/VisionPipeline.h"
#include "vision/VisionRunner.h"

#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/objdetect.hpp"
#include "opencv2/videoio.hpp"

#include "Colors.hh"
#include "RollingMedian.hh"

namespace texastorque {

    class MagazinePipe : public frc::VisionPipeline {
    public:
        const double fullness = .4;

        nt::NetworkTableEntry ballColor;
        nt::NetworkTableEntry pixelPerc;
        nt::NetworkTableEntry pixelCount;
        nt::NetworkTableEntry pixelPercRoll;
        nt::NetworkTableEntry pixelCountRoll;

        cs::CvSource cvSource;

        RollingMedian redRollingMedian = RollingMedian(20);
        RollingMedian blueRollingMedian = RollingMedian(20);

        MagazinePipe(nt::NetworkTableInstance& ntinst);

        bool processBound(cv::Mat frame, Color color);


        void Process(cv::Mat& input) override;

        void checkForFrameEmpty(cv::Mat frame);
    };
}

#endif