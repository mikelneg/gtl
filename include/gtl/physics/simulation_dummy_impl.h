/*-------------------------------------------------------------

Copyright (c) 2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/

#ifndef RIWOWOAOASJIFAA_GTL_PHYSICS_SIMULATION_DUMMY_IMPL_H_
#define RIWOWOAOASJIFAA_GTL_PHYSICS_SIMULATION_DUMMY_IMPL_H_

#include <atomic>
#include <thread>
#include <type_traits>
#include <vector>

#include <gtl/swap_vector.h>

#include <vn/single_consumer_queue.h>
#include <vn/swap_object.h>

#include <boost/variant.hpp>

#include <gtl/entity/render_data.h>
#include <gtl/physics/units.h>
#include <gtl/physics/common_types.h>
#include <gtl/physics/command_variant.h>

#include <gtl/physics/simulation_interface.h>

#include <gtl/draw_kit.h>

namespace gtl {
namespace physics {

    class simulation_dummy_impl : public simulation {
        using entity_type = entity::render_data;

        vn::swap_object<simulation_render_data> render_data_;

        std::atomic_flag quit_;
        std::thread thread_;

    public:
        simulation_dummy_impl(vn::single_consumer_queue<gtl::physics::command_variant>&, gtl::draw_kit&);

        bool extract_render_data(simulation_render_data& c) final
        {
            return render_data_.swap_out(c);
        }

        ~simulation_dummy_impl() final
        {
            quit_.clear();
            thread_.join();
        }
    };
}
} // namespace
#endif