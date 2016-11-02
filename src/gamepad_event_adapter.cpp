/*-------------------------------------------------------------

Copyright (c) 2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/

#include "gtl/gamepad_event_adapter.h"

#include <gtl/events.h>
#include <memory>
#include <vector>
#include <limits>

#include <GamePad.h>

#include <Eigen/Core>

namespace gtl {
namespace win {

    gamepad_event_adapter::gamepad_event_adapter() : thumbstick_state_{}, gamepad{std::make_unique<DirectX::GamePad>()}
    {
        button_state.Reset();
    }

    void gamepad_event_adapter::append_state_events(std::vector<gtl::event>& event_queue)
    {
        auto state = gamepad->GetState(0);

        // button_state.Update(state);

        auto const append = [&](gtl::event e) { event_queue.emplace_back(e); };

        auto const diff = [](float f, float g) { return std::fabs(f - g) > std::numeric_limits<float>::epsilon(); };

        if (diff(state.thumbSticks.leftX, thumbstick_state_.left_x) || diff(state.thumbSticks.leftY, thumbstick_state_.left_y))
        {
            thumbstick_state_.left_x = state.thumbSticks.leftX;
            thumbstick_state_.left_y = state.thumbSticks.leftY;
            append(gtl::events::dpad_pressed{thumbstick_state_.left_x, thumbstick_state_.left_y});
        }

        // Eigen::Vector2f dpad_vec{state.thumbSticks.leftX, state.thumbSticks.leftY};

        // bool dpad_pressed = dpad_vec.squaredNorm() > 0.0f;

        // dpad_vec =

        // if (state.IsDPadDownPressed()) { dpad_pressed = true; dpad_vec += Eigen::Vector2f{0.0f,-1.0f}; }
        // if (state.IsDPadUpPressed()) { dpad_pressed = true; dpad_vec += Eigen::Vector2f{0.0f,1.0f}; }
        // if (state.IsDPadLeftPressed()) { dpad_pressed = true; dpad_vec += Eigen::Vector2f{-1.0f,0.0f}; }
        // if (state.IsDPadRightPressed()) { dpad_pressed = true; dpad_vec += Eigen::Vector2f{1.0f,0.0f}; }

        // if (state.IsLeftThumbStickDown()) { dpad_pressed = true; dpad_vec += Eigen::Vector2f{0.0f,-1.0f}; }
        // if (state.IsLeftThumbStickUp()) { dpad_pressed = true; dpad_vec += Eigen::Vector2f{0.0f,1.0f}; }
        // if (state.IsLeftThumbStickLeft()) { dpad_pressed = true; dpad_vec += Eigen::Vector2f{-1.0f,0.0f}; }
        // if (state.IsLeftThumbStickRight()) { dpad_pressed = true; dpad_vec += Eigen::Vector2f{1.0f,0.0f}; }

        // if (dpad_pressed)
        //    append(gtl::events::dpad_pressed{dpad_vec.x() * 0.01f, dpad_vec.y() * 0.01f});
    }
}
} // namespaces