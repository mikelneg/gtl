/*-------------------------------------------------------------

Copyright (c) 2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/

#ifndef IWOIWBZLWAFWEF_GTL_SCENES_H_
#define IWOIWBZLWAFWEF_GTL_SCENES_H_

/*-------------------------------------------------------------

not used -- keeping for reference

---------------------------------------------------------------*/

#include <boost/functional/hash.hpp>
#include <boost/variant.hpp>
#include <gtl/events.h>

#include <vn/boost_variant_utilities.h>
#include <vn/has_equality_operator.h>
#include <vn/type_list.h>

//#include <gtl/intro_scene.h>
//#include <gtl/main_scene.h>
#include <gtl/d3d_types.h>
#include <vector>

#include <utility>

#include <cassert>
#include <chrono>

namespace gtl {

namespace scenes {

    // class intro_scene;
    // class main_scene;

    namespace detail {
        struct empty_scene { // we are using a custom empty_scene for the variant, so no need for boost::blank cases in
                             // visitors..
            empty_scene() = default;
            empty_scene(empty_scene const&) = default;
            empty_scene(empty_scene&&) = default;
            empty_scene& operator=(empty_scene&&) = default;
            std::vector<ID3D12CommandList*> draw(int, float, gtl::d3d::rtv_descriptor_heap&) const
            {
                return {};
            }
            template <typename YieldType>
            gtl::event handle_events(YieldType&) const
            {
                return gtl::events::exit_state{0};
            }
        };

        static_assert(boost::has_nothrow_copy<empty_scene>::value, "empty_scene() must satisfy boost::has_nothrow_copy");
        static_assert(boost::has_nothrow_constructor<empty_scene>::value, "empty_scene() must satisfy boost::has_nothrow_constructor");

        template <typename>
        class transition_scene;

        template <typename>
        class inverse_transition_scene;
    }

    template <typename... Ts>
    using scene_variant = typename boost::make_recursive_variant<detail::empty_scene, Ts..., detail::transition_scene<boost::recursive_variant_>,
                                                                 detail::inverse_transition_scene<boost::recursive_variant_>>::type;

    template <typename VariantType>
    bool has_same_type(VariantType const& lhs, VariantType const& rhs)
    {
        return apply_visitor(vn::visitors::same_type{}, lhs, rhs);
    }

    namespace detail {
        template <typename T>
        class transition_scene { // currently defined here (instead of transition_scene.h) -- might relocate..
            T first_, second_;
            std::chrono::high_resolution_clock::duration transition_duration_;
            std::chrono::high_resolution_clock::time_point begin_timepoint_;
            std::chrono::high_resolution_clock::time_point mutable current_timepoint_;

        public:
            transition_scene(T&& s1, T&& s2, std::chrono::milliseconds t)
                : first_{std::move(s1)}, second_{std::move(s2)}, transition_duration_{t}, begin_timepoint_{std::chrono::high_resolution_clock::now()}
            {
                assert(t.count() > 0);
            }

            // transition_scene() = default;
            transition_scene(transition_scene&&) = default;
            transition_scene& operator=(transition_scene&&) = default;

            template <typename YieldType>
            gtl::event handle_events(YieldType& yield) const
            {
                std::chrono::high_resolution_clock::time_point const end_time_ = begin_timepoint_ + transition_duration_;
                while ((current_timepoint_ = std::chrono::high_resolution_clock::now()) < end_time_)
                {
                    if (same_type(yield.get(), gtl::event{gtl::events::exit_immediately{}}))
                    {
                        return yield.get();
                    }
                    yield();
                }
                return gtl::events::exit_state{0};
            }

            // template <typename I, typename RTV, typename CLIST>
            // void draw(float, I idx, RTV& rtv, CLIST& clist_) const {
            //    float r = (current_timepoint_ - begin_timepoint_).count() /
            //    static_cast<float>(transition_duration_.count());
            //    if (r < 0.0f) { r = 0.0f; } else if (r > 1.0f) { r = 1.0f; }
            //    boost::apply_visitor([&](auto& v){ v.draw(idx,rtv,clist_,1.0f-r); },first_);
            //    boost::apply_visitor([&](auto& v){ v.draw(idx,rtv,clist_,r); },second_);
            //}

            //  How to approach this:
            //  (1) render targets are completely decided internally by the scene, they do all the binding, and they
            //  close
            //      their own list(s)
            //  (2) vector<commandlist*> is returned and somewhere up the chain they are executed..
            //  (3) what about indices for double-buffered resources..

            std::vector<ID3D12CommandList*> draw(int idx, float, gtl::d3d::rtv_descriptor_heap& rtv) const
            {
                float r = (current_timepoint_ - begin_timepoint_).count() / static_cast<float>(transition_duration_.count());
                if (r < 0.0f)
                {
                    r = 0.0f;
                }
                else if (r > 1.0f)
                {
                    r = 1.0f;
                }
                auto v{boost::apply_visitor([&](auto& v) { return v.draw(idx, 1.0f - r, rtv); }, first_)};
                auto v_2 = boost::apply_visitor([&](auto& v) { return v.draw(idx, r, rtv); }, second_);
                v.insert(end(v), begin(v_2), end(v_2));
                return v;
            }

            T const& first() const
            {
                return first_;
            }
            T const& second() const
            {
                return second_;
            }

            T&& swap_first(T&& t)
            {
                using std::swap;
                swap(t, first_);
                return std::move(t);
            }
            T&& swap_second(T&& t)
            {
                using std::swap;
                swap(t, second_);
                return std::move(t);
            }
        };

        template <typename T>
        class inverse_transition_scene { // currently defined here (instead of transition_scene.h) -- might relocate..
            T first_, second_;
            std::chrono::high_resolution_clock::duration transition_duration_;
            std::chrono::high_resolution_clock::time_point begin_timepoint_;
            std::chrono::high_resolution_clock::time_point mutable current_timepoint_;

        public:
            inverse_transition_scene(T&& s1, T&& s2, std::chrono::milliseconds t)
                : first_{std::move(s1)}, second_{std::move(s2)}, transition_duration_{t}, begin_timepoint_{std::chrono::high_resolution_clock::now()}
            {
                assert(t.count() > 0);
            }

            // transition_scene() = default;
            inverse_transition_scene(inverse_transition_scene&&) = default;
            inverse_transition_scene& operator=(inverse_transition_scene&&) = default;

            template <typename YieldType>
            gtl::event handle_events(YieldType& yield) const
            {
                std::chrono::high_resolution_clock::time_point const end_time_ = begin_timepoint_ + transition_duration_;
                while ((current_timepoint_ = std::chrono::high_resolution_clock::now()) < end_time_)
                {
                    if (same_type(yield.get(), gtl::event{gtl::events::exit_immediately{}}))
                    {
                        return yield.get();
                    }
                    yield();
                }
                return gtl::events::exit_state{0};
            }

            // template <typename I, typename RTV, typename CLIST>
            // void draw(float, I idx, RTV& rtv, CLIST& clist_) const {
            //    float r = (current_timepoint_ - begin_timepoint_).count() /
            //    static_cast<float>(transition_duration_.count());
            //    if (r < 0.0f) { r = 0.0f; } else if (r > 1.0f) { r = 1.0f; }
            //    boost::apply_visitor([&](auto& v){ v.draw(idx,rtv,clist_,1.0f-r); },first_);
            //    boost::apply_visitor([&](auto& v){ v.draw(idx,rtv,clist_,r); },second_);
            //}

            //  How to approach this:
            //  (1) render targets are completely decided internally by the scene, they do all the binding, and they
            //  close
            //      their own list(s)
            //  (2) vector<commandlist*> is returned and somewhere up the chain they are executed..
            //  (3) what about indices for double-buffered resources..

            std::vector<ID3D12CommandList*> draw(int idx, float, gtl::d3d::rtv_descriptor_heap& rtv) const
            {
                float r = (current_timepoint_ - begin_timepoint_).count() / static_cast<float>(transition_duration_.count());
                if (r < 0.0f)
                {
                    r = 0.0f;
                }
                else if (r > 1.0f)
                {
                    r = 1.0f;
                }
                auto v{boost::apply_visitor([&](auto& v) { return v.draw(idx, r, rtv); }, second_)};
                auto v_2 = boost::apply_visitor([&](auto& v) { return v.draw(idx, 1.0f - r, rtv); }, first_);
                v.insert(end(v), begin(v_2), end(v_2));
                return v;
            }

            T const& first() const
            {
                return first_;
            }
            T const& second() const
            {
                return second_;
            }

            T&& swap_first(T&& t)
            {
                using std::swap;
                swap(t, first_);
                return std::move(t);
            }
            T&& swap_second(T&& t)
            {
                using std::swap;
                swap(t, second_);
                return std::move(t);
            }
        };

        template <typename V>
        struct variant_hash : boost::static_visitor<std::size_t> {
            template <typename T>
            inline std::size_t operator()(T const&) const
            {
                return boost::hash<int>{}(vn::index_of<T, vn::mpl_sequence_to_list<V>>::value);
            }

            template <typename T>
            inline std::size_t operator()(transition_scene<T> const& t) const
            {
                std::size_t s = 0;
                // boost::hash<int>{}( vn::index_of<transition_scene<T>, vn::mpl_sequence_to_list<V>>::value);
                boost::hash_combine(s, apply_visitor(*this, t.first()));
                boost::hash_combine(s, apply_visitor(*this, t.second()));
                return s;
            }
        };
    }

} // namespace

// template <typename ...Ts>
// using scene = scenes::scene_variant<Ts...>;

} // namespace gtl

#endif