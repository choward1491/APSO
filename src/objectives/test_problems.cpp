//
//  test_problems.cpp
//  async_pso
//
//  Created by Christian Howard on 7/1/19.
//  Copyright © 2019 Christian Howard. All rights reserved.
//

#include <random>
#include <cmath>
#include <chrono>
#include <thread>
#include "test_problems.hpp"

namespace test {
    
    std::uniform_int_distribution<int> U(1, 10000);
    
    double levyfunc_n13::operator()(const std::vector<double>& x) const {
        const double s1 = std::sin(3.0 * M_PI * x[0]);
        const double s2 = std::sin(3.0 * M_PI * x[1]);
        const double s3 = std::sin(2.0 * M_PI * x[1]);
        const double d1 = (x[0] - 1.0);
        const double d2 = (x[1] - 1.0);
        const double output = s1 * s1 + d1*d1*(1.0 + s2*s2) + d2*d2*(1.0 + s3*s3);
        std::chrono::milliseconds time(U(gen));
        std::this_thread::sleep_for(time);
        return output;
    }
    
    double quadratic::operator()(const std::vector<double>& x) const {
        double val = 0.0;
        for(auto xx: x){
            val += xx * xx;
        }
        std::chrono::milliseconds time(U(gen));
        std::this_thread::sleep_for(time);
        return val;
    }
    
    
}
