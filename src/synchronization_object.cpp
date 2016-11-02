/*-------------------------------------------------------------

Copyright (c) 2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/

#include "gtl/synchronization_object.h"

#include <atomic>
#include <cassert>
#include <cstdint>
#include <gtl/d3d_helper_funcs.h>
#include <gtl/d3d_types.h>

namespace gtl {
namespace d3d {

    frame_synchronizer::frame_synchronizer(command_queue& cqueue_, unsigned max_value_in_cycle, unsigned tolerance)
        : cqueue_{cqueue_}, fence_{gtl::d3d::get_device_from(cqueue_)}, tolerance_{tolerance}, cycle_length_{max_value_in_cycle + 1}
    {
        //
        // cycle_length:  1 2 3 4 5 ...
        //
        // possible
        // tolerances:    0 0 0 0 0
        //                  1 1 1 1
        //                    2 2 2
        //                      3 3
        //                        4

        // so, for example, if length is 5 and tolerance is 4, and if the last-observed-frame-index was 1,
        // we are still "synchronized" if the last-submitted-frame-index is 5 (5-1=4, which is the tolerance);
        // if the last-observed-frame-index remains 1, though, then we will be desynchronized if the
        // last-submitted-frame-index is 6 or greater..

        assert(tolerance < cycle_length_);
        if (tolerance >= cycle_length_)
        {
            throw std::logic_error{"frame_synchronizer tolerance must be less than the cycle length."};
        }
        wait_for_values_to_sync_at(0);
    }

    frame_synchronizer::~frame_synchronizer()
    {
        wait_for_values_to_sync_at(0);
    }

    unsigned frame_synchronizer::max_value() const
    {
        return static_cast<unsigned>(cycle_length_ - 1);
    }

    unsigned frame_synchronizer::periodic_value() const
    {
        return static_cast<unsigned>(last_set_value_ % cycle_length_);
    }

    void frame_synchronizer::submit()
    {
        cqueue_->Signal(fence_.get(), last_set_value_);
        ++last_set_value_;
    }

    void frame_synchronizer::synchronized_submit()
    {
        fence_.synchronized_increment(cqueue_);
        ++last_set_value_;
    }

    bool frame_synchronizer::synchronized() const
    {
        uint64_t diff = last_set_value_ - last_observed_value_;
        if (diff > tolerance_)
        {
            last_observed_value_ = fence_->GetCompletedValue();
            diff = last_set_value_ - last_observed_value_;
        }
        return (diff <= tolerance_);
    }

    void frame_synchronizer::wait_for_values_to_sync_at(uint64_t new_value)
    {
        last_set_value_ = new_value;
        fence_.synchronized_set(last_set_value_, cqueue_);
        last_observed_value_ = fence_->GetCompletedValue();
        assert(last_observed_value_ == last_set_value_);
    }

    //

    synchronization_object::synchronization_object(command_queue& cqueue_, unsigned max_value_in_cycle, unsigned allowed_desync_)
        : cqueue_{cqueue_}, fence_{gtl::d3d::get_device_from(cqueue_)}, allowed_latency_{allowed_desync_}, cycle_length_{max_value_in_cycle + 1}
    {
        assert(allowed_latency_ <= cycle_length_ - 2);
        wait_for_values_to_sync_at(0);
    }

    synchronization_object::~synchronization_object()
    {
        wait_for_values_to_sync_at(0);
    }

    unsigned synchronization_object::periodic_value() const
    {
        return static_cast<unsigned>(last_set_value_ % cycle_length_);
    }

    void synchronization_object::advance()
    {
        cqueue_->Signal(fence_.get(), last_set_value_);
        ++last_set_value_;
    }

    void synchronization_object::synchronous_advance()
    {
        fence_.synchronized_increment(cqueue_);
        ++last_set_value_;
    }

    bool synchronization_object::values_are_synchronized() const
    {
        // TODO it's possible this could be improved by remembering the fence's value and only polling when necessary..
        uint64_t const diff = last_set_value_ - fence_->GetCompletedValue();
        return (diff <= allowed_latency_);
    }

    void synchronization_object::wait_for_values_to_sync_at(uint64_t new_value)
    {
        last_set_value_ = new_value;
        fence_.synchronized_set(last_set_value_, cqueue_);
        assert(last_set_value_ == fence_->GetCompletedValue());
    }
}
} // namespaces