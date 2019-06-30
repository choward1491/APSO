//
//  distr_message.hpp
//  distributed_hybrid_meshing
//
//  Created by Christian Howard on 6/23/19.
//  Copyright © 2019. All rights reserved.
//

#ifndef distr_message_hpp
#define distr_message_hpp

#include <vector>
#include <mpi.h>
#include "serialization.hpp"

namespace distributed {
    
    using byte_t = unsigned char;
    
    class message {
    public:
        
        // enum definitions
        enum buf_type: int { Send = 0, Receive};
        
        // ctor/dtor
        message();
        ~message() = default;
        
        // method to send the message
        int send();
        int get_send_error_code() const;
        
        // setter methods
        message& set_communicator(MPI_Comm com);
        message& set_process_rank();
        message& set_destination_rank(int dest_rank);
        message& set_message_id(size_t ID);
        message& set_pool_id(size_t ID);
        message& set_msg_type(int mtype);
        message& set_msg_tag(int tag);
        
        // getter methods
        int get_type() const;
        int get_dest_rank() const;
        size_t get_pool_id() const;
        
        // get mpi request
        MPI_Request& get_mpi_request();
        
        // get whether we have had a response
        // or not yet
        bool did_get_response() const;
        void set_response(bool got_response);
        
        // outgoing buffer stuff
        const byte_t* get_send_buffer() const;
        byte_t* get_send_buffer();
        size_t get_send_buffer_size() const;
        
        // incoming buffer stuff
        const byte_t* get_receive_buffer() const;
        void resize_receive_buffer(size_t nsize);
        byte_t* get_receive_buffer();
        size_t get_receive_buffer_size() const;
        
        // method to fill up buffers
        void reset_buffer(int buf_type = Send);
        
        template<typename T> void add_data(const T& out_data, int buf_type = Send) {
            std::vector<byte_t>* bufs[2] = { &send_data, &response_data };
            size_t* buf_sizes[2] = { &send_data_size, &response_size };
            auto& buf = bufs[buf_type];
            auto* size_= buf_sizes[buf_type];
            
            // get size of byte content for data we want to add
            size_t data_byte_content = util::byte_content(out_data);
            
            // if the buffer cannot contain the new data, resize
            buf->resize( *size_ + data_byte_content );
            
            // stuff the buffer with the necessary data
            byte_t* buf_ = &(*buf)[0];
            *size_ = util::serialize(out_data, buf_, *size_);
        }
        
    private:
        
        // internal state
        int                 tag, my_rank, dest_rank, error_code;
        size_t              task_id, ID, pool_id;
        int                 message_type;
        std::vector<byte_t> send_data;
        size_t              send_data_size;
        std::vector<byte_t> response_data;
        size_t              response_size;
        MPI_Request         req;
        bool                did_get_response_;
        MPI_Comm            comm;
    };
}

#endif /* distr_message_hpp */
