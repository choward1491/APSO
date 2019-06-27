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
#include "compare_pso/swarm.h"
#include "compare_pso/cost_function.h"

struct quadratic {
    double operator()(const std::vector<double>& x) const {
        double val = 0.0;
        for(auto xx: x){ val += xx * xx; }
        return val;
    }
};

//function for distributing particles across the processors
void distribute_particles(const int num_particles, const int num_proc, const int proc_id,
                          int &particle_id_start, int &particles_per_proc)
{
    if (num_proc > num_particles) {
        // Don't worry about this special case. Assume the number of particles
        // is greater than the number of processors.
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    
    particle_id_start = num_particles / num_proc * proc_id;
    particles_per_proc = num_particles / num_proc;
    if (proc_id == num_proc - 1) {
        // Give remainder to last process
        particles_per_proc += num_particles % num_proc;
    }
}


int main(int argc, const char * argv[]) {
    
    // initialize the MPI stuff
    MPI_Init(nullptr, nullptr);
    
    // get the local rank
    int local_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &local_rank);
    
    int tot_ranks;
    MPI_Comm_size(MPI_COMM_WORLD, &tot_ranks);
    
    // specify the lower and upper bounds
    std::vector<double> lb{-1, -1}, ub{1, 1};
    
    // setup the swarm
    int max_iter = 200000;
    int num_particles = 200;
    async::pso::swarm<quadratic> swarm_(num_particles / tot_ranks);
    swarm_.set_bounds(lb, ub);
    swarm_.set_mpi_comm(MPI_COMM_WORLD);
    swarm_.set_tag(5);
    swarm_.set_msg_check_frequency(max_iter);
    swarm_.set_print_flag(false);
    swarm_.initialize();
    
    double t1 = MPI_Wtime();
    
    // do particle swarm iterations //2000000
    for(int i = 0; i < max_iter; ++i){
        swarm_.iterate();
    }
    
    double t2 = MPI_Wtime();
    
    // print the result
    printf("Rank(%i): Time Elapsed is %lf seconds\n", local_rank, t2 - t1);
    printf("Rank(%i): fval^* = %0.5e\n", local_rank, swarm_.get_best_objective_value());
    printf("Rank(%i): x^*    = [ ", local_rank);
    auto& x = swarm_.get_best_position();
    for(auto xv: x){
        printf("%0.5e ", xv);
    }
    printf("]\n");
    
    // run davids thing
    int num_dim = 2;
    auto cost_func_ptr = new x_sq();
    std::vector<double> lower_bnds = {-1, -1};
    std::vector<double> upper_bnds = {1, 1};
    bounds bnds(num_dim, lower_bnds,upper_bnds);
    
    int num_proc;  //number of processors
    int proc_id;   //processor id
    MPI_Comm_size(MPI_COMM_WORLD, &num_proc);
    MPI_Comm_rank(MPI_COMM_WORLD, &proc_id);
    
    int particle_id_start = 0;
    int particles_per_proc = 0;
    
    distribute_particles(num_particles, num_proc, proc_id, particle_id_start, particles_per_proc);
    
    swarm swarm_obj(particles_per_proc, num_dim, max_iter, cost_func_ptr, bnds, num_proc, proc_id);
    swarm_obj.init_swarm();
    
    printf("Rank(%i).v2: Start iterating\n", local_rank);
    t1 = MPI_Wtime();
    swarm_obj.iterate();
    t2 = MPI_Wtime();
    
    printf("Rank(%i).v2: Time Elapsed is %lf seconds\n", local_rank, t2 - t1);
    
    printf("Rank(%i): x^*    = [ ", local_rank);
    auto& x2 = swarm_obj.best_soln;
    for(auto xv: x2){
        printf("%0.5e ", xv);
    }
    printf("]\n");
    
    delete cost_func_ptr;
    cost_func_ptr = nullptr;
    
    // finalize
    MPI_Finalize();
    return 0;
}
