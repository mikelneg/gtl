/*-------------------------------------------------------------

Copyright (c) 2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/

#ifndef YUWOQWFAVF_GTL_D3D_SYNCHRONIZATION_OBJECT_H_
#define YUWOQWFAVF_GTL_D3D_SYNCHRONIZATION_OBJECT_H_

#include <atomic>
#include <cassert>
#include <cstdint>
#include <gtl/d3d_types.h>

namespace gtl {
namespace d3d {

    class synchronization_object {

        gtl::d3d::command_queue& cqueue_;
        gtl::d3d::fence fence_;

        uint64_t last_set_value_{};
        uint64_t const allowed_latency_{};
        uint64_t const cycle_length_{};

    public:
        synchronization_object(gtl::d3d::command_queue&,                 // example:
                               unsigned max_value_in_cycle,              // If desired cycle is [0,1,2,3], max_value_in_cycle should be 3.
                               unsigned tolerated_difference_in_values); // If tolerated_diff is 2, the "completed" index value can lag behind
                                                                         //    the index value submitted to op() by 2; when it is greater,
        //    the desync_call is executed instead (might be useful for logging..)
        template <typename F, typename G>
        void operator()(F&& sync_call, G&& desync_call)
        {
            if (values_are_synchronized())
            {
                sync_call(*this);
            }
            else
            {
                desync_call();
            }
        }

        ~synchronization_object(); // synchronizes

        synchronization_object(synchronization_object&&) = delete;
        synchronization_object& operator==(synchronization_object&&) = delete;

        friend unsigned value(synchronization_object const& s)
        {
            return s.periodic_value();
        }
        friend void advance(synchronization_object& s)
        {
            s.advance();
        }
        friend void synchronous_advance(synchronization_object& s)
        {
            s.synchronous_advance();
        }

    private:
        void advance(); // TODO should probably consider making a distinct type for sync/async synchronization
        void synchronous_advance();
        unsigned periodic_value() const;
        bool values_are_synchronized() const;
        void wait_for_values_to_sync_at(uint64_t new_value); // synchronizes
    };

    //////////

    class frame_synchronizer {

        gtl::d3d::command_queue& cqueue_;
        gtl::d3d::fence fence_;

        uint64_t const tolerance_{};
        uint64_t const cycle_length_{};

        uint64_t mutable last_observed_value_{};
        uint64_t last_set_value_{};

        class frame_index {
            frame_synchronizer& parent_;
            inline auto value() const
            {
                return parent_.periodic_value();
            }
            inline auto max_value() const
            {
                return parent_.max_value();
            }
            inline void submit()
            {
                parent_.submit();
            }
            inline void synchronized_submit()
            {
                parent_.synchronized_submit();
            }

        public:
            constexpr frame_index(frame_synchronizer& p) noexcept : parent_{p}
            {
            }
            friend auto max_value(frame_index const& s)
            {
                return s.max_value();
            }
            friend auto value(frame_index const& s)
            {
                return s.value();
            }
            friend void submit(frame_index& s)
            {
                s.submit();
            }
            friend void async_advance(frame_index& s)
            {
                s.submit();
            }
            friend void synchronized_submit(frame_index& s)
            {
                s.synchronized_submit();
            }
        };

        void submit();              // advances the value asynchronously
        void synchronized_submit(); // advances the value synchronosouly
        unsigned periodic_value() const;
        unsigned max_value() const;
        bool synchronized() const;
        void wait_for_values_to_sync_at(uint64_t new_value); // synchronizes

    public:
        frame_synchronizer(gtl::d3d::command_queue&,
                           unsigned max_index_value, // maximum value in the set of cycled index values
                           unsigned tolerance);      // maximum difference between submitted and completed frame indices

        ~frame_synchronizer(); // synchronizes

        frame_synchronizer(frame_synchronizer&&) = delete;
        frame_synchronizer& operator==(frame_synchronizer&&) = delete;

        template <typename F, typename G>
        void operator()(F&& sync_call, G&& desync_call)
        {
            if (synchronized())
            {
                sync_call(frame_index{*this});
            }
            else
            {
                desync_call();
            }
        }

        template <typename F>
        void force_synchronize_then_call(F&& func)
        {
            synchronized_submit();
            func();
        }
    };
}
} // namespaces
#endif
