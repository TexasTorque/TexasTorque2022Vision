//
// Copyright (c) Texas Torque 2022
//
// Authors: Justus, Jacob, Omar, Jack
//

#include "Intake.hh"

namespace texastorque {
    time_t timeNow() {
        return std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    }   

    IntakePipe::IntakePipe(std::string name, nt::NetworkTableInstance& ntinst) {  
        std::shared_ptr<nt::NetworkTable> fmstable = ntinst.GetTable("FMSInfo");
        alliance = fmstable->GetEntry("IsRedAlliance");
        std::shared_ptr<nt::NetworkTable> table = ntinst.GetTable("ball_detection_" + name);
        cvSource = frc::CameraServer::GetInstance()->PutVideo(
                "ball_detection_display_" + name,
                320, 240);
        this->ballPosition = table->GetEntry("position");
        this->ballRadius = table->GetEntry("radius");
        lastTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        this->bounds = IntakeBound(fetchColorFromFMS());
    }

    Color IntakePipe::fetchColorFromFMS() {
        return alliance.GetBoolean(false) ? RED : BLUE;
    }

    void IntakePipe::Process(cv::Mat& input) {
        time_t time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        if (time - lastTime > 1) {
            lastTime = time;
            bounds = IntakeBound(fetchColorFromFMS());
        }

        frame = input;
        // these dont need to be prefixed with cv:: ?

        if (bounds.color == RED) 
            frame = ~frame; // invert color space to allow const

        // convert to HSV color space
        cvtColor(frame, frame, cv::COLOR_BGR2HSV);
        // remove non-colored range
        inRange(frame, bounds.lower, bounds.upper, frame);

        // blur together features
        medianBlur(frame, frame, 17);

        // erode extraneous data
        erode(frame, frame, cv::Mat(), cv::Point(-1, -1), 3);

        // dilate good data
        dilate(frame, frame, cv::Mat(), cv::Point(-1, -1), 5);

        // Remove small details
        morphologyEx(frame, frame, cv::MORPH_HITMISS, cv::Mat());

        // get circles using hough-circles
        std::vector <cv::Vec3f> circles;
        HoughCircles(frame, circles, cv::HOUGH_GRADIENT, 1, 25, 30, 15, 15, 0);

        // if there is a circle
        if (circles.size() > 0) {
            // get the largest one and send
            cv::Vec3f biggest = fetchBiggestCircle(circles);

            cv::Point center = cv::Point(biggest[0], biggest[1]);

            // draw circle center
            cv::circle(frame, center, 1, cv::Scalar(0, 100, 100), 3,
                        cv::LINE_AA);

            // set x-value (px) and radius (px)
            ballPosition.SetDouble(biggest[0]);
            ballRadius.SetDouble(biggest[2]);

            for (auto &&circle: circles) {
                cv::Point center = cv::Point(circle[0], circle[1]);
                int radius = circle[2];
                // draw circle
                cv::circle(input, center, radius, cv::Scalar(255, 0, 255), 3,
                            cv::LINE_AA); // draws the circles on the original frame
            }

        } else {
            // if none are found, set default value (-1)
            ballPosition.SetDouble(0);
            ballRadius.SetDouble(0);
        }

        // This is a request from the drivers. It helps when the
        // Robot needs to do a complex operation when you can't
        // see it, like climbing, or intaking on the other side
        // of the hub. This shouldnt be too much of a performance
        // issue, and if it is we can always remove it.
        cvSource.PutFrame(input);
    }

    void IntakePipe::checkForFrameEmpty(cv::Mat frame) {
        if (frame.empty()) throw std::runtime_error("Frame is empty");
    }

    cv::Vec3f IntakePipe::fetchBiggestCircle(std::vector <cv::Vec3f> circles) {
        cv::Vec3f biggest;
        int bigrad = 0;
        for (size_t i = 0; i < circles.size(); i++) {
            if (circles[i][2] > bigrad) {
                bigrad = circles[i][2];
                biggest = circles[i];
            }
        }
        return biggest;
    }
}