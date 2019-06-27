//
//  high_level_task.cpp
//  distributed_vec
//
//  Created by Christian Howard on 6/15/19.
//  Copyright © 2019. All rights reserved.
//


#ifndef high_level_task_hxx
#define high_level_task_hxx

#define HEADER template<typename other_data_t>
#define CLASS high_level_task<other_data_t>
#include "high_level_task.hpp"

namespace distributed {
        
    // define high level task class
    HEADER CLASS::high_level_subtask::high_level_subtask():num_complete(0), c_callback(nullptr),
    callback_data(nullptr)
    {
        
    }
    
    // methods for high level subtask
    HEADER void CLASS::high_level_subtask::add_task(task_id tID, other_data_t data) {
        task_and_other tc { tID, data, false };
        task_tuples.push_back(tc);
    }
    HEADER size_t CLASS::high_level_subtask::num_tasks() const {
        return task_tuples.size();
    }
    HEADER size_t CLASS::high_level_subtask::num_complete_tasks() const {
        return num_complete;
    }
    HEADER void CLASS::high_level_subtask::reset() {
        num_complete = 0;
        task_tuples.resize(0);
        c_callback = nullptr;
        callback_data = nullptr;
    }
    
    HEADER bool CLASS::high_level_subtask::is_complete() {
        return (num_complete == task_tuples.size());
    }
    HEADER void CLASS::high_level_subtask::set_completion_callback( std::function<bool(task_id, other_data_t*, void*)> cb ) {
        c_callback = cb;
    }
    HEADER void CLASS::high_level_subtask::set_callback_data(void* ref) {
        callback_data = ref;
    }
    
    HEADER typename CLASS::task_id CLASS::high_level_subtask::get_task_id(size_t idx) const {
        return task_tuples[idx].tID;
    }
    HEADER const other_data_t* CLASS::high_level_subtask::get_other_data(size_t idx) const {
        return &task_tuples[idx].data;
    }
    
    HEADER void CLASS::high_level_subtask::check_for_complete_tasks() {
        const size_t num_tasks = task_tuples.size();
        
        // loop through low level tasks and see if they have
        // completed, using the callback function specified
        // for this subtask
        for(size_t i = 0; i < num_tasks; ++i){
            task_and_other* to_ptr = &task_tuples[i];
            if( !to_ptr->complete ){
                to_ptr->complete = c_callback(to_ptr->tID, &to_ptr->data, callback_data);
                num_complete += to_ptr->complete;
            }
        }// end for i
    }
    
    HEADER CLASS::high_level_task():state(Complete){
        
    }
    
    HEADER void CLASS::set_state(int s) {
        state = s;
    }
    
    HEADER int  CLASS::get_state() const {
        return state;
    }
    
    HEADER typename CLASS::high_level_subtask& CLASS::get_subtask() {
        return subtask;
    }
    HEADER const typename CLASS::high_level_subtask& CLASS::get_subtask() const {
        return subtask;
    }
    HEADER void CLASS::check_for_completeness() {
        subtask.check_for_complete_tasks();
        
        // if the subtask is complete, transition
        // to the next state and do any necessary
        // work required in that transition process
        if( subtask.is_complete() ){
            transition();
        }
    }
    HEADER bool CLASS::is_complete() const {
        return (state == Complete);
    }
    
}// end namespace distributed

#undef HEADER
#undef CLASS

#endif /* high_level_task_hxx */
