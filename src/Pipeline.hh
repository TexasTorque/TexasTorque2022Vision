/**
 * Copyright (c) Texas Torque 2022
 *
 * @author Justus Languell
 */

#ifndef TEXASTORQUE_PIPELINE
#define TEXASTORQUE_PIPELINE

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
    class Pipeline : public frc::VisionPipeline {
    public:
        cs::CvSource cvSource;

        Pipeline(std::string name, nt::NetworkTableInstance& ntinst);
    
        void Process(cv::Mat& input) override;
    };
}

#endif