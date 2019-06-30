//
//  pool_vector.hxx
//  meshing
//
//  Created by Christian Howard on 4/3/19.
//  Copyright © 2019 SDG. All rights reserved.
//

#ifndef pool_vector_hxx
#define pool_vector_hxx

#define HEADER template<typename T, template<typename S> class container>
#define CLASS pool_vector<T, container>
#include "pool_vector.hpp"

namespace util {
            
    HEADER CLASS::pool_vector(size_t init_allocation){
        _size = num_allocated = num_capacity = num_free_ids = 0;
        factor = 2;
        bias = 16;
        reserve(init_allocation);
    }
    HEADER bool CLASS::reserve(size_t size) {
        size_t old_capacity = num_capacity;
        num_capacity = size;
        
        // fill the data list with new items
        for(size_t i = old_capacity; i < num_capacity; ++i){
            availability.push_back(true);
            unique_handle<T> tmp;
            data.push_back(tmp);
        }
        
        // reserved memory fine
        // should write this to check if allocation succeeded,
        // but will worry about this later
        return true;
    }
    HEADER raw_handle<T> CLASS::operator[](size_t idx) {
        return raw_handle<T>(data[idx]);
    }
    HEADER const raw_handle<T> CLASS::operator[](size_t idx) const {
        return raw_handle<T>(data[idx]);
    }
    
    HEADER bool CLASS::is_active(size_t idx) const {
        return !availability[idx];
    }
    
    // set parameters for resizing
    HEADER void CLASS::set_resize_bias(size_t b) { bias = b; }
    HEADER void CLASS::set_resize_factor(size_t c) { factor = c; }
    
    HEADER size_t CLASS::insert(const T& value) {
        
        if( num_free_ids ){
            
            // grab next open ID
            size_t ID = get_free_id();
            
            // set the value for that reference
            // based on the input value
            *operator[](ID) = value;
            
            // return the ID
            return ID;
            
        }else{
            
            // if the number allocated equals the capacity
            // then resize the pool using doubling
            if( num_allocated == num_capacity ){
                reserve(num_capacity*factor + bias);
            }
            
            // construct an element off the back of this list
            auto& handle = data[num_allocated];
            handle.create();
            
            // overwrite the value
            *handle = value;
            
            // mark it as not available
            availability[num_allocated] = false;
            
            // increment the size of the list
            ++_size;
            
            // return the ID for this value
            return num_allocated++;
        }
    }
    HEADER void CLASS::erase(size_t idx) {
        if( !availability[idx] ){
            --_size;
            free_ids.push(idx);
            num_free_ids++;
            availability[idx] = true;
        }
    }
    HEADER size_t CLASS::get_free_id() {
        
        if( num_free_ids ){
            // grab ID off of the top of the list
            // and shrink the list size
            --num_free_ids;
            size_t new_id = free_ids.front(); free_ids.pop();
            ++_size;
            
            // mark the new ID as not available
            availability[new_id] = false;
            
            // return the new ID
            return new_id;
            
        }else{
            
            // if the number allocated equals the capacity
            // then resize the pool using doubling
            if( num_allocated == num_capacity ){
                reserve(num_capacity*factor + bias);
            }
            
            // construct an element off the back of this list
            data[num_allocated].create();
            
            // mark it as not available
            availability[num_allocated] = false;
            
            // increment the size of the list
            ++_size;
            
            // return the new ID
            return num_allocated++;
        }
    }
    HEADER size_t CLASS::size() const {
        return _size;
    }
    HEADER size_t CLASS::capacity() const {
        return num_capacity;
    }
    HEADER bool CLASS::empty() const {
        return _size == 0;
    }
    
    HEADER void CLASS::print_available_ids() const {
        printf("[ ");
        for(size_t i = 0; i < num_free_ids; ++i){
            size_t id = free_ids.front();
            free_ids.pop();
            free_ids.push(id);
            printf("%zu ", id);
        }
        printf("]\n");
    }
}

#undef HEADER
#undef CLASS

#endif /* pool_vector_h */
