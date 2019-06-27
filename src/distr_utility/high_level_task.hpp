//
//  high_level_task.hpp
//  distributed_vec
//
//  Created by Christian Howard on 6/15/19.
//  Copyright © 2019. All rights reserved.
//

#ifndef high_level_task_hpp
#define high_level_task_hpp

#include <functional>
#include <vector>

namespace distributed {
        
    template<typename other_data_t>
    class high_level_task {
    public:
        
        // specify useful type aliases
        using task_id       = size_t;
        
        // specify special states
        static constexpr int Null = 0;
        static constexpr int Complete = -1;
        
        // define high level task class
        class high_level_subtask {
        public:
            
            // ctor/dtor
            high_level_subtask();
            ~high_level_subtask() = default;
            
            // given methods for high level task
            void add_task(task_id tID, other_data_t data = other_data_t());
            size_t num_tasks() const;
            size_t num_complete_tasks() const;
            task_id get_task_id(size_t idx) const;
            const other_data_t* get_other_data(size_t idx) const;
            void reset();
            
            // callback methods for managing when
            // the task actually completes
            bool is_complete();
            void set_completion_callback( std::function<bool(task_id, other_data_t*, void*)> cb );
            void set_callback_data(void* ref);
            
        private:
            friend class high_level_task;
            
            // method to check for completion of the tasks
            void check_for_complete_tasks();
            
            // struct to store a task and container ID tuple
            struct task_and_other {
                task_id         tID;
                other_data_t    data;
                bool            complete;
            };
            
            // internal state
            size_t                      num_complete;
            void*                       callback_data;
            std::vector<task_and_other> task_tuples;
            
            // this callback is meant to check for whether an input
            // low level task id, along with some other data associated
            // with it and the subtask itself, has completed.
            std::function<bool(task_id, other_data_t*, void*)> c_callback;
        };
        
        // ctor/dtor
        high_level_task();
        virtual ~high_level_task() = default;
        
        // methods for handling high level tasks
        high_level_subtask& get_subtask();
        const high_level_subtask& get_subtask() const;
        void set_state(int s);
        int  get_state() const;
        void check_for_completeness();
        bool is_complete() const;
        
    private:
        
        // state variable for the high level task
        int state;
        
        // define a single subtask for now
        // that will encapsulate many low level tasks
        // and be used to transition to new subtasks,
        // if necessary
        high_level_subtask subtask;
        
        // method for defining the transition
        virtual void transition() = 0;
        
    };
}// end distributed

#include "high_level_task.hxx"

#endif /* high_level_task_hpp */
