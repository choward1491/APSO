//
//  message_manager4.cpp
//  async_pso
//
//  Created by Christian Howard on 7/1/19.
//  Copyright © 2019 Christian Howard. All rights reserved.
//

#include "message_manager4.hpp"

namespace distributed {
    
    msg_manager4::msg_manager4():num_complete(0),tag(101),manager_id(0){
        comm = MPI_COMM_WORLD;
        MPI_Comm_rank(comm, &local_rank);
        temp_buffer.resize(1028);
        messages.reserve(128);
        mpi_buf.resize(1028*10);
        MPI_Buffer_attach(mpi_buf.data(), static_cast<int>(mpi_buf.size()));
    }
    msg_manager4::~msg_manager4(){
    }
    // method to set the ID for this manager
    void msg_manager4::set_id(size_t ID) {
        manager_id = ID;
    }
    size_t msg_manager4::get_id() const {
        return manager_id;
    }
    
    // methods to work with the messages
    util::raw_handle<message> msg_manager4::get_message_at(size_t message_id) {
        return messages[message_id];
    }
    int msg_manager4::get_message_type_at(size_t message_id) const {
        return messages[message_id]->get_type();
    }
    
    bool msg_manager4::is_message_complete(size_t message_id) const {
        return messages[message_id]->did_get_response();
    }
    
    size_t msg_manager4::create_message() {
        size_t size_ = messages.size();
        size_t id = msg_pool.get_free_id();
        raw_msg_t msg_ = msg_pool[id];
        
        // initialize the message with the known details
        msg_->set_communicator(comm)
        .set_msg_tag(tag)
        .set_message_id(size_)
        .set_pool_id(id)
        .set_process_rank();
        messages.push_back(msg_);
        
        return size_;
    }
    
    typename msg_manager4::raw_msg_t msg_manager4::create_indep_message() {
        
        size_t id = msg_pool.get_free_id();
        raw_msg_t msg_ = msg_pool[id];
        
        msg_->set_communicator(comm)
        .set_msg_tag(tag)
        .set_message_id(0)
        .set_pool_id(id)
        .set_process_rank();
        
        return msg_;
    }
    
    void msg_manager4::add_msg_to_response_queue(raw_msg_t msg) {
        response_q.push(msg);
    }
    
    size_t msg_manager4::num_messages() const {
        return messages.size();
    }
    bool msg_manager4::all_messages_complete() const {
        return num_complete == messages.size();
    }
    void msg_manager4::clear_messages() {
        for(size_t i = 0; i < messages.size(); ++i){
            auto msg = messages[i];
            msg->reset_buffer(message::Send);
            msg->reset_buffer(message::Receive);
            msg_pool.erase(msg->get_pool_id());
            messages[i] = nullptr;
        }
        messages.resize(0);
        num_complete = 0;
    }
    
    void msg_manager4::check_responses_complete() {
        int flag = 0;
        size_t qsize = response_q.size();
        for(size_t i = 0; i < qsize; ++i){
            raw_msg_t msg = response_q.front(); response_q.pop();
            
            // test to see if the request is complete
            MPI_Test(&msg->get_mpi_request(),
                     &flag,
                     MPI_STATUS_IGNORE);
            
            // if test shows the request is not complete,
            // then push the msg back onto the queue
            if( !flag ){ response_q.push(msg); }
            else{
                msg->reset_buffer();
                msg_pool.erase(msg->get_pool_id());
            }
        }// end loop over your response queue
    }
    void msg_manager4::probe_for_responses(int num2process) {
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
                size_t id = recv_pool.get_free_id();
                raw_arecv_t arecv_ = recv_pool[id];
                arecv_->pool_id = id;
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
    void msg_manager4::check_get_async_responses() {
        size_t size_ = recv_q.size();
        for(size_t i = 0; i < size_; ++i){
            auto arecv = recv_q.front(); recv_q.pop();
            int flag = 0;
            MPI_Test(&arecv->req, &flag, MPI_STATUS_IGNORE);
            if( flag ){
                process_recv(*arecv);
                recv_pool.erase(arecv->pool_id);
            }else{ recv_q.push(arecv); }
        }
    }
    
    void msg_manager4::process_recv(async_recv& arecv) {
        
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
    void msg_manager4::check_message_completeness(int num2process) {
        check_responses_complete();
        probe_for_responses(num2process);
        check_get_async_responses();
    }
    
    void msg_manager4::increment_number_complete_msgs() {
        ++num_complete;
    }
    
    // set/get the tag for this class
    void msg_manager4::set_manager_tag(int tag_) {
        tag = tag_;
    }
    int msg_manager4::get_manager_tag() const {
        return tag;
    }
    
    // set communicator
    void msg_manager4::set_mpi_comm(MPI_Comm com) {
        comm = com;
        MPI_Comm_rank(comm, &local_rank);
    }
    
    typename msg_manager4::probe_t msg_manager4::perform_nonblock_probe() const {
        
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
