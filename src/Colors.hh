//
// Copyright (c) Texas Torque 2022
//
// Authors: Justus, Jacob, Omar, Jack
//

#ifndef TEXASTORQUE_COLORS
#define TEXASTORQUE_COLORS

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
    enum Color {
        RED, BLUE
    };

    class MagazineBound {
    public:
        Color color;
        cv::Scalar upper, lower;

        MagazineBound(Color color) : color(color) {
            if (color == RED) {
                lower = cv::Scalar(90, 50, 70);
                upper = cv::Scalar(128, 255, 255);
            } else if (color == BLUE) {
                lower = cv::Scalar(90, 50, 70);
                upper = cv::Scalar(128, 255, 255);
            } else
                throw std::runtime_error("Unknown color");
        }
    };

    class IntakeBound {
    public:
        Color color;
        cv::Scalar upper, lower;

        IntakeBound(Color color) : color(color) {
            if (color == RED) {
                lower = cv::Scalar(43, 18, 234);
                upper = cv::Scalar(110, 172, 255);
            } else if (color == BLUE) {
                lower = cv::Scalar(90, 50, 70);
                upper = cv::Scalar(128, 255, 255);
            } else
                throw std::runtime_error("Unknown color");
        }
    };
}

#endif