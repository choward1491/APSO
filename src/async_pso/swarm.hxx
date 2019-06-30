//
//  swarm.hxx
//  async_pso
//
//  Created by Christian Howard on 6/26/19.
//  Copyright © 2019 Christian Howard. All rights reserved.
//

#ifndef swarm_hxx
#define swarm_hxx

#define HEADER template<typename func_type>
#define CLASS swarm<func_type>

#include "swarm.hpp"

namespace async {
    namespace pso {
        
            
            //ctor/dtor
        HEADER CLASS::swarm(int num_particles):particles(num_particles), frequency(1),
        w(0.9),phi_l(0.7), phi_g(0.5), do_print(true)
        {
            comm = MPI_COMM_WORLD;
            MPI_Comm_rank(comm, &local_rank);
            int seed_val = (local_rank*1749 << 4) ^ 17;
            gen.seed(seed_val);
            set_tag(0);
            gcom.set_prng(gen);
        }
        
        HEADER void CLASS::set_mpi_comm(MPI_Comm com) {
            MPI_Comm_rank(com, &local_rank);
            gen.seed(local_rank);
            gcom.set_mpi_comm(com);
            comm = com;
        }
        
        HEADER void CLASS::set_tag(int tag) {
            gcom.set_manager_tag(tag);
        }
        
        HEADER void CLASS::set_print_flag(bool do_print_) {
            do_print = do_print_;
        }
        
        // set the bounds
        HEADER void CLASS::set_bounds(const std::vector<double>& lb_, const std::vector<double>& ub_){
            lb = lb_; ub = ub_;
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
        }
        
        // perform an iteration
        HEADER void CLASS::iterate() {
            
            // compute the values of the particles
            for(auto& p: particles){
                double fval = objective_func(p.get_current_position());
                p.set_function_value(fval);
                
                // set values into the global estimate tracker
                gcom.update_global_best_est(fval, p.get_current_position());
            }
            
            // update the particles
            const std::vector<double>& global_best = gcom.best_position();
            for(auto& p: particles){
                
                // update the particle with the current
                // global best estimate
                p.update(global_best);
            }
            
            // send out message and receive results, if necessary
            if( ++counter % frequency == 0 ){
                
                // check for completeness
                gcom.check_message_completeness(16);
                if( gcom.num_messages() ){
                    if( gcom.all_messages_complete() ){
                        gcom.load_responses_update_estimate();
                    }
                }else{
                    gcom.send_global_best_est();
                }
                
                
                // print message
                if( do_print ){
                    printf("Rank(%i): f_{best} = %0.5e @ [ ", local_rank, gcom.best_function_value());
                    for(size_t i = 0; i < global_best.size(); ++i){
                        printf("%0.3e ", global_best[i]);
                    }
                    printf("]\n");
                }
            }
        }
        
        // get the function reference
        HEADER func_type& CLASS::get_objective_func() {
            return objective_func;
        }
        
        // get the global communicator
        HEADER global_comm& CLASS::get_communicator() {
            return gcom;
        }
        
        HEADER double CLASS::get_best_objective_value() const {
            return gcom.best_function_value();
        }
        HEADER const std::vector<double>& CLASS::get_best_position() const {
            return gcom.best_position();
        }
        
    }// end namespace pso
}// end namespace async

#undef HEADER
#undef CLASS

#endif /* pso_hxx */
