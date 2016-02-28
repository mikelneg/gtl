#ifndef BJJOWAASF_GTL_GUI_RECT_H_
#define BJJOWAASF_GTL_GUI_RECT_H_

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)                              
    
    namespace gtl::gui
    class rect;
-----------------------------------------------------------------------------*/

namespace gtl {
namespace gui {

    class rect {
        int upper_left_x_, upper_left_y_;
        unsigned width_, height_;
    public:
        rect(int x, int y, unsigned w, unsigned h);


    };
    
}} // namespaces
#endif
