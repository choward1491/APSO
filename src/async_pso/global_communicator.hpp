//
//  global_communicator.hpp
//  async_pso
//
//  Created by Christian Howard on 6/26/19.
//  Copyright © 2019 Christian Howard. All rights reserved.
//

#ifndef global_communicator_hpp
#define global_communicator_hpp

#include <vector>
#include <random>
#include "../distr_utility/message_manager.hpp"

namespace async {
    namespace pso {
        
        /*
         Class for managing data messages from one
         swarm partition to another
         */
        class global_comm : public distributed::msg_manager {
        public:
            
            // ctor/dtor
            global_comm();
            ~global_comm() = default;
            
            // set the number of processors we will send messages to
            // without replacement
            void set_num_scatter(int k = 5);
            
            // set the communicator
            void set_mpi_comm(MPI_Comm comm);
            
            // set random generator
            void set_prng(std::mt19937& gen);
            
            // try to update the global best estimate
            // by passing in some function value and the
            // corresponding position found
            void update_global_best_est(double func_val, const std::vector<double>& position);
            
            // method to send a message with the
            // current global best estimate
            void send_global_best_est();
            
            // load the responses from the other swarms
            // and get the optimal function value
            // and best position
            void load_responses_update_estimate();
            
            // get the current best estimates
            double best_function_value() const;
            const std::vector<double>& best_position() const;
            
        private:
            
            // the number of processors to send the messages to
            int num_sample, local_rank, tot_rank;
            
            // specify the best function value
            // and the best position
            double best_fval;
            std::vector<double> best_pos;
            std::vector<int> samples;
            std::vector<int> perm_samples;
            
            // random sampler
            std::mt19937* eng;
            
            // message types
            enum msg_type: int {
                SendEstimate = 0,
                RespondToEstimate
            };
            
            // type aliases
            using byte_t = distributed::byte_t;
            using uniq_msg_handle = util::unique_handle<distributed::message>;
            using metadata_t = distributed::msg_manager::metadata_t;
            
            // overloaded response handler
            void response_handler(byte_t* buf, metadata_t metadata, int src_rank);
            
            // generate samples without replacement
            void get_samples();
            
        };
    }
} // end namespace async

#endif /* global_communicator_hpp */
