#ifndef NBJKWWAFFEEGF_GTL_COMMAND_VARIANT_H_
#define NBJKWWAFFEEGF_GTL_COMMAND_VARIANT_H_

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)                              
    
    namespace gtl::
    
    command variant definition
-----------------------------------------------------------------------------*/

#include <boost/variant.hpp>
                            
namespace gtl {

    namespace commands {
        struct draw {};

        struct resize {};
        struct handle {};                       
    }

    using command_variant = boost::variant<
                                           commands::draw,
                                           commands::resize,
                                           commands::handle
                                          >;
     
} // namespace
#endif           
  
  