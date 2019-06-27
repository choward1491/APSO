//
//  global_communicator.cpp
//  async_pso
//
//  Created by Christian Howard on 6/26/19.
//  Copyright © 2019 Christian Howard. All rights reserved.
//

#include <limits>
#include "global_communicator.hpp"

namespace async {
    namespace pso {
            
        // ctor/dtor
        global_comm::global_comm():best_fval(std::numeric_limits<double>::max()),eng(nullptr) {
            set_num_scatter(5);
            set_mpi_comm(MPI_COMM_WORLD);
        }
        
        // set the number of processors we will send messages to
        // without replacement
        void global_comm::set_num_scatter(int k) {
            num_sample = k;
            
            // resize the samples list
            samples.resize(num_sample);
        }
        
        void global_comm::get_samples() {
            int n = static_cast<int>(perm_samples.size())-1;
            for(int i = 0; i < num_sample; ++i){
                std::uniform_int_distribution<int> U(0, n-i);
                int idx = U(*eng);
                samples[i] = perm_samples[idx];
                std::swap(perm_samples[idx], perm_samples[n - i]);
            }
        }
        
        void global_comm::set_mpi_comm(MPI_Comm com) {
            distributed::msg_manager::set_mpi_comm(com);
            
            // get local rank and number of processes
            MPI_Comm_rank(com, &local_rank);
            MPI_Comm_size(com, &tot_rank);
            
            // fill up the samples list
            perm_samples.resize(0);
            for(int i = 0; i < tot_rank; ++i){
                if( i != local_rank ){
                    perm_samples.push_back(i);
                }
            }// end for i
            
            // change the size of the sample,
            // if necessary
            if( num_sample > (tot_rank-1) ){
                set_num_scatter(tot_rank-1);
            }
        }
        
        void global_comm::set_prng(std::mt19937& gen) {
            eng = &gen;
        }
        
        void global_comm::update_global_best_est(double func_val, const std::vector<double>& position) {
            if( func_val < best_fval ){
                best_fval = func_val;
                best_pos.resize(position.size());
                for(size_t i = 0; i < position.size(); ++i){
                    best_pos[i] = position[i];
                }
            }
        }
        
        // method to send a message with the
        // current global best estimate
        void global_comm::send_global_best_est(){
            
            // get samples of indices to send messages to
            get_samples();
            
            // send messages for this
            for(int rank: samples){
                
                // get a message
                size_t mID = create_message();
                util::raw_handle<distributed::message> msg_ = messages[mID];
                
                // set it up to be send
                msg_->set_destination_rank(rank)
                .set_msg_type(SendEstimate);
                
                // setup metadata
                metadata_t mdata;
                mdata.is_response = false;
                mdata.mngr_id   = manager_id;
                mdata.msg_id    = mID;
                mdata.msg_type  = SendEstimate;
                
                // add metadata and main data
                msg_->add_data(mdata);
                msg_->add_data(best_fval);
                msg_->add_data(best_pos);
                
                // send the message
                msg_->send();
            }
        }
        
        void global_comm::load_responses_update_estimate() {
            double fval = 0;
            for(size_t i = 0; i < num_messages(); ++i){
                auto msg_ = get_message_at(i);
                size_t offset = util::deserialize(fval, msg_->get_receive_buffer());
                
                if( fval < best_fval ){
                    best_fval = fval;
                    offset = util::deserialize(best_pos, msg_->get_receive_buffer(), offset);
                }
            }// loop over messages
            clear_messages();
        }
        
        // get the current best estimates
        double global_comm::best_function_value() const {
            return best_fval;
        }
        const std::vector<double>& global_comm::best_position() const {
            return best_pos;
        }
        
        // overloaded response handler
        void global_comm::response_handler(byte_t* buf, metadata_t metadata, int src_rank) {
            
            // create a new message
            uniq_msg_t msg_ = create_indep_message();
            msg_->set_destination_rank(src_rank);
            
            // set metadata as a response
            metadata.is_response = true;
            
            // set the message metadata
            metadata.msg_type = RespondToEstimate;
            msg_->add_data(metadata);
            
            // extract data to see if we
            // should update the best estimate
            double fval = 0.0;
            size_t offset = util::deserialize(fval, buf);
            
            if( fval < best_fval ){
                best_fval = fval;
                offset = util::deserialize(best_pos, buf, offset);
            }
            
            // add the response data to the message
            msg_->add_data(best_fval);
            msg_->add_data(best_pos);
            
            // send the message
            msg_->send();
            
            // add the message to the response q
            add_msg_to_response_queue(msg_);
            
        }
            
    }
} // end namespace async
