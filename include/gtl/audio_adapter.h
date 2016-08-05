#ifndef WYBOWAFBBASF_GTL_AUDIO_ADAPTER_H_
#define WYBOWAFBBASF_GTL_AUDIO_ADAPTER_H_

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)
    
    class gtl::win::audio_adapter
        
-----------------------------------------------------------------------------*/

#include <memory>
#include <unordered_map>
#include <string>

#include <gtl/events.h>

#include <gtl/tags.h>

namespace gtl {
namespace win {

class audio_adapter {
  
    struct priv_impl;

    std::unique_ptr<priv_impl> audio_engine;    
    bool uninitialize_in_dtor;

public:    
    audio_adapter(audio_adapter&&);
    audio_adapter();
    audio_adapter(gtl::tags::debug);
    ~audio_adapter();

    void update();
    void add_effect(std::string name, std::wstring wav_file);
    void play_effect(std::string name); 

    audio_adapter& operator=(audio_adapter&&) = delete;
};

                                      

}} // namespace
#endif