/*
 *  unique_handle.hpp
 *
 *  Project: meshing
 *  Author : Christian J Howard
 *  Date   : 5/15/18
 *  Purpose:
 *
 */

#ifndef unique_handle_interface_hpp
#define unique_handle_interface_hpp

#include <stdio.h>
#include <memory>

#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>

namespace local {
    template<class T> struct _Unique_if {
        typedef std::unique_ptr<T> _Single_object;
    };
    
    template<class T> struct _Unique_if<T[]> {
        typedef std::unique_ptr<T[]> _Unknown_bound;
    };
    
    template<class T, size_t N> struct _Unique_if<T[N]> {
        typedef void _Known_bound;
    };
    
    template<class T, class... Args>
    typename _Unique_if<T>::_Single_object
    make_unique(Args&&... args) {
        return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    }
    
    template<class T>
    typename _Unique_if<T>::_Unknown_bound
    make_unique(size_t n) {
        typedef typename std::remove_extent<T>::type U;
        return std::unique_ptr<T>(new U[n]());
    }
    
    template<class T, class... Args>
    typename _Unique_if<T>::_Known_bound
    make_unique(Args&&...) = delete;
}

namespace util {
        
    template<typename T>
    class unique_handle {
    public:
        
        // ctor/dtor
        unique_handle();
        unique_handle(unique_handle & h);
        unique_handle(const unique_handle & h);
        ~unique_handle() = default;
        
        // useful operators
        unique_handle& operator=(const unique_handle& h);
        unique_handle& operator=(unique_handle& h);
        unique_handle& operator=(const unique_handle&& h);
        T& operator*();
        const T& operator*() const;
        operator T*();
        operator const T*();
        
        // creation methods
        template <typename... Args>
        void create(Args&&... args){
            ref = std::move(local::make_unique<T>(std::forward<Args>(args)...));
        }
        
        // operators to allow use in containers
        bool operator<(const unique_handle<T>& h) const;
        bool operator==(const unique_handle<T>& h) const;
        bool operator!=(const unique_handle<T>& h) const;
        bool is_null() const;
        
        // methods to get handle data references
        T* operator->();
        const T* operator->() const;
        T* raw_ptr();
        const T* raw_ptr() const;
        T& raw_ref();
        const T& raw_ref() const;
        
        // free
        void free();
        
    private:
        mutable std::unique_ptr<T> ref;
        
    };
    
}// end namespace util



#include "unique_handle.hxx"

#endif /* unique_handle_interface_hpp */
