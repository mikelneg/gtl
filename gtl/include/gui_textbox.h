#ifndef NOWOAWAAAAWA_GTL_GUI_TEXTBOX_H_
#define NOWOAWAAAAWA_GTL_GUI_TEXTBOX_H_

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)                              
    
    namespace gtl::gui
    class textbox;
-----------------------------------------------------------------------------*/

#include <string>

namespace gtl {
namespace gui {

    

    class textbox {
        std::string text_;

    public:
        textbox(unsigned x, unsigned y, unsigned w, unsigned h, std::string&& text);

    };
    

}} // namespaces
#endif
