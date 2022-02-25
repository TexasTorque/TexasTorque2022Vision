//
// Copyright (c) Texas Torque 2022
//
// Authors: Justus, Jacob, Omar, Jack
//

#ifndef TEXASTORQUE_ROLLINGMEDIAN
#define TEXASTORQUE_ROLLINGMEDIAN

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
  class RollingMedian {
    private:
        int window;
        std::deque<double> values; 

    public:
        RollingMedian(int window);

        ~RollingMedian();

        double calculate(double value);
    };
}

#endif