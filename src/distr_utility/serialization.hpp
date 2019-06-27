//
//  serialization.hpp
//  meshing
//
//  Created by Christian Howard on 5/15/19.
//  Copyright © 2019. All rights reserved.
//

#ifndef serialization_h
#define serialization_h

#include <cstdio>
#include <vector>

namespace util {
        
    // serialization for primitive types
    template<typename T> size_t byte_content(const T& data){
        return sizeof(T);
    }
    
    template<typename T> size_t serialize(const T& data, unsigned char* buffer, size_t start_idx = 0)
    {
        std::memcpy((void*)(buffer + start_idx), (void*)&data, sizeof(data));
        return start_idx + sizeof(data);
    }
    template<typename T> size_t deserialize(T& data, const unsigned char* buffer, size_t start_idx = 0)
    {
        std::memcpy((void*)&data, (void*)(buffer + start_idx), sizeof(data));
        return start_idx + sizeof(data);
    }
    
    // serialization for vector containers
    template<typename T> size_t byte_content(const std::vector<T>& data){
        
        // get number of bytes for the size variable
        size_t num_req_bytes = sizeof(size_t);
        
        // get the number of bytes for each element of the vector
        for(size_t i = 0; i < data.size(); ++i){
            num_req_bytes += byte_content(data[i]);
        }// end for i
        
        // return the total number of required bytes
        return num_req_bytes;
    }
    
    template<typename T> size_t serialize(const std::vector<T>& data, unsigned char* buffer, size_t start_idx = 0)
    {
        // add the number of elements in the data
        const size_t size_ = data.size();
        start_idx = serialize(size_, buffer, start_idx);
        
        // add the actual data elements
        for(size_t i = 0; i < size_; ++i){
            start_idx = serialize(data[i], buffer, start_idx);
        }// end for i
        
        // return the final index after adding all the data
        return start_idx;
    }
    
    template<typename T> size_t deserialize(std::vector<T>& data, const unsigned char* buffer, size_t start_idx = 0)
    {
        // get the number of elements in the array
        size_t size_ = 0;
        start_idx = deserialize(size_, buffer, start_idx);
        
        // resize the input array
        data.resize(size_);
        
        // fill the array with the data in the buffer
        for(size_t i = 0; i < size_; ++i){
            start_idx = deserialize(data[i], buffer, start_idx);
        }// end for i
        
        // return the number of bytes we are at in the array
        return start_idx;
    }
    
}

#endif /* serialization_h */
