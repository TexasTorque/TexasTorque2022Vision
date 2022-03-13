//
// Copyright (c) Texas Torque 2022
//
// Authors: Justus, Jacob, Omar, Jack
//

#include "Magazine.hh"

namespace texastorque {


    MagazinePipe::MagazinePipe(nt::NetworkTableInstance& ntinst) {  
        std::shared_ptr<nt::NetworkTable> table = ntinst.GetTable("ball_mag");
        cvSource = frc::CameraServer::GetInstance()->PutVideo("ball_mag_display", 160, 120);
        this->ballColor = table->GetEntry("color");
    }

    bool MagazinePipe::processBound(cv::Mat frame, bool isRed) {

        if (isRed) frame = ~frame; // invert color space to allow const

        cvtColor(frame, frame, cv::COLOR_BGR2HSV);

        inRange(frame, isRed ? lowerRed : lowerBlue, isRed ? upperRed : upperBlue, frame);

        // blur together features
        medianBlur(frame, frame, 17);

        // erode extraneous data
        erode(frame, frame, cv::Mat(), cv::Point(-1, -1), 3);

        // dilate good data
        dilate(frame, frame, cv::Mat(), cv::Point(-1, -1), 5);

        // Remove small details
        morphologyEx(frame, frame, cv::MORPH_HITMISS, cv::Mat());

        // if (bounds.color == BLUE) cvSource.PutFrame(frame);

        int pixels = frame.cols * frame.rows;

        int count = 0;
        for (int i = 0; i < frame.cols; i++) {
            for (int j = 0; j < frame.rows; j++)
                if (frame.at<cv::Vec3b>(i, j)[0] == 255) 
                    count++;
        }

        double rollingResult = isRed
                ? redRollingMedian.calculate(count * 1.) 
                : blueRollingMedian.calculate(count * 1.);

        double percentRoll = (rollingResult * 1.) / (pixels * 1.);
        return percentRoll > fullness;
    
        if (isRed) cvSource.PutFrame(frame);
    }

    void MagazinePipe::Process(cv::Mat& input) {
        checkForFrameEmpty(input);

        bool red = processBound(input.clone(), true);
        bool blue = processBound(input.clone(),  false);

        if (red) {
            // wpi::outs() << "RED\n";
            ballColor.SetString("red");
        } else 
        if (blue) {
            // wpi::outs() << "BLUE\n";
            ballColor.SetString("blue");
        } else {
            // wpi::outs() << "NONE\n";
            ballColor.SetString("none");
        }
    }

    void MagazinePipe::checkForFrameEmpty(cv::Mat frame) {
        if (frame.empty()) throw std::runtime_error("Frame is empty");
    }
}