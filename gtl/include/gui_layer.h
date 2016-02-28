#ifndef NVWIOAFFWWWF_GTL_GUI_LAYER_H_
#define NVWIOAFFWWWF_GTL_GUI_LAYER_H_

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)                              
    
    namespace gtl::gui
    class gui_layer;
-----------------------------------------------------------------------------*/

#include <vector>

namespace gtl {
namespace gui {

    class gui_element;

    class gui_layer {
        
        std::vector<gui_element> elements_;
                
    
    public:
        gui_layer() = default;
        gui_layer(gui_layer&&) = default;
        gui_layer& operator=(gui_layer&&) = default;

        
        void operator()();



    };


}} // namespaces
#endif
