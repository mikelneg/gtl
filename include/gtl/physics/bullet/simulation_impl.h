/*-------------------------------------------------------------

Copyright (c) 2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/

#ifndef BWAFOWIEJFAOIG_GTL_PHYSICS_BULLET_SIMULATION_IMPL_H_
#define BWAFOWIEJFAOIG_GTL_PHYSICS_BULLET_SIMULATION_IMPL_H_

#include <gtl/physics/simulation_interface.h>

#include <atomic>
#include <thread>

//#include <gtl/swap_vector.h>

#include <vn/single_consumer_queue.h>
#include <vn/swap_object.h>

#include <gtl/physics/common_types.h>
#include <gtl/physics/command_variant.h>
#include <gtl/box2d_adapter.h>               // HACK change this..

namespace gtl {
namespace physics {

    class bullet_simulation : public simulation {
        
        vn::swap_object<simulation_render_data> render_data_;
    
        std::atomic_flag quit_;
        std::thread thread_;
    
    public:

        bullet_simulation(vn::single_consumer_queue<gtl::physics::command_variant>&, 
                          gtl::box2d_adapter&);
    
        bool extract_render_data(simulation_render_data& c) final 
        {
            return render_data_.swap_out(c);
        }
    
        ~bullet_simulation() final
        {
            quit_.clear();
            thread_.join();
        }
    };

}
} // namespace
#endif