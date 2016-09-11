#ifndef OWIAOIBHOIAHWEFOIJ_GTL_SCENES_INTRO_SCENE_H_
#define OWIAOIBHOIAHWEFOIJ_GTL_SCENES_INTRO_SCENE_H_

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)

    namespace gtl::scenes

    class intro_scene;
-----------------------------------------------------------------------------*/

#include <gtl/include/events.h>
#include <gtl/include/keyboard_enum.h>

#include <iostream>
#include <memory>

namespace gtl {
namespace scenes {

    class A {
    public:
        A()
        {
            std::cout << "A()\n";
        }
        A(A const&)
        {
            std::cout << "A(A const&)\n";
        }
        ~A()
        {
            std::cout << "~A()\n";
        }
    };

    class intro_scene {

        std::unique_ptr<A> ptr;

    public:
        intro_scene() : ptr{std::make_unique<A>()}
        {
        }
        intro_scene(intro_scene&&) = default;
        // intro_scene(intro_scene const&) = default;
        intro_scene& operator=(intro_scene&&) = default;

        std::vector<ID3D12GraphicsCommandList*> draw(int, float f) const
        {
            std::cout << std::fixed << "intro_scene(" << f << ")";
            return {};
        }

        template <typename YieldType>
        gtl::event handle_events(YieldType& yield) const
        {
            namespace ev = gtl::events;
            int count{};
            while (!same_type(yield().get(), ev::exit_immediately{}))
            {
                if (same_type(yield.get(), ev::keydown{}))
                {
                    if (boost::get<ev::keydown>(yield.get().value()).key == gtl::keyboard::Q)
                    {
                        std::cout << "intro_scene(): q pressed, exiting A from route 0 (none == " << count << ")\n";
                        return gtl::events::exit_state{0};
                    }
                    else if (boost::get<ev::keydown>(yield.get().value()).key == gtl::keyboard::K)
                    {
                        std::cout << "intro_scene(): k pressed, exiting A from route 1 (none == " << count << ")\n";
                        throw std::runtime_error{"throw from intro_scene handle_events()"};
                        return gtl::events::exit_state{1};
                    }
                    else
                    {
                        std::cout << "intro_scene(): some key pressed..\n";
                    }
                }
                else if (same_type(yield.get(), ev::none{}))
                {
                    count++;
                }
            }
            return gtl::events::exit_state{0};
        }
    };

    //  struct state_A {
    //
    //    std::string message{"I'm an A!"};
    //    std::unique_ptr<A> a;
    //
    //    state_A() : a{std::make_unique<A>()} {}
    //    state_A(state_A&&) = default;
    //    state_A& operator=(state_A&&) = default;
    //
    //    int operator()(evco::pull_type& yield) const {
    //        int count{};
    //        while (!same_type(yield().get(),ev::sendquit{})){
    //            if (same_type(yield.get(),ev::keydown{})){
    //                if ( boost::get<ev::keydown>( yield.get().value_ ).key == gtl::keyboard::Q) {
    //                    std::cout << "A:: q pressed, exiting A from route 0...\n";
    //                    std::cout << "leaving A with none-count == " << count << "\n";
    //                    return 1;
    //                } else
    //                if ( boost::get<ev::keydown>( yield.get().value_ ).key == gtl::keyboard::K) {
    //                    std::cout << "A:: k pressed, exiting A from route 1...\n";
    //                    std::cout << "leaving A with none-count == " << count << "\n";
    //                    return 2;
    //                } else { std::cout << "A:: key pressed..\n"; }
    //            } else if (same_type(yield.get(),ev::none{})) {
    //                count++;
    //             }
    //        }
    //        std::cout << "leaving A with none-count == " << count << "\n";
    //        return 0;
    //    }
    //
    //    void draw(std::string s, float f = 1.0f) const {
    //        std::cout << "(" << s << "," << message << "," << f << ")\n";
    //    }
    //};
}
} // namespaces
#endif
