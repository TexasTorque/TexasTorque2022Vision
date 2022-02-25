//
// Copyright (c) Texas Torque 2022
//
// Authors: Justus, Jacob, Omar, Jack
//

#ifndef TEXASTORQUE_INTAKE
#define TEXASTORQUE_INTAKE

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

namespace texastorque {
    class IntakePipe : public frc::VisionPipeline {
    public:
        nt::NetworkTableEntry ballPosition;
        nt::NetworkTableEntry ballRadius;
        cs::CvSource cvSource;
        IntakeBound bounds = IntakeBound(RED);
        cv::Mat frame;

        IntakePipe(std::string name, nt::NetworkTableInstance& ntinst, Color color);

        void Process(cv::Mat& input) override;

        void checkForFrameEmpty(cv::Mat frame);

        cv::Vec3f fetchBiggestCircle(std::vector <cv::Vec3f> circles);
    };
}

#endif