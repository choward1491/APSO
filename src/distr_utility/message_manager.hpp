//
//  message_manager.hpp
//  distributed_hybrid_meshing
//
//  Created by Christian Howard on 6/23/19.
//  Copyright © 2019. All rights reserved.
//

#ifndef message_manager_hpp
#define message_manager_hpp

#include <vector>
#include <queue>
#include "distr_message.hpp"
#include "unique_handle.hpp"
#include "raw_handle.hpp"

namespace distributed {
    
    class msg_manager {
    public:
        
        // define useful metadata type
        struct metadata_t {
            bool   is_response;
            int    msg_type;
            size_t mngr_id, msg_id;
        };
        
        // ctor/dtor
        msg_manager();
        virtual ~msg_manager() = default;
        
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
        using uniq_msg_t = util::unique_handle<message>;
        int tag, local_rank;
        size_t manager_id, num_complete;
        MPI_Comm comm;
        std::vector<uniq_msg_t> messages;
        std::vector<byte_t> temp_buffer;
        
        // define the probe struct
        struct probe_t {
            int error_code;
            MPI_Status status;
            bool flag;
        };
        
        void increment_number_complete_msgs();
        uniq_msg_t create_indep_message();
        void add_msg_to_response_queue(uniq_msg_t msg);
        
    private:
        
        // queue to store response messages
        std::queue<uniq_msg_t> response_q;
        
        // perform nonblocking probe
        probe_t perform_nonblock_probe() const;
        
        // define virtual method for handling responses
        virtual void response_handler(byte_t* buf, metadata_t metadata, int src_rank) = 0;
        
    };
    
}

#endif /* message_manager_hpp */
