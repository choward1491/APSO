//
//  main.cpp
//  async_pso
//
//  Created by Christian Howard on 6/26/19.
//  Copyright © 2019 Christian Howard. All rights reserved.
//

#include <mpi.h>
#include <iostream>
#include "async_pso/swarm.hpp"

struct quadratic {
    double operator()(const std::vector<double>& x) const {
        double val = 0.0;
        for(auto xx: x){ val += xx * xx; }
        return val;
    }
};


int main(int argc, const char * argv[]) {
    
    // initialize the MPI stuff
    MPI_Init(nullptr, nullptr);
    
    // specify the lower and upper bounds
    std::vector<double> lb{-1, -1}, ub{1, 1};
    
    // setup the swarm
    async::pso::swarm<quadratic> swarm_(10);
    swarm_.set_bounds(lb, ub);
    swarm_.set_mpi_comm(MPI_COMM_WORLD);
    swarm_.set_tag(5);
    swarm_.set_msg_check_frequency(5);
    swarm_.initialize();
    
    // do particle swarm iterations
    for(int i = 0; i < 100; ++i){
        swarm_.iterate();
    }
    
    // finalize
    MPI_Finalize();
    return 0;
}
