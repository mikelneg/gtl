/*-------------------------------------------------------------

Copyright (c) 2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/

#ifndef YWGOWAZXBQQWEFEVVOEFIJH_GTL_SCENE_H_
#define YWGOWAZXBQQWEFEVVOEFIJH_GTL_SCENE_H_

#include <memory>
#include <type_traits>
#include <utility>

#include <gtl/tags.h>

namespace gtl {

template <typename CommandVariant>
class scene {

    struct scene_interface {
        virtual void dispatch(CommandVariant) const = 0;
        virtual ~scene_interface()
        {
        }
    };

    template <typename T>
    class scene_priv_impl final : public scene_interface {

        T obj;

    public:
        template <typename... Qs>
        scene_priv_impl(Qs&&... qs) // noexcept(noexcept(obj(std::forward<Qs>(qs)...)))
            : obj(std::forward<Qs>(qs)...)
        {
        }

        virtual void dispatch(CommandVariant v) const final
        {
            apply_visitor(obj, v); // adl call, should pick up boost::apply_visitor if using boost::variant CommandVariant
        }
    };

    std::unique_ptr<scene_interface> ptr;

public:
    template <typename C, typename... Ps>
    scene(gtl::tags::construct<C>, Ps&&... ps) : ptr{std::make_unique<scene_priv_impl<C>>(std::forward<Ps>(ps)...)}
    {
    }

    template <typename T>
    scene(T&& t) : ptr{std::make_unique<scene_priv_impl<std::remove_reference_t<T>>>(std::forward<T>(t))}
    {
    }

    scene(scene&&) = default;
    scene& operator=(scene&&) = default;

    template <typename T>
    inline void send_command(T&& t) const
    {
        ptr->dispatch(t);
    }

    // friend void swap(scene& lhs, scene& rhs) { using std::swap; swap(lhs.ptr,rhs.ptr); }
};
}
#endif