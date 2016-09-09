#include "gtl/gamepad_event_adapter.h"

#include <gtl/events.h>
#include <memory>
#include <vector>

#include <GamePad.h>

#include <Eigen/Core>

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)    
-----------------------------------------------------------------------------*/

namespace gtl {
namespace win {

    gamepad_event_adapter::gamepad_event_adapter()
        : gamepad{ std::make_unique<DirectX::GamePad>() }
    {
        button_state.Reset();
    }

    void gamepad_event_adapter::append_state_events(std::vector<gtl::event>& event_queue)
    {
        auto state = gamepad->GetState(0);

        //button_state.Update(state);

        auto const append = [&](gtl::event e) { event_queue.emplace_back(e); };

        Eigen::Vector2f dpad_vec{ state.thumbSticks.leftX, state.thumbSticks.leftY };
        bool dpad_pressed = dpad_vec.squaredNorm() > 0.0f;

        //dpad_vec =

        //if (state.IsDPadDownPressed()) { dpad_pressed = true; dpad_vec += Eigen::Vector2f{0.0f,-1.0f}; }
        //if (state.IsDPadUpPressed()) { dpad_pressed = true; dpad_vec += Eigen::Vector2f{0.0f,1.0f}; }
        //if (state.IsDPadLeftPressed()) { dpad_pressed = true; dpad_vec += Eigen::Vector2f{-1.0f,0.0f}; }
        //if (state.IsDPadRightPressed()) { dpad_pressed = true; dpad_vec += Eigen::Vector2f{1.0f,0.0f}; }

        //if (state.IsLeftThumbStickDown()) { dpad_pressed = true; dpad_vec += Eigen::Vector2f{0.0f,-1.0f}; }
        //if (state.IsLeftThumbStickUp()) { dpad_pressed = true; dpad_vec += Eigen::Vector2f{0.0f,1.0f}; }
        //if (state.IsLeftThumbStickLeft()) { dpad_pressed = true; dpad_vec += Eigen::Vector2f{-1.0f,0.0f}; }
        //if (state.IsLeftThumbStickRight()) { dpad_pressed = true; dpad_vec += Eigen::Vector2f{1.0f,0.0f}; }

        if (dpad_pressed)
            append(gtl::events::dpad_pressed{ dpad_vec.x() * 0.01f, dpad_vec.y() * 0.01f });
    }
}
} // namespaces