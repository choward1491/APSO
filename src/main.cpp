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
#include "sync_pso/sync_swarm.hpp"

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
    
    // get the local rank
    int local_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &local_rank);
    
    // get the total ranks
    int tot_ranks;
    MPI_Comm_size(MPI_COMM_WORLD, &tot_ranks);
    
    // specify the lower and upper bounds
    std::vector<double> lb{-1, -1}, ub{1, 1};
    
    // setup the swarm
    int num_particles = 10*48, num_iterations = 400000;
    //async::pso::swarm<quadratic> swarm_( num_particles / tot_ranks );
    sync::pso::swarm<quadratic> swarm_( num_particles / tot_ranks );
    swarm_.set_bounds(lb, ub);
    swarm_.set_mpi_comm(MPI_COMM_WORLD);
    swarm_.set_msg_check_frequency(1);
    swarm_.set_print_flag(false);
    swarm_.initialize();

    char filename[256] = {'\0'};
    sprintf(filename, "dummy_output%i.txt", local_rank);
    
    FILE* file_ = fopen(filename, "w");
    if( file_ ){
	fprintf(file_, "hi there\n");
	fclose(file_);
    }

    if( local_rank == 0 ){
        printf("Rank(%i): Starting the run\n", local_rank);
    }
    double t1 = MPI_Wtime();
    // do particle swarm iterations
    for(int i = 0; i < num_iterations; ++i){
        swarm_.iterate();
    }
    double t2 = MPI_Wtime();
    
    // print the result
    if( local_rank == 0 ){
        printf("Rank(%i): Runtime is %0.5es\n", local_rank, t2 - t1);
        printf("Rank(%i): fval^* = %0.5e\n", local_rank, swarm_.get_best_objective_value());
        printf("Rank(%i): x^*    = [ ", local_rank);
        auto& x = swarm_.get_best_position();
        for(auto xv: x){
            printf("%0.5e ", xv);
        }
        printf("]\n");
    }
    
    // finalize
    MPI_Finalize();
    return 0;
}
