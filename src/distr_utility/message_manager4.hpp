//
//  message_manager4.hpp
//  async_pso
//
//  Created by Christian Howard on 7/1/19.
//  Copyright © 2019 Christian Howard. All rights reserved.
//

#ifndef message_manager4_hpp
#define message_manager4_hpp

#include <vector>
#include <queue>
#include "distr_message.hpp"
#include "unique_handle.hpp"
#include "raw_handle.hpp"
#include "pool_vector.hpp"

namespace distributed {
    
    class msg_manager4 {
    public:
        
        // define useful metadata type
        struct metadata_t {
            bool   is_response;
            int    msg_type;
            size_t mngr_id, msg_id;
        };
        
        // ctor/dtor
        msg_manager4();
        virtual ~msg_manager4();
        
        // method to set the ID for this manager
        void set_id(size_t ID);
        size_t get_id() const;
        
        // set/get the tag for this class
        void set_manager_tag(int tag);
        int get_manager_tag() const;
        
        // set communicator
        void set_mpi_comm(MPI_Comm com);
        
        // methods to work with the messages
        util::raw_handle<message> get_message_at(size_t message_id);
        int get_message_type_at(size_t message_id) const;
        bool is_message_complete(size_t message_id) const;
        size_t create_message();
        size_t num_messages() const;
        bool all_messages_complete() const;
        void clear_messages();
        
        // methods for checking progress
        void check_message_completeness(int num2process = 128);
        
    protected:
        
        // define the probe struct
        struct probe_t {
            int error_code;
            MPI_Status status;
            bool flag;
        };
        
        struct async_recv {
            size_t pool_id;
            std::vector<byte_t> buf;
            MPI_Request req;
            int src_rank;
        };
        using uniq_arecv_t = util::unique_handle<async_recv>;
        using raw_arecv_t = util::raw_handle<async_recv>;
        using uniq_msg_t = util::unique_handle<message>;
        using raw_msg_t  = util::raw_handle<message>;
        
        // main protected variables
        int tag, local_rank;
        size_t manager_id, num_complete;
        MPI_Comm comm;
        std::vector<raw_msg_t> messages;
        
        // protected methods
        void increment_number_complete_msgs();
        raw_msg_t create_indep_message();
        void add_msg_to_response_queue(raw_msg_t msg);
        
    private:
        
        // queue to store response messages
        util::pool_vector<async_recv>   recv_pool;
        util::pool_vector<message>      msg_pool;
        std::queue<raw_msg_t>           response_q;
        std::queue<raw_arecv_t>         recv_q;
        
        // perform nonblocking probe
        probe_t perform_nonblock_probe() const;
        void check_responses_complete();
        void probe_for_responses(int num2process);
        void check_get_async_responses();
        void process_recv(async_recv& arecv);
        
        // define virtual method for handling responses
        virtual void response_handler(byte_t* buf, metadata_t metadata, int src_rank) = 0;
        
    };
    
}

#endif /* message_manager4_hpp */
