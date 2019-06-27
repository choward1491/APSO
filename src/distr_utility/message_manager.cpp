//
//  message_manager.cpp
//  distributed_hybrid_meshing
//
//  Created by Christian Howard on 6/23/19.
//  Copyright © 2019. All rights reserved.
//

#include "message_manager.hpp"

namespace distributed {
    
    msg_manager::msg_manager():num_complete(0),tag(101),manager_id(0){
        comm = MPI_COMM_WORLD;
        MPI_Comm_rank(comm, &local_rank);
    }
        // method to set the ID for this manager
    void msg_manager::set_id(size_t ID) {
        manager_id = ID;
    }
    size_t msg_manager::get_id() const {
        return manager_id;
    }
        
    // methods to work with the messages
    util::raw_handle<message> msg_manager::get_message_at(size_t message_id) {
        return messages[message_id];
    }
    int msg_manager::get_message_type_at(size_t message_id) const {
        return messages[message_id]->get_type();
    }
    
    bool msg_manager::is_message_complete(size_t message_id) const {
        return messages[message_id]->did_get_response();
    }
    
    size_t msg_manager::create_message() {
        size_t size_ = messages.size();
        messages.emplace_back();
        messages[size_].free();
        messages[size_].create();
        
        // initialize the message with the known details
        util::raw_handle<message> msg_ = messages[size_];
        msg_->set_communicator(comm)
            .set_msg_tag(tag)
            .set_message_id(size_)
            .set_process_rank();
        
        return size_;
    }
    
    typename msg_manager::uniq_msg_t msg_manager::create_indep_message() {
        
        uniq_msg_t msg_;
        msg_.create();
        msg_->set_communicator(comm)
        .set_msg_tag(tag)
        .set_message_id(0)
        .set_process_rank();
        
        return msg_;
    }
    
    void msg_manager::add_msg_to_response_queue(uniq_msg_t msg) {
        response_q.push(msg);
    }
    
    size_t msg_manager::num_messages() const {
        return messages.size();
    }
    bool msg_manager::all_messages_complete() const {
        return num_complete == messages.size();
    }
    void msg_manager::clear_messages() {
        for(size_t i = 0; i < messages.size(); ++i){
            messages[i].free();
        }
        messages.resize(0);
        num_complete = 0;
    }
    
    // methods for checking progress
    void msg_manager::check_message_completeness(int num2process) {
        
        int flag = 0;
        size_t qsize = response_q.size();
        for(size_t i = 0; i < qsize; ++i){
            uniq_msg_t msg = response_q.front(); response_q.pop();
            
            // test to see if the request is complete
            MPI_Test(&msg->get_mpi_request(),
                     &flag,
                     MPI_STATUS_IGNORE);
            
            // if test shows the request is not complete,
            // then push the msg back onto the queue
            if( !flag ){ response_q.push(msg); }
            else{ msg.free(); }
        }// end loop over your response queue
        
        for(int i = 0; i < num2process; ++i){
            auto probe_ = perform_nonblock_probe();
            
            if( probe_.flag && probe_.error_code == MPI_SUCCESS ){
                
                // variable representing buffer size
                int incoming_data_size = 0;
                
                // get data size using the probe status
                MPI_Get_count(&probe_.status,
                              MPI_BYTE,
                              &incoming_data_size);
                
                // allocate the buffer, if needed
                temp_buffer.resize(incoming_data_size);
                
                // perform the receive call
                int err = MPI_Recv(&temp_buffer[0],
                                   incoming_data_size,
                                   MPI_BYTE,
                                   probe_.status.MPI_SOURCE,
                                   tag,
                                   comm,
                                   &probe_.status
                                   );
                
                // now parse the message for important data
                metadata_t metadata;
                byte_t* buf = &temp_buffer[0];
                size_t offset = util::deserialize(metadata, buf);
                size_t msg_gut_size = incoming_data_size - offset;
                
                //if this is a response message, handle the response
                int src_rank = probe_.status.MPI_SOURCE;
                if( !metadata.is_response ){ response_handler(buf + offset, metadata, src_rank); }
                
                // otherwise, extract the result and stuff into the appropriate
                // message within the structure
                else{
                    
                    // fill the message with the non-metadata portion of message
                    util::raw_handle<message> msg_ = messages[metadata.msg_id];
                    msg_->resize_receive_buffer(msg_gut_size);
                    std::memcpy(msg_->get_receive_buffer(),
                                buf + offset,
                                msg_gut_size);
                    
                    // free the message request; should be done at this point
                    MPI_Wait(&msg_->get_mpi_request(), MPI_STATUS_IGNORE);
                    
                    // increment the number complete
                    msg_->set_response(true);
                    increment_number_complete_msgs();
                }
                
            }else{
                if( !probe_.flag ){ break; }
            }
            
        }// end for i
    }
    
    void msg_manager::increment_number_complete_msgs() {
        ++num_complete;
    }
    
    // set/get the tag for this class
    void msg_manager::set_manager_tag(int tag_) {
        tag = tag_;
    }
    int msg_manager::get_manager_tag() const {
        return tag;
    }
    
    // set communicator
    void msg_manager::set_mpi_comm(MPI_Comm com) {
        comm = com;
        MPI_Comm_rank(comm, &local_rank);
    }
    
    typename msg_manager::probe_t msg_manager::perform_nonblock_probe() const {
        
        // init vars
        int flag;
        struct probe_t probe_;
        
        // perform the non-blocking probe
        probe_.error_code = MPI_Iprobe(MPI_ANY_SOURCE,
                                       tag,
                                       comm,
                                       &flag,
                                       &probe_.status);
        
        // construct the probe struct instance
        probe_.flag = (flag != 0);
        
        // return the probe instance
        return probe_;
    }
    
}
