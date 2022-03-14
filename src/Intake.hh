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

namespace texastorque {
    class IntakePipe : public frc::VisionPipeline {
    public:

        const cv::Scalar lowerRed = cv::Scalar(0, 60, 60);
        const cv::Scalar upperRed = cv::Scalar(40, 255, 255);
        const cv::Scalar lowerBlue = cv::Scalar(90, 50, 50);
        const cv::Scalar upperBlue = cv::Scalar(128, 255, 255);

        nt::NetworkTableEntry alliance;
        nt::NetworkTableEntry ballPosition; 
        nt::NetworkTableEntry ballRadius;
        cs::CvSource cvSource;
        cv::Mat frame;
        time_t lastTime;
        bool isRed;

        nt::NetworkTableEntry lowerH;
        nt::NetworkTableEntry lowerS;
        nt::NetworkTableEntry lowerV;

        nt::NetworkTableEntry upperH;
        nt::NetworkTableEntry upperS;
        nt::NetworkTableEntry upperV;

        cv::Scalar upper, lower;

        IntakePipe(std::string name, nt::NetworkTableInstance& ntinst);
    
        void Process(cv::Mat& input) override;

        void checkForFrameEmpty(cv::Mat frame);

        cv::Vec3f fetchBiggestCircle(std::vector <cv::Vec3f> circles);
        
    };
}

#endif