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


    class JustusPipeline : public frc::VisionPipeline {
    public:
        nt::NetworkTableEntry ballColor;
        cs::CvSource cvSource;
        Bound bounds = Bound(RED);
        cv::Mat frame;

        JustusPipeline(nt::NetworkTableInstance& ntinst) {  

            std::shared_ptr<nt::NetworkTable> table = ntinst.GetTable("ball_mag");
            cvSource = frc::CameraServer::GetInstance()->PutVideo("ball_mag_display", 320, 240);
            this->ballColor = table->GetEntry("color");
            this->bounds = bounds;

        }

        bool processBound(cv::Mat frame, Bound bound) {

            if (bounds.color == RED)
                frame = ~frame; 

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

            int pixels = frame.rows * frame.cols;

            int white = 0;
            for (int i = 0; i < frame.rows; i++) {
                for (int j = 0; j < frame.cols; j++)
                    if (frame.at<cv::Vec3b>(i, j).val[0] == 255)
                        white++;

                if (white > pixels / 2)
                    return true;
            }
            return false;
         
        }


        void Process(cv::Mat& input) override {
            bool blue = processBound(input, Bound(BLUE));
            bool red = processBound(input, Bound(RED));

            if (red)
                ballColor.SetString("red");
            else if (blue)
                ballColor.SetString("blue");
            else
                ballColor.SetString("none");
        }

        void checkForFrameEmpty(cv::Mat frame) {
            if (frame.empty()) throw std::runtime_error("Frame is empty");
        }
    };
} 


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
    for (const auto &config: cameraConfigs)
        cameras.emplace_back(StartCamera(config));
    // start image processing on camera 0 if present
    if (cameras.size() < 1) return -1;

    JustusPipeline* pipe = new JustusPipeline(ntinst);
    std::thread([&] {
        frc::VisionRunner<JustusPipeline> runner(
                cameras[0], pipe,
                [&](JustusPipeline &pipeline) {});
        runner.RunForever();
    }).detach();

    for (;;) std::this_thread::sleep_for(std::chrono::seconds(10));
}