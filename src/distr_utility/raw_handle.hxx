//
//  ptr_handle.hxx
//  meshing
//
//  Created by Christian Howard on 4/3/19.
//  Copyright © 2019. All rights reserved.
//

#ifndef ptr_handle_hxx
#define ptr_handle_hxx

#define HEADER template<typename T>
#define CLASS raw_handle<T>

#include "raw_handle.hpp"

namespace util {
            
    // ctor/dtor
    HEADER CLASS::raw_handle():ptr(nullptr){}
    HEADER CLASS::raw_handle(T* sp):ptr(sp){}
    HEADER CLASS::raw_handle(const unique_handle<T>& h):ptr((T*)h.raw_ptr()) {}
    HEADER CLASS::raw_handle(const std::unique_ptr<T> & sp):ptr(sp.get()) {}
    HEADER CLASS::raw_handle(const raw_handle & h):ptr(h.ptr){}
    HEADER CLASS::~raw_handle(){ ptr = nullptr; }
    
    // useful operators
    HEADER CLASS& CLASS::operator=(T* _ptr) {
        ptr = _ptr;
        return *this;
    }
    HEADER CLASS& CLASS::operator=(const raw_handle& h) {
        if( this != &h ){
            ptr = h.ptr;
        }
        return *this;
    }
    HEADER CLASS& CLASS::operator=(const std::unique_ptr<T>& sp) {
        ptr = (T*)sp.get();
        return *this;
    }
    HEADER CLASS& CLASS::operator=(const unique_handle<T>& h) {
        ptr = (T*)h.raw_ptr();
        return *this;
    }
    HEADER T& CLASS::operator*() { return *ptr; }
    HEADER const T& CLASS::operator*() const { return *ptr; }
    HEADER CLASS::operator T*() {return *ptr; }
    HEADER CLASS::operator const T*() { return *ptr; }
    
    // operators to allow use in containers
    HEADER bool CLASS::operator<(const raw_handle& h) const {
        return ptr < h.ptr;
    }
    HEADER bool CLASS::operator==(const raw_handle& h) const {
        return ptr == h.ptr;
    }
    HEADER bool CLASS::operator!=(const raw_handle& h) const {
        return ptr != h.ptr;
    }
    HEADER bool CLASS::is_null() const { return ptr; }
    
    // methods to get handle data references
    HEADER T* CLASS::operator->() { return ptr; }
    HEADER const T* CLASS::operator->() const { return ptr; }
    HEADER T* CLASS::raw_ptr() { return ptr; }
    HEADER const T* CLASS::raw_ptr() const { return ptr; }
    HEADER T& CLASS::raw_ref() { return ptr; }
    HEADER const T& CLASS::raw_ref() const { return ptr; }
    
    //T* ptr;
    
}// end namespace util


#undef HEADER
#undef CLASS
#endif /* ptr_handle_h */
