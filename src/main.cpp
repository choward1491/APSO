//
//  main.cpp
//  async_pso
//
//  Created by Christian Howard on 6/26/19.
//  Copyright © 2019 Christian Howard. All rights reserved.
//

#include <mpi.h>
#include <iostream>
#include <chrono>
#include <thread>
#include "async_pso/swarm.hpp"
#include "sync_pso/sync_swarm.hpp"
#include "objectives/test_problems.hpp"


int main(int argc, const char * argv[]) {
    
    // initialize the MPI stuff
    MPI_Init(nullptr, nullptr);
    
    // get the local rank
    int local_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &local_rank);
    
    // get the total ranks
    int tot_ranks;
    MPI_Comm_size(MPI_COMM_WORLD, &tot_ranks);
    
    /*
    double val = 0.0;
    double value = 3.1415, value2 = 0.0;
    int N = 50;
    MPI_Request reqs[50];
    int flags[50];
    bool complete = false;
    
    double t1 = MPI_Wtime();
    if( local_rank == 0 ){
        for(int i = 0; i < 10000000; ++i){
            if( i < N ){
                MPI_Isend(&value, 1, MPI_DOUBLE, 1, 0, MPI_COMM_WORLD, reqs + i);
            }else{
                MPI_Test(reqs + (i % 50), flags + (i % 50), MPI_STATUS_IGNORE);
            }
            val += i * 2.9;
        }
    }
    
    if( local_rank == 1 ){
        
        for(int i = 0; i < 10000000; ++i){
            if( i < N ){
                MPI_Irecv(&value2, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, reqs + i);
            }else{
                MPI_Test(reqs + (i % 50), flags + (i % 50), MPI_STATUS_IGNORE);
            }
            val += i * 2.9;
        }
    }
    
    MPI_Waitall(N, reqs, MPI_STATUSES_IGNORE);
    double t2 = MPI_Wtime();
    printf("Rank(%i): Runtime for chunk 1 is %lf\n", local_rank, t2 - t1);
    
    double t3 = MPI_Wtime();
    if( local_rank == 0 ){
        for(int i = 0; i < N; ++i){
            MPI_Send(&value, 1, MPI_DOUBLE, 1, 0, MPI_COMM_WORLD);
        }
    }
    
    if( local_rank == 1 ){
        for(int i = 0; i < N; ++i){
            MPI_Recv(&value2, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
    }
    
    double val2 = 0.0;
    for(int i = 0; i < 10000000; ++i){
        val2 += i * 2.9;
    }
    double t4 = MPI_Wtime();
    printf("Rank(%i): Runtime for chunk 2 is %lf\n", local_rank, t4 - t3);
    */
    
    
    // specify the lower and upper bounds
    std::vector<double> lb{-10, -10}, ub{10, 10};
    
    // setup the swarm
    //int num_particles = 4800, num_iterations = 4000000;
    int num_particles = 48*3, num_iterations = 100;
    
    {   // start test async swarm
        async::pso::swarm<test::levyfunc_n13> swarm_async( num_particles / tot_ranks );
        
        // test the async first
        test::gen.seed(17);
        swarm_async.set_bounds(lb, ub);
        swarm_async.set_mpi_comm(MPI_COMM_WORLD);
        swarm_async.set_msg_check_frequency(1);
        swarm_async.set_print_flag(false);
        swarm_async.initialize();
        
        if( local_rank == 0 ){
            printf("Rank(%i): Starting the run\n", local_rank);
        }
        double t1 = MPI_Wtime();
        // do particle swarm iterations
        for(int i = 0; i < num_iterations; ++i){
            swarm_async.iterate();
        }
        double t2 = MPI_Wtime();
        
        // print the result
        if( local_rank == 0 ){
            printf("Rank(%i): Runtime is %0.5es\n", local_rank, t2 - t1);
            printf("Rank(%i): fval^* = %0.5e\n", local_rank, swarm_async.get_best_objective_value());
            printf("Rank(%i): x^*    = [ ", local_rank);
            auto& x = swarm_async.get_best_position();
            for(auto xv: x){
                printf("%0.5e ", xv);
            }
            printf("]\n");
        }
    }   // end test async swarm
    
    {   // test the sync swarm
        sync::pso::swarm<test::levyfunc_n13> swarm_sync( num_particles / tot_ranks );
        test::gen.seed(17);
        swarm_sync.set_bounds(lb, ub);
        swarm_sync.set_mpi_comm(MPI_COMM_WORLD);
        swarm_sync.set_msg_check_frequency(1);
        swarm_sync.set_print_flag(false);
        swarm_sync.initialize();
        
        if( local_rank == 0 ){
            printf("Rank(%i): Starting the run\n", local_rank);
        }
        double t1 = MPI_Wtime();
        // do particle swarm iterations
        for(int i = 0; i < num_iterations; ++i){
            swarm_sync.iterate();
        }
        double t2 = MPI_Wtime();
        
        // print the result
        if( local_rank == 0 ){
            printf("Rank(%i): Runtime is %0.5es\n", local_rank, t2 - t1);
            printf("Rank(%i): fval^* = %0.5e\n", local_rank, swarm_sync.get_best_objective_value());
            printf("Rank(%i): x^*    = [ ", local_rank);
            auto& x = swarm_sync.get_best_position();
            for(auto xv: x){
                printf("%0.5e ", xv);
            }
            printf("]\n");
        }
    }   // end test sync swarm
    
    /*
    int dest_rank = (local_rank+1)%tot_ranks, val = 10, tmp = 0;
    MPI_Status stat;
    
#if 1
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
            msg1.free();
        }
        
        MPI_Iprobe(dest_rank, 0, MPI_COMM_WORLD, &flag, &stat);
        
        // check for a reply
        if( flag ){
            
            // get the response
            MPI_Recv(&tmp, 1, MPI_INT, stat.MPI_SOURCE,
                     stat.MPI_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            
            counter++;
            
            // create message and write a response
            msg1.create();
            msg = msg1.raw_ptr();
            msg->reset_buffer();
            msg->set_msg_tag(0)
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
