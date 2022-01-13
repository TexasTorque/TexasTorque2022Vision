#include "pipeline.h"

// Setup function implementation (constructor)
Pipeline::Pipeline(nt::NetworkTableInstance ntinst) {
    this->ntinst = ntinst;

    
}

// Update function implementation
void Pipeline::Process(cv::Mat& mat) {
   count++;

    ntinst.GetEntry("count").SetDouble(count);
}
