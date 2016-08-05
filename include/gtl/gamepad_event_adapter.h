#ifndef YWIIABZXVVSWF_GTL_GAMEPAD_EVENT_adapter_H_
#define YWIIABZXVVSWF_GTL_GAMEPAD_EVENT_adapter_H_

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)
    
    class gtl::win::gamepad_event_adapter
        - Generates gtl events based on gamepad state
-----------------------------------------------------------------------------*/

#include <vector>
#include <memory>
#include <gtl/events.h>

#include <GamePad.h>

namespace gtl {
namespace win {


class gamepad_event_adapter {    
    std::unique_ptr<DirectX::GamePad> gamepad;
    DirectX::GamePad::ButtonStateTracker button_state;
public:
    gamepad_event_adapter();
    void append_state_events(std::vector<gtl::event>&);
};

                                      

}} // namespace
#endif