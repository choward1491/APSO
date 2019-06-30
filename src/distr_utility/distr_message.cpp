//
//  distr_message.cpp
//  distributed_hybrid_meshing
//
//  Created by Christian Howard on 6/23/19.
//  Copyright © 2019. All rights reserved.
//

#include "distr_message.hpp"

namespace distributed {
    
    // ctor/dtor
    message::message() {
        task_id = ID = tag = my_rank = dest_rank = -1;
        error_code = message_type = send_data_size = response_size = 0;
        did_get_response_ = false;
        comm = MPI_COMM_WORLD;
    }
        
        // method to send the message
    int message::send() {
        
        // get buffer data
        const byte_t* buf = get_send_buffer();
        int buf_size= static_cast<int>(get_send_buffer_size());
        
        // send the non-blocking message
        error_code = MPI_Isend(buf,
                               buf_size,
                               MPI_BYTE,
                               dest_rank,
                               tag,
                               comm,
                               &req);
        
        // return the error code
        return error_code;
    }
        
        // setter methods
    message& message::set_communicator(MPI_Comm com) {
        comm = com;
        return *this;
    }
    message& message::set_process_rank() {
        MPI_Comm_rank(comm, &my_rank);
        return *this;
    }
    message& message::set_destination_rank(int dest_rank_) {
        dest_rank = dest_rank_;
        return *this;
    }
    message& message::set_message_id(size_t ID_) {
        ID = ID_;
        return *this;
    }
    message& message::set_pool_id(size_t ID_) {
        pool_id = ID_;
        return *this;
    }
    message& message::set_msg_type(int mtype) {
        message_type = mtype;
        return *this;
    }
    message& message::set_msg_tag(int tag_) {
        tag = tag_;
        return *this;
    }
    
    int message::get_type() const {
        return message_type;
    }
    int message::get_dest_rank() const {
        return dest_rank;
    }
    size_t message::get_pool_id() const {
        return pool_id;
    }
    
    // error code stuff
    int message::get_send_error_code() const {
        return error_code;
    }
    
    // outgoing buffer stuff
    const byte_t* message::get_send_buffer() const {
        return &send_data[0];
    }
    byte_t* message::get_send_buffer() {
        return &send_data[0];
    }
    size_t message::get_send_buffer_size() const {
        return send_data_size;
    }
    
    // incoming buffer stuff
    const byte_t* message::get_receive_buffer() const {
        return &response_data[0];
    }
    byte_t* message::get_receive_buffer() {
        return &response_data[0];
    }
    size_t message::get_receive_buffer_size() const {
        return response_size;
    }
    void message::resize_receive_buffer(size_t nsize) {
        response_data.resize(nsize);
        response_size = nsize;
    }
    
    // method to fill up buffers
    void message::reset_buffer(int buf_type) {
        if( buf_type == Send ){
            send_data_size = 0;
        }else{
            response_size = 0;
        }
    }
    
    // get mpi request
    MPI_Request& message::get_mpi_request() {
        return req;
    }
    
    // get whether we have had a response
    // or not yet
    bool message::did_get_response() const {
        return did_get_response_;
    }
    void message::set_response(bool got_response) {
        did_get_response_ = got_response;
    }
}
