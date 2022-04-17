/**
 * Copyright (c) Texas Torque 2022
 *
 * @author Justus Languell
 */

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

#include "Pipeline.hh"
#include "Setup.hh"

cs::VideoSource* getCameraByName(std::vector<cs::VideoSource> cameras, const std::string& name) {
    for (auto& camera : cameras) if (camera.GetName() == name) return &camera;
    return nullptr;
}

int main(int argc, char *argv[]) {
    using namespace texastorque;
    using namespace setup;

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

    if (cameras.size() < 1) return -1;

    Pipeline* pipe = new Pipeline("Front", ntinst);
    std::thread([&] {
        frc::VisionRunner<Pipeline> runner(
                *getCameraByName(cameras, "Front"), pipe,
                [&](Pipeline &pipeline) {});
        runner.RunForever();
    }).detach();

    for (;;) std::this_thread::sleep_for(std::chrono::seconds(10));
}
