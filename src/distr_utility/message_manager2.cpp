//
//  message_manager2.cpp
//  async_pso
//
//  Created by Christian Howard on 6/29/19.
//  Copyright © 2019 Christian Howard. All rights reserved.
//

#include "message_manager2.hpp"


namespace distributed {
    
    msg_manager2::msg_manager2():num_complete(0),tag(101),manager_id(0){
        comm = MPI_COMM_WORLD;
        MPI_Comm_rank(comm, &local_rank);
    }
    // method to set the ID for this manager
    void msg_manager2::set_id(size_t ID) {
        manager_id = ID;
    }
    size_t msg_manager2::get_id() const {
        return manager_id;
    }
    
    // methods to work with the messages
    util::raw_handle<message> msg_manager2::get_message_at(size_t message_id) {
        return messages[message_id];
    }
    int msg_manager2::get_message_type_at(size_t message_id) const {
        return messages[message_id]->get_type();
    }
    
    bool msg_manager2::is_message_complete(size_t message_id) const {
        return messages[message_id]->did_get_response();
    }
    
    size_t msg_manager2::create_message() {
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
    
    typename msg_manager2::uniq_msg_t msg_manager2::create_indep_message() {
        
        uniq_msg_t msg_;
        msg_.create();
        msg_->set_communicator(comm)
        .set_msg_tag(tag)
        .set_message_id(0)
        .set_process_rank();
        
        return msg_;
    }
    
    void msg_manager2::add_msg_to_response_queue(uniq_msg_t msg) {
        response_q.push(msg);
    }
    
    size_t msg_manager2::num_messages() const {
        return messages.size();
    }
    bool msg_manager2::all_messages_complete() const {
        return num_complete == messages.size();
    }
    void msg_manager2::clear_messages() {
        for(size_t i = 0; i < messages.size(); ++i){
            messages[i].free();
        }
        messages.resize(0);
        num_complete = 0;
    }
    
    void msg_manager2::check_responses_complete() {
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
    }
    void msg_manager2::probe_for_responses(int num2process) {
        for(int i = 0; i < num2process; ++i){
            auto probe_ = perform_nonblock_probe();
            
            if( probe_.flag && probe_.error_code == MPI_SUCCESS ){
                
                // variable representing buffer size
                int incoming_data_size = 0;
                
                // get data size using the probe status
                MPI_Get_count(&probe_.status,
                              MPI_BYTE,
                              &incoming_data_size);
                
                // create an async recv instance
                uniq_arecv_t arecv_;
                arecv_.create();
                arecv_->buf.resize(incoming_data_size);
                arecv_->src_rank = probe_.status.MPI_SOURCE;
                
                // do a non-blocking receive
                int err = MPI_Irecv(arecv_->buf.data(),
                                    incoming_data_size,
                                    MPI_BYTE,
                                    probe_.status.MPI_SOURCE,
                                    tag,
                                    comm,
                                    &arecv_->req);
                
                // add this async recv to the queue
                recv_q.push(arecv_);
                
            }else{
                if( !probe_.flag ){ break; }
            }
        }// end for i
    }
    void msg_manager2::check_get_async_responses() {
        size_t size_ = recv_q.size();
        for(size_t i = 0; i < size_; ++i){
            auto arecv = recv_q.front(); recv_q.pop();
            int flag = 0;
            MPI_Test(&arecv->req, &flag, MPI_STATUS_IGNORE);
            if( flag ){
                process_recv(*arecv);
                arecv.free();
            }else{ recv_q.push(arecv); }
        }
    }
        
    void msg_manager2::process_recv(async_recv& arecv) {
        
        // now parse the message for important data
        metadata_t metadata;
        byte_t* buf = arecv.buf.data();
        size_t offset = util::deserialize(metadata, buf);
        size_t msg_gut_size = arecv.buf.size() - offset;
        
        //if this is a response message, handle the response
        int src_rank = arecv.src_rank;
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
        
    }
    
    // methods for checking progress
    void msg_manager2::check_message_completeness(int num2process) {
        
        check_responses_complete();
        probe_for_responses(num2process);
        check_get_async_responses();
    }
    
    void msg_manager2::increment_number_complete_msgs() {
        ++num_complete;
    }
    
    // set/get the tag for this class
    void msg_manager2::set_manager_tag(int tag_) {
        tag = tag_;
    }
    int msg_manager2::get_manager_tag() const {
        return tag;
    }
    
    // set communicator
    void msg_manager2::set_mpi_comm(MPI_Comm com) {
        comm = com;
        MPI_Comm_rank(comm, &local_rank);
    }
    
    typename msg_manager2::probe_t msg_manager2::perform_nonblock_probe() const {
        
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
