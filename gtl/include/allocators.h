#ifndef YWBVVZXZZC_GTL_ALLOCATORS_H_
#define YWBVVZXZZC_GTL_ALLOCATORS_H_

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)                              
    
    namespace gtl::allocators::

    useful allocator types:

        template <typename T>
        class no_trivial_value_initialization     (alias no_trivial_init<T>)
        
            - If type has trivial default construction, does nothing (no value initialization)
            - Useful in cases like "std::vector<char, no_trivial_init<char>> v(100);"
              to initialize the vector _without_ initializing each char             
                    
-----------------------------------------------------------------------------*/

#include <type_traits>
#include <utility>
#include <memory>

#include <iostream>

namespace gtl {
namespace allocators {
    
    template <typename T, typename A = std::allocator<T>>
    struct no_trivial_value_initialization : A {                
        
        template <typename R> 
        struct rebind {
            using other = no_trivial_value_initialization<R,typename std::allocator_traits<A>::template rebind_alloc<R>>;
        };
    
        using A::A;
    
        template <typename R>
        inline std::enable_if_t<std::is_trivially_default_constructible<R>::value>
        construct(R*) noexcept {} // do nothing
    };

    template <typename T>
    using no_trivial_init = no_trivial_value_initialization<T>;
        
}} // namespaces
#endif
