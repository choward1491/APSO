//
//  pool_vector.hpp
//  meshing
//
//  Created by Christian Howard on 4/3/19.
//  Copyright © 2019 SDG. All rights reserved.
//

#ifndef pool_vector_hpp
#define pool_vector_hpp

#include <queue>
#include <atomic>
#include "unique_handle.hpp"
#include "raw_handle.hpp"

namespace util {
        
    // define default container type
    template<typename S> using std_vec = std::vector<S>;
    
    // define reusable data container
    template<typename T,
            template<typename S> class container = std_vec >
    class pool_vector {
    public:
        
        // ctor/dtor
        pool_vector(size_t init_allocation = 64);
        
        // method for reserving the size
        bool reserve(size_t size);
        
        // method for retrieving the handles for data
        raw_handle<T> operator[](size_t idx);
        const raw_handle<T> operator[](size_t idx) const;
        bool is_active(size_t idx) const;
        
        // method for inserting new data by value
        size_t insert(const T& value);
        
        // method for removing data at an index
        void erase(size_t idx);
        
        // method for grabbing an open ID
        size_t get_free_id();
        
        // method for returning the number
        // of elements being used in the vector
        size_t size() const;
        size_t capacity() const;
        bool empty() const;
        
        // set parameters for resizing
        void set_resize_bias(size_t b);
        void set_resize_factor(size_t c);
        
        // print available IDs
        void print_available_ids() const;
        
    private:
        
        // internal counter and modification vars
        std::atomic<size_t> num_free_ids, num_allocated, num_capacity, _size, bias, factor;
        
        // list that stores whether a given data item is
        // available for being recycled or not
        container<bool> availability;
        
        // list that stores available IDs
        //container<size_t> free_ids;
        mutable std::queue<size_t> free_ids;
        
        // list that stores data, whether in use or
        // an item to be recycled
        container<unique_handle<T>> data;
        
    };
}


#include "pool_vector.hxx"

#endif /* pool_vector_h */
