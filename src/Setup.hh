/**
 * Copyright (c) Texas Torque 2022
 *
 * @author Justus Languell
 */

// This needs to be split up between a .hh and a .cc
// and cleaned up

#ifndef TEXASTORQUE_SETUP
#define TEXASTORQUE_SETUP

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

namespace setup {
    static const char *configFile = "/boot/frc.json";

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
        wpi::outs() << "1\n";
        cs::UsbCamera camera{config.name, config.path};
        wpi::outs() << "2\n";
        camera.SetConfigJson(config.config);
        wpi::outs() << "3\n";
        camera.SetConnectionStrategy(cs::VideoSource::kConnectionKeepOpen);
        wpi::outs() << "4\n";

        return camera;
    }
} 

#endif