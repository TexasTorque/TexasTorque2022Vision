//
// Copyright (c) Texas Torque 2022
//
// Authors: Justus, Jacob, Omar, Jack
//


#include "RollingMedian.hh"

namespace texastorque {

    RollingMedian::RollingMedian(int window) {
        this->window = window;
    }

    RollingMedian::~RollingMedian() {
        values.clear();
    }

    double RollingMedian::calculate(double value) {
        if (values.size() >= window)
            values.pop_front();
        values.push_back(value);

        std::vector<double> vec = {values.begin(), values.end()};
        std::sort(vec.begin(), vec.end());
        return (vec.size() % 2 == 0) 
                ? (vec[vec.size() / 2] + vec[vec.size() / 2 - 1]) / 2. 
                : vec[vec.size() / 2];
        
    }
}