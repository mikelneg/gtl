/*-------------------------------------------------------------

Copyright (c) const2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/

#ifndef OWIUHFOAIOJOSIVWOAF_GTL_PHYSICS_BULLET_COMMAND_VISITOR_H_
#define OWIUHFOAIOJOSIVWOAF_GTL_PHYSICS_BULLET_COMMAND_VISITOR_H_

/*-------------------------------------------------------------

    class gtl::physics::bullet::command_visitor

    -   visitor that handles gtl::physics::command_variant types
        for the Bullet physics engine

---------------------------------------------------------------*/

#include <gtl/physics/command_variant.h>
#include <iostream>
#include <cassert>

namespace gtl {
namespace physics {
namespace detail {

    template <typename World, typename Map>
    struct bullet_command_visitor {

        World& world_;
        Map& map_;        

        template <typename T>
        bool operator()(T const&) const { assert(false); return true; } // shouldn't be called
          
        bool operator()(commands::static_box const& o)
           { 
               return true;  
           }
        bool operator()(commands::dynamic_jointed_boxes const& o) const
           { 
               return true;  
           }
        bool operator()(commands::dynamic_box const& o) const           
           {   
               return true;  
           }
        bool operator()(commands::destroy_object_implode const& o) const
           { 
               return true;  
           }
        bool operator()(commands::boost_object const& o) const
           { 
               std::cout << "impl dudes!\n"; 
               return true;  
           }
        bool operator()(commands::boost_object_vec const& o) const   
           { 
               return true;  
           }
        bool operator()(commands::drive_object_vec const& o) const
           { 
               return true;  
           }        
    };                        

}}}   // namespace
#endif