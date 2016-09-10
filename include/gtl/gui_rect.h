/*-------------------------------------------------------------

Copyright (c) 2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/

#ifndef BJJOWAASF_GTL_GUI_RECT_H_
#define BJJOWAASF_GTL_GUI_RECT_H_

namespace gtl {
namespace gui {

    class rect {
        int upper_left_x_, upper_left_y_;
        unsigned width_, height_;

    public:
        rect(int x, int y, unsigned w, unsigned h);
    };
}
} // namespaces
#endif
