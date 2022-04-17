/**
 * Copyright (c) Texas Torque 2022
 *
 * @author Justus Languell
 */

#include "Pipeline.hh"

namespace texastorque { 
    time_t timeNow() {
        return std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    }   

    Pipeline::Pipeline(std::string name, nt::NetworkTableInstance& ntinst) {  
        cvSource = frc::CameraServer::GetInstance()->PutVideo(name, 640, 480);
    }

    void Pipeline::Process(cv::Mat& input) {
        cv::Mat flipped;
        cv::flip(input, flipped, 0);
        cvSource.PutFrame(flipped);  
    }
}


