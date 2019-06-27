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
            ref = std::move(std::make_unique<T>(std::forward<Args>(args)...));
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
        std::unique_ptr<T> ref;
        
    };
    
}// end namespace util



#include "unique_handle.hxx"

#endif /* unique_handle_interface_hpp */
