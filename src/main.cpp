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
    //int num_particles = 4800, num_iterations = 4000000;
    int num_particles = 48, num_iterations = 4000000 * 10;
    async::pso::swarm<quadratic> swarm_( num_particles / tot_ranks );
    //sync::pso::swarm<quadratic> swarm_( num_particles / tot_ranks );
    swarm_.set_bounds(lb, ub);
    swarm_.set_mpi_comm(MPI_COMM_WORLD);
    swarm_.set_msg_check_frequency(1);
    swarm_.set_print_flag(false);
    swarm_.initialize();
    
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
     
    
    /*
    int dest_rank = (local_rank+1)%tot_ranks, val = 10, tmp = 0;
    MPI_Status stat;
    
#if 0
    distributed::message msg;
    
    double t1 = MPI_Wtime();
    msg .set_msg_tag(0)
        .set_msg_type(0)
        .set_message_id(0)
        .set_communicator(MPI_COMM_WORLD)
        .set_process_rank()
        .set_destination_rank(dest_rank);
    msg.add_data(val);
    int counter = 0, max_count = 4000000 * 10;
    
    // send a message if you are the rank 0 person
    if( local_rank == 0 ){
        msg.send();
    }
    
    // loop until done
    int flag;
    while( counter < max_count ){
        
        if( msg.get_mpi_request() != MPI_REQUEST_NULL ){
            MPI_Test(&msg.get_mpi_request(), &flag, MPI_STATUS_IGNORE);
            if( flag ){
                msg.get_mpi_request() = MPI_REQUEST_NULL;
            }
        }
        
        MPI_Iprobe(dest_rank, 0, MPI_COMM_WORLD, &flag, &stat);
        
        // check for a reply
        if( flag ){
            
            // get the response
            MPI_Recv(&tmp, 1, MPI_INT, stat.MPI_SOURCE,
                     stat.MPI_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            
            counter++;
            
            msg.reset_buffer();
            msg.add_data(val);
            msg.send();
            
        }
        
    }
    
    double t2 = MPI_Wtime();
    
    if( local_rank == 0 ){ printf("Rank(%i): Runtime is %0.5es\n", local_rank, t2 - t1); }
    
#else
    util::unique_handle<distributed::message> msg1, msg2;
    msg1.create();
    distributed::message* msg = msg1.raw_ptr();
    
    double t1 = MPI_Wtime();
    msg->set_msg_tag(0)
    .set_msg_type(0)
    .set_message_id(0)
    .set_communicator(MPI_COMM_WORLD)
    .set_process_rank()
    .set_destination_rank(dest_rank);
    msg->add_data(val);
    int counter = 0, max_count = 4000000 * 10;
    
    // send a message if you are the rank 0 person
    if( local_rank == 0 ){
        msg1->send();
    }
    
    // loop until done
    int flag;
    while( counter < max_count ){
        
        if( msg->get_mpi_request() != MPI_REQUEST_NULL ){
            MPI_Test(&msg->get_mpi_request(), &flag, MPI_STATUS_IGNORE);
            //msg1.free();
        }
        
        MPI_Iprobe(dest_rank, 0, MPI_COMM_WORLD, &flag, &stat);
        
        // check for a reply
        if( flag ){
            
            // get the response
            MPI_Recv(&tmp, 1, MPI_INT, stat.MPI_SOURCE,
                     stat.MPI_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            
            counter++;
            
            // create message and write a response
            //msg1.create();
            msg->reset_buffer();
            msg1->set_msg_tag(0)
            .set_msg_type(0)
            .set_message_id(0)
            .set_communicator(MPI_COMM_WORLD)
            .set_process_rank()
            .set_destination_rank(dest_rank);
            msg->add_data(val);
            msg->send();
            
        }
        
    }
    
    double t2 = MPI_Wtime();
    
    if( local_rank == 0 ){ printf("Rank(%i): Runtime is %0.5es\n", local_rank, t2 - t1); }
    
#endif
    */
    // finalize
    MPI_Finalize();
    return 0;
}
