#ifndef TORQUE_PIPELINE_H_
#define TORQUE_PIPELINE_H_

#include <string>

#include "networktables/NetworkTableInstance.h"
#include "vision/VisionPipeline.h"

class Pipeline : public frc::VisionPipeline {
  public:
    // Define class scope variables
    nt::NetworkTableInstance ntinst;
    int count = 1;

    // Setup function definition (constructor)
    Pipeline(nt::NetworkTableInstance ntinst);

    // Update function definition
    void Process(cv::Mat& mat) override;
};

#endif