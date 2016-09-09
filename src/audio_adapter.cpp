#include "gtl/audio_adapter.h"

#include <exception>
#include <memory>

#include <Audio.h>

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)    
-----------------------------------------------------------------------------*/

namespace gtl {
namespace win {

    struct audio_adapter::priv_impl {
        DirectX::AudioEngine audio_engine;
        std::unordered_map<std::string, DirectX::SoundEffect> sound_effects;
        priv_impl(DirectX::AUDIO_ENGINE_FLAGS flags)
            : audio_engine{ flags }
        {
        }
    };

    audio_adapter::audio_adapter()
        : audio_engine{ std::make_unique<priv_impl>(DirectX::AudioEngine_Default) }
    {
        auto result = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        uninitialize_in_dtor = (result == S_OK || result == S_FALSE);
    }

    audio_adapter::audio_adapter(gtl::tags::debug)
        : audio_engine{ std::make_unique<priv_impl>(DirectX::AudioEngine_Default | DirectX::AudioEngine_Debug) }
    {
        auto result = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        uninitialize_in_dtor = (result == S_OK || result == S_FALSE);
    }

    audio_adapter::~audio_adapter()
    {
        if (uninitialize_in_dtor) {
            CoUninitialize();
        }
    }

    audio_adapter::audio_adapter(audio_adapter&& other)
        : uninitialize_in_dtor{ other.uninitialize_in_dtor }
    {
        other.uninitialize_in_dtor = false;
    }

    void audio_adapter::add_effect(std::string name, std::wstring wav_file)
    {
        if (audio_engine->sound_effects.count(name) == 0) {
            audio_engine->sound_effects.emplace(name, DirectX::SoundEffect{ std::addressof(audio_engine->audio_engine), wav_file.c_str() });
        }
    }

    void audio_adapter::play_effect(std::string name) // const
    {
        if (audio_engine->sound_effects.count(name) > 0) {
            (audio_engine->sound_effects).at(name).Play(); // For some reason this isn't const..
        }
    }

    void audio_adapter::update()
    {
        if (!audio_engine->audio_engine.Update())
            throw std::runtime_error{ "Audio device not available." };
    }
}
} // namespace