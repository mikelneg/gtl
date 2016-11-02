/*-------------------------------------------------------------

Copyright (c) 2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/

#ifndef YWIIABZXVVSWF_GTL_GAMEPAD_EVENT_adapter_H_
#define YWIIABZXVVSWF_GTL_GAMEPAD_EVENT_adapter_H_

#include <gtl/events.h>
#include <memory>
#include <vector>

#include <GamePad.h>

namespace gtl {
namespace win {

    class gamepad_event_adapter {

        struct thumbstick_state {
            float left_x, left_y;
        } thumbstick_state_;

        std::unique_ptr<DirectX::GamePad> gamepad;
        DirectX::GamePad::ButtonStateTracker button_state;

    public:
        gamepad_event_adapter();
        void append_state_events(std::vector<gtl::event>&);
    };
}
} // namespace
#endif