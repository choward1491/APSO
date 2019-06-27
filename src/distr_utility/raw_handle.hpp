//
//  raw_handle.hpp
//  meshing
//
//  Created by Christian Howard on 4/3/19.
//  Copyright © 2019. All rights reserved.
//

#ifndef raw_handle_hpp
#define raw_handle_hpp

#include <stdio.h>
#include <memory>
#include "unique_handle.hpp"

namespace util {
        
    template<typename T>
    class raw_handle {
    public:
        
        // ctor/dtor
        raw_handle();
        raw_handle(T* sp);
        raw_handle(const std::unique_ptr<T> & sp);
        raw_handle(const unique_handle<T> & sp);
        raw_handle(const raw_handle & h);
        ~raw_handle();
        
        // useful operators
        raw_handle& operator=(T* ptr);
        raw_handle& operator=(const unique_handle<T>& h);
        raw_handle& operator=(const raw_handle& h);
        raw_handle& operator=(const std::unique_ptr<T>& sp);
        T& operator*();
        const T& operator*() const;
        operator T*();
        operator const T*();
        
        // operators to allow use in containers
        bool operator<(const raw_handle& h) const;
        bool operator==(const raw_handle& h) const;
        bool operator!=(const raw_handle& h) const;
        bool is_null() const;
        
        // methods to get handle data references
        T* operator->();
        const T* operator->() const;
        T* raw_ptr();
        const T* raw_ptr() const;
        T& raw_ref();
        const T& raw_ref() const;
        
    private:
        T* ptr;
        
    };
    
}// end namespace util

#include "raw_handle.hxx"

#endif /* raw_handle_h */
