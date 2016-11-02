/*-------------------------------------------------------------

Copyright (c) 2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/
#include <iostream>

#include <Windows.h>

#include <gtl/d3d_types.h>
#include <gtl/gtl_window.h>
#include <gtl/resource_locator.h>
#include <gtl/stage.h>
#include <gtl/tags.h>

#include <gtl/main_scene.h>
#include <gtl/scene.h>

#include <gtl/audio_adapter.h>
#include <gtl/gamepad_event_adapter.h>

int main(HINSTANCE hinst, HINSTANCE, LPSTR, int)
{
    try
    {
        gtl::resource_locator r{"data/app_settings.xml", gtl::tags::xml_format{}};

        gtl::win::window win{hinst, 960, 540, u8"Main Window"};
        gtl::d3d::device dev{gtl::tags::debug{}, std::cout};
        gtl::d3d::command_queue cqueue{dev};
        gtl::d3d::swap_chain swchain{get_hwnd(win), cqueue, 3};

        gtl::win::audio_adapter audio_adapter; // HACK testing audio
        audio_adapter.add_effect("click", L"data\\sound_effects\\click_x.wav");

        gtl::scenes::main_scene scene_{dev, swchain, cqueue};
        gtl::stage stage{swchain, cqueue, 3, scene_, audio_adapter};

        gtl::d3d::report_live_objects(dev);

        gtl::win::gamepad_event_adapter gamepad;

        win.enter_message_loop([&](auto& event_queue_) {

            audio_adapter.update();
            gamepad.append_state_events(event_queue_);

            // for (auto&& e : event_queue_)
            //{
            stage.dispatch_events(event_queue_);
            //}

            stage.present(swchain);
        });
    }
    catch (std::exception& ex)
    {
        std::cout << "std::exception caught: " << ex.what() << "\n";
    }
    catch (...)
    {
        std::cout << "unknown exception caught..\n";
    }
}
