/*-------------------------------------------------------------

Copyright (c) 2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/

#ifndef NBZZZOPWPWRWR_GTL_SCENES_TRANSITION_SCENE_H_
#define NBZZZOPWPWRWR_GTL_SCENES_TRANSITION_SCENE_H_

/*-------------------------------------------------------------

not used -- keeping for reference

---------------------------------------------------------------*/

namespace gtl {
namespace scenes {
    namespace detail {

        //class transition_scene; // currently defined in scenes.h
        //
        //            class transition_scene {
        //                scenes::scene_variant first_, second_;
        //                std::chrono::high_resolution_clock::duration const transition_duration_;
        //                std::chrono::high_resolution_clock::time_point const begin_timepoint_;
        //                std::chrono::high_resolution_clock::time_point mutable current_timepoint_;
        //
        //            public:
        //
        //                transition_scene() = default;
        //                transition_scene(scenes::scene_variant&& s1, scenes::scene_variant&& s2, std::chrono::milliseconds t)
        //                    :   first_{std::move(s1)}, second_{std::move(s2)},
        //                        transition_duration_{t},
        //                        begin_timepoint_{std::chrono::high_resolution_clock::now()+t}
        //                    { assert(t.count() > 0); }
        //
        //                transition_scene(transition_scene const&) = default;
        //                transition_scene(transition_scene&&) = default;
        //                //transition_scene& operator=(transition_scene&&) = default;
        //
        //                template <typename YieldType>
        //                gtl::event handle_events(YieldType& yield) const {
        //                    std::chrono::high_resolution_clock::time_point const end_time_ = begin_timepoint_ + transition_duration_;
        //                    while ((current_timepoint_ = std::chrono::high_resolution_clock::now()) < end_time_)
        //                    {
        //                        if (same_type(yield.get(),gtl::events::exit_immediately{})) {
        //                            return gtl::events::exit_immediately{};
        //                        }
        //                        yield();
        //                    }
        //                    return gtl::events::exit_state{0};
        //                }
        //
        //                void draw(float) const {
        //                    float r = (current_timepoint_ - begin_timepoint_).count() / static_cast<float>(transition_duration_.count());
        //                    if (r < 0.0f) { r = 0.0f; } else if (r > 1.0f) { r = 1.0f; }
        //                    boost::apply_visitor([&](auto& v){ v.draw(1.0f-r); },first_);
        //                    boost::apply_visitor([&](auto& v){ v.draw(r); },second_);
        //                }
        //
        //                auto first() { return first_; }
        //                auto second() { return second_; }
        //
        //            };
        //
    }
}
} // namespaces
#endif
