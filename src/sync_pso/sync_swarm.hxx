//
//  sync_swarm.hxx
//  async_pso
//
//  Created by Christian Howard on 6/28/19.
//  Copyright © 2019 Christian Howard. All rights reserved.
//

#ifndef sync_swarm_hxx
#define sync_swarm_hxx

#define HEADER template<typename func_type>
#define CLASS swarm<func_type>

#include <limits>
#include "sync_swarm.hpp"

namespace sync {
    namespace pso {
        
        
        //ctor/dtor
        HEADER CLASS::swarm(int num_particles):particles(num_particles), frequency(1),
        w(0.9),phi_l(0.7), phi_g(0.5), do_print(true)
        {
            comm = MPI_COMM_WORLD;
            MPI_Comm_rank(comm, &local_rank);
            MPI_Comm_size(comm, &tot_ranks);
            int seed_val = (local_rank*1749 << 4) ^ 17;
            gen.seed(seed_val);
            gbest_fval = std::numeric_limits<double>::max();
        }
        
        HEADER void CLASS::set_mpi_comm(MPI_Comm com) {
            MPI_Comm_rank(com, &local_rank);
            MPI_Comm_size(com, &tot_ranks);
            gen.seed(local_rank);
            comm = com;
        }
        
        HEADER void CLASS::set_print_flag(bool do_print_) {
            do_print = do_print_;
        }
        
        // set the bounds
        HEADER void CLASS::set_bounds(const std::vector<double>& lb_, const std::vector<double>& ub_){
            lb = lb_; ub = ub_;
            gbest_pos.resize(lb.size());
            send_buf.resize(lb.size()+1);
        }
        
        HEADER void CLASS::set_momentum(double omega) {
            w = omega;
        }
        HEADER void CLASS::set_particle_weights(double phi_local, double phi_global){
            phi_l = phi_local;
            phi_g = phi_global;
        }
        
        // set how often we try to send/receive messages
        HEADER void CLASS::set_msg_check_frequency(size_t freq){
            frequency = freq;
        }
        
        // initialize the swarm
        HEADER void CLASS::initialize() {
            counter = 0;
            size_t dim = lb.size();
            for(auto&p: particles){
                p.set_num_dims(dim);
                p.initialize( gen, lb, ub );
                p.set_momentum(w);
                p.set_particle_weights(phi_l, phi_g);
            }
            recv_buf.resize( (dim+1) * tot_ranks );
        }
        
        // perform an iteration
        HEADER void CLASS::iterate() {
            
            // compute the values of the particles
            for(auto& p: particles){
                double fval = objective_func(p.get_current_position());
                p.set_function_value(fval);
                
                // set values into the global estimate tracker
                if( fval < gbest_fval ){
                    auto& bsoln = p.get_best_position();
                    for(size_t i = 0; i < bsoln.size(); ++i){
                        gbest_pos[i] = bsoln[i];
                    }
                    gbest_fval = fval;
                }
            }
            
            // send out message and receive results, if necessary
            if( ++counter % frequency == 0 ){
                
                // get the number of data being used here
                int num_data = static_cast<int>(lb.size()) + 1;
                
                // fill the send buffer
                send_buf[0] = gbest_fval;
                for(int i = 0; i < gbest_pos.size(); ++i){
                    send_buf[i+1] = gbest_pos[i];
                }
                
                // synchronize
                MPI_Allgather(&send_buf[0], num_data, MPI_DOUBLE,
                              &recv_buf[0], num_data, MPI_DOUBLE,
                              comm);
                
                // update the optimal result
                int opt_index = 0;
                double bval = recv_buf[0];
                for(int i = 1; i < tot_ranks; ++i){
                    const double ival = recv_buf[i*num_data];
                    if( ival < bval ){
                        bval = ival;
                        opt_index = i;
                    }
                }// for i
                
                gbest_fval = bval;
                for(size_t i = 0; i < lb.size(); ++i){
                    gbest_pos[i] = recv_buf[(i+1) + opt_index*num_data];
                }
                
                // print message
                if( do_print ){
                    printf("Rank(%i): f_{best} = %0.5e @ [ ", local_rank, gbest_fval);
                    for(size_t i = 0; i < gbest_pos.size(); ++i){
                        printf("%0.3e ", gbest_pos[i]);
                    }
                    printf("]\n");
                }
            }
            
            // update the particles
            for(auto& p: particles){
                
                // update the particle with the current
                // global best estimate
                p.update(gbest_pos);
            }
        }
        
        // get the function reference
        HEADER func_type& CLASS::get_objective_func() {
            return objective_func;
        }
        
        HEADER double CLASS::get_best_objective_value() const {
            return gbest_fval;
        }
        HEADER const std::vector<double>& CLASS::get_best_position() const {
            return gbest_pos;
        }
        
    }// end namespace pso
}// end namespace async

#undef HEADER
#undef CLASS

#endif /* sync_swarm_h */
