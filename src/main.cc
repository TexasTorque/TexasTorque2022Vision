//
// Copyright (c) Texas Torque 2022
//
// Authors: Justus, Jacob, Omar, Jack
//

#include <algorithm>
#include <chrono>
#include <ctime>
#include <iostream>
#include <stdexcept>

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

static const char *configFile = "/boot/frc.json";

namespace {
    unsigned int team;
    bool server = false;

    struct CameraConfig {
        std::string name;
        std::string path;
        wpi::json config;
        wpi::json streamConfig;
    };

    std::vector <CameraConfig> cameraConfigs;
    std::vector <cs::VideoSource> cameras;

    wpi::raw_ostream &ParseError() {
        return wpi::errs() << "config error in '" << configFile << "': ";
    }

    bool ReadCameraConfig(const wpi::json &config) {
        CameraConfig c;

        // name
        try {
            c.name = config.at("name").get<std::string>();
        } catch (const wpi::json::exception &e) {
            ParseError() << "could not read camera name: " << e.what() << '\n';
            return false;
        }

        // path
        try {
            c.path = config.at("path").get<std::string>();
        } catch (const wpi::json::exception &e) {
            ParseError() << "camera '" << c.name
                         << "': could not read path: " << e.what() << '\n';
            return false;
        }

        // stream properties
        if (config.count("stream") != 0) c.streamConfig = config.at("stream");

        c.config = config;

        cameraConfigs.emplace_back(std::move(c));
        return true;
    }

    bool ReadConfig() {
        // open config file
        std::error_code ec;
        wpi::raw_fd_istream is(configFile, ec);
        if (ec) {
            wpi::errs() << "could not open '" << configFile << "': " << ec.message()
                        << '\n';
            return false;
        }

        // parse file
        wpi::json j;
        try {
            j = wpi::json::parse(is);
        } catch (const wpi::json::parse_error &e) {
            ParseError() << "byte " << e.byte << ": " << e.what() << '\n';
            return false;
        }

        // top level must be an object
        if (!j.is_object()) {
            ParseError() << "must be JSON object\n";
            return false;
        }

        // team number
        try {
            team = j.at("team").get<unsigned int>();
        } catch (const wpi::json::exception &e) {
            ParseError() << "could not read team number: " << e.what() << '\n';
            return false;
        }
        // ntmode (optional)
        if (j.count("ntmode") != 0) {
            try {
                auto str = j.at("ntmode").get<std::string>();
                wpi::StringRef s(str);
                if (s.equals_lower("client")) {
                    server = false;
                } else if (s.equals_lower("server")) {
                    server = true;
                } else {
                    ParseError() << "could not understand ntmode value '" << str
                                 << "'\n";
                }
            } catch (const wpi::json::exception &e) {
                ParseError() << "could not read ntmode: " << e.what() << '\n';
            }
        }

        // cameras
        try {
            for (auto &&camera: j.at("cameras")) {
                if (!ReadCameraConfig(camera)) return false;
            }
        } catch (const wpi::json::exception &e) {
            ParseError() << "could not read cameras: " << e.what() << '\n';
            return false;
        }

        return true;
    }

    cs::UsbCamera StartCamera(const CameraConfig &config) {
        wpi::outs() << "Starting camera '" << config.name << "' on " << config.path
                    << '\n';
        auto inst = frc::CameraServer::GetInstance();
        cs::UsbCamera camera{config.name, config.path};
        camera.SetConfigJson(config.config);
        camera.SetConnectionStrategy(cs::VideoSource::kConnectionKeepOpen);

        return camera;
    }

    enum Color {
        RED, BLUE
    };

    class Bound {
    public:
        Color color;
        cv::Scalar upper, lower;

        Bound(Color color) : color(color) {
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

// Could probably be replaced with 
    Color readAlliancColor(nt::NetworkTableEntry *entry) {
        std::string color = entry->GetString("none");
        if (color == "none") throw std::runtime_error("Unknown color");
        if (color == "red") return RED;
        else if (color == "blue") return BLUE;
        else throw std::runtime_error("Unknown color");
        return RED; // never executed
    }

    class JacksPipeline : public frc::VisionPipeline {
    public:
        nt::NetworkTableEntry *ballPosition;
        nt::NetworkTableEntry *ballRadius;
        cs::CvSource cvSource =
                frc::CameraServer::GetInstance()->PutVideo("ball_detection_display", 320, 240);
        Bound bounds = Bound(RED);
        cv::Mat frame;

        JacksPipeline(nt::NetworkTableEntry *x, nt::NetworkTableEntry *r, Color alliance) {
            this->ballPosition = x;
            this->ballRadius = r;
            this->bounds = Bound(alliance);
        }

        void Process(cv::Mat &input) override {
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
                ballPosition->SetDouble(biggest[0]);
                ballRadius->SetDouble(biggest[2]);

                for (auto &&circle: circles) {
                    cv::Point center = cv::Point(circle[0], circle[1]);
                    int radius = circle[2];
                    // draw circle
                    cv::circle(input, center, radius, cv::Scalar(255, 0, 255), 3,
                               cv::LINE_AA); // draws the circles on the original frame
                }

            } else {
                // if none are found, set default value (-1)
                ballPosition->SetDouble(0);
                ballRadius->SetDouble(0);
            }

            // This is a request from the drivers. It helps when the
            // Robot needs to do a complex operation when you can't
            // see it, like climbing, or intaking on the other side
            // of the hub. This shouldnt be too much of a performance
            // issue, and if it is we can always remove it.
            cvSource.PutFrame(input);
        }

        void checkForFrameEmpty(cv::Mat frame) {
            if (frame.empty()) throw std::runtime_error("Frame is empty");
        }

        cv::Vec3f fetchBiggestCircle(std::vector <cv::Vec3f> circles) {
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
    };
} // namespace

// NetworkTable Spec:

// Table:       ball_detection
//   Entry:     alliance_color
//     Type:    string
//     Value:   "blue"
//     Value:   "red"
//     Default: "none"
//   Entry:     position
//     Type:    double
//     Value:   0 <= x <= 640
//     Default: 0
//   Entry: 	radius
//     Type:    double
//     Value: 	0 <= 640
//     Default: 0

int main(int argc, char *argv[]) {
    if (argc >= 2) configFile = argv[1];

    if (!ReadConfig()) return EXIT_FAILURE;

    auto ntinst = nt::NetworkTableInstance::GetDefault();
    if (server) {
        wpi::outs() << "Setting up NetworkTables server\n";
        ntinst.StartServer();
    } else {
        wpi::outs() << "Setting up NetworkTables client for team " << team
                    << '\n';
        ntinst.StartClientTeam(team);
        ntinst.StartDSClient();
    }

    std::shared_ptr <nt::NetworkTable> table = ntinst.GetTable("ball_detection");
    nt::NetworkTableEntry ballPosition = table->GetEntry("position");
    nt::NetworkTableEntry ballRadius = table->GetEntry("radius");
    nt::NetworkTableEntry alliance = table->GetEntry("alliance_color");
    Color color = readAlliancColor(&alliance);

    for (const auto &config: cameraConfigs)
        cameras.emplace_back(StartCamera(config));
    // start image processing on camera 0 if present
    if (cameras.size() >= 1) {
        std::thread([&] {
            frc::VisionRunner <JacksPipeline> runner(
                    cameras[0], new JacksPipeline(&ballPosition, &ballRadius, color),
                    [&](JacksPipeline &pipeline) {});
            runner.RunForever();
        }).detach();
    }

    for (;;) std::this_thread::sleep_for(std::chrono::seconds(10));
}