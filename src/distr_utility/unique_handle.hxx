/*
 *  generic_handle.hxx
 *
 *  Project: meshing
 *  Author : Christian J Howard
 *  Date   : 5/15/18
 *  Purpose:
 *
 */

#ifndef generic_handle_impl_hxx
#define generic_handle_impl_hxx

// useful macros for defining class methods
#define HEADER template<typename T>
#define CLASS unique_handle<T>

// template class interface
#include "unique_handle.hpp"

// implementation details

namespace util {
        
    HEADER CLASS::unique_handle():ref(nullptr) {
        
    }
    
    HEADER CLASS::unique_handle(const unique_handle & h):ref(nullptr) {
        if( !h.is_null() ){
            ref.reset( new T() );
            *ref.get() = *h;
        }
    }
    
    HEADER CLASS::unique_handle(unique_handle & h):ref(nullptr) {
        if( !h.is_null() ){
            ref = std::move(h.ref);
        }
    }
    
    HEADER CLASS& CLASS::operator=(const unique_handle& h) {
        if( this != &h ){
            ref = nullptr;
            if( !h.is_null() ){
                ref.reset( new T() );
                *ref.get() = *h;
            }
        }
        return *this;
    }
    
    HEADER CLASS& CLASS::operator=(unique_handle& h) {
        if( this != &h ){
            ref = nullptr;
            if( !h.is_null() ){
                ref = std::move(h.ref);
            }
        }
        return *this;
    }
    
    HEADER CLASS& CLASS::operator=(const unique_handle&& h) {
        ref = std::move(h.ref);
        return *this;
    }
    
    HEADER T& CLASS::operator*() {
        return raw_ref();
    }
    HEADER const T& CLASS::operator*() const {
        return raw_ref();
    }
    HEADER CLASS::operator T*() {
        return ref.get();
    }
    HEADER CLASS::operator const T*() {
        return ref.get();
    }
    
    HEADER T* CLASS::operator->() {
        return ref.get();
    }
    
    HEADER const T* CLASS::operator->() const {
        return ref.get();
    }
    
    HEADER bool CLASS::is_null() const {
        return !ref.get();
    }
    
    HEADER bool CLASS::operator<(const unique_handle<T>& h) const {
        const T *pl = ref.get();
        const T *pr = h.raw_ptr();
        /*
        if( pl == nullptr && pr != nullptr ){ return true; }
        else if( pl != nullptr && pr == nullptr ){ return false; }
        else{ return pl < pr; }*/
        return pl < pr;
    }
    
    HEADER bool CLASS::operator==(const unique_handle<T>& h) const {
        return !this->operator!=(h);
    }
    HEADER bool CLASS::operator!=(const unique_handle<T>& h) const {
        return h.raw_ptr() != ref.get();
    }
    
    HEADER T* CLASS::raw_ptr(){
        return ref.get();
    }
    HEADER const T* CLASS::raw_ptr() const {
        return ref.get();
    }
    HEADER T& CLASS::raw_ref() {
        return *ref.get();
    }
    HEADER const T& CLASS::raw_ref() const {
        return *ref.get();
    }
    
    HEADER void CLASS::free() {
        ref = nullptr;
    }
    
}// end namespace util


// get rid of macro definitions
#undef HEADER
#undef CLASS

#endif /* generic_handle_impl_hxx */


