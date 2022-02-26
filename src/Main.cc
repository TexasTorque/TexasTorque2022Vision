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

#include "Colors.hh"
#include "Intake.hh"
#include "Magazine.hh"
#include "Setup.hh"


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
    // start image processing on camera 0 if present
    if (cameras.size() < 1) return -1;

    MagazinePipe* pipe = new MagazinePipe(ntinst);
    std::thread([&] {
        frc::VisionRunner<MagazinePipe> runner(
                cameras[0], pipe,
                [&](MagazinePipe &pipeline) {});
        runner.RunForever();
    }).detach();


    IntakePipe* pipe1 = new IntakePipe("left", ntinst);
    std::thread([&] {
        frc::VisionRunner<IntakePipe> runner(
                cameras[1], pipe1,
                [&](IntakePipe &pipeline) {});
        runner.RunForever();
    }).detach();

     IntakePipe* pipe2 = new IntakePipe("right", ntinst);
     std::thread([&] {
        frc::VisionRunner<IntakePipe> runner(
                cameras[2], pipe2,
                [&](IntakePipe &pipeline) {});
        runner.RunForever();
    }).detach();


    for (;;) std::this_thread::sleep_for(std::chrono::seconds(10));
}
