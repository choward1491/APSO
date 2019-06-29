//
//  sync_swarm.hpp
//  async_pso
//
//  Created by Christian Howard on 6/28/19.
//  Copyright © 2019 Christian Howard. All rights reserved.
//

#ifndef sync_swarm_hpp
#define sync_swarm_hpp

#include <random>
#include <vector>
#include "global_communicator.hpp"
#include "../particle/particle.hpp"

namespace sync {
    namespace pso {
        
        template<typename func_type>
        class swarm {
        public:
            
            //ctor/dtor
            swarm(int num_particles = 20);
            ~swarm() = default;
            
            // set the MPI communicator
            void set_print_flag(bool do_print);
            void set_mpi_comm(MPI_Comm com);
            
            // set the bounds
            void set_bounds(const std::vector<double>& lb, const std::vector<double>& ub);
            
            // set how often we try to send/receive messages
            void set_msg_check_frequency(size_t freq);
            void set_momentum(double omega);
            void set_particle_weights(double phi_local, double phi_global);
            
            // initialize the swarm
            void initialize();
            
            // perform an iteration
            void iterate();
            
            // get the function reference
            func_type& get_objective_func();
            
            // methods to retrieve the optimal objective
            // function and position for the swarm on this rank
            double get_best_objective_value() const;
            const std::vector<double>& get_best_position() const;
            
        private:
            
            // the frequency at which we send/receive messages
            bool do_print;
            size_t frequency, counter;
            double w, phi_l, phi_g;
            
            // particles of the swarm
            std::vector<::pso::particle>    particles;
            std::vector<double>             gbest_pos;
            double                          gbest_fval;
            
            // specify buffers
            std::vector<double> send_buf;
            std::vector<double> recv_buf;
            
            // bounds for the domain
            std::vector<double> lb, ub;
            
            // objective function
            func_type objective_func;
            
            // random number generator
            std::mt19937 gen;
            
            // MPI stuff
            int local_rank, tot_ranks;
            MPI_Comm comm;
            
            
        };
        
    }// end namespace pso
}// end namespace sync

#include "sync_swarm.hxx"

#endif /* sync_swarm_h */
