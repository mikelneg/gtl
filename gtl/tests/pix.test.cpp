#include "catch.hpp"

#include <iostream>
#include <gtl/include/pix.h>

#include <cstddef>
#include <type_traits>

TEST_CASE("gtl::pix","[gtl::pix]") {                    
    SECTION("basic tests for gtl::pix conversions")
    {
        gtl::pix::lpx lpx{1920};
        gtl::pix::ppi base_ppi{96};
        gtl::pix::ppi no_scale_ppi{96};
        gtl::pix::ppi dense_ppi{96 * 2};
        gtl::pix::ppi sparse_ppi{96 / 2};

        using gtl::pix::convert_to;
        using namespace gtl::pix::literals;

        CHECK((  value(convert_to<gtl::pix::px>(1920_lpx, no_scale_ppi, base_ppi)) == Approx(1920).epsilon(1)      ));
        CHECK((  value(convert_to<gtl::pix::px>(1920_lpx, dense_ppi, base_ppi))    == Approx(1920 * 2).epsilon(1)  ));
        CHECK((  value(convert_to<gtl::pix::px>(1920_lpx, sparse_ppi, base_ppi))   == Approx(1920 / 2).epsilon(1)  ));

        CHECK((  value(convert_to<gtl::pix::px>(4.0_inches, 96_ppi)) == Approx(4 * 96).epsilon(1)    ));
        CHECK((  value(convert_to<gtl::pix::px>(4_inches, 96_ppi))   == Approx(4 * 96).epsilon(1)    ));        


        CHECK((  value(convert_to<gtl::pix::px>(1_pt)) == Approx(0.75).epsilon(0.0001)     ));
        CHECK((  value(convert_to<gtl::pix::pt>(1_px)) == Approx(4 / 3.0).epsilon(0.0001)     ));

        CHECK((  value(convert_to<gtl::pix::px>(72_pt)) == Approx(96).epsilon(1)     ));
        CHECK((  value(convert_to<gtl::pix::inches>(96_px, 96_ppi)) == Approx(1).epsilon(0.001)    ));
        
        std::cout << std::setprecision(32) << value(convert_to<gtl::pix::pt>(96_px)) << "\n";
        std::cout << std::setprecision(32) << value(convert_to<gtl::pix::px>(72_pt)) << "\n";
        std::cout << std::setprecision(32) << value(convert_to<gtl::pix::inches>(96_px, 96_ppi)) << "\n";
        std::cout << std::setprecision(32) << value(convert_to<gtl::pix::inches>(convert_to<gtl::pix::px>(72_pt), 96_ppi)) << "\n";
        //CHECK((  value(convert_to<gtl::pix::inches>(convert_to<gtl::pix::px>(72_pt), 96_ppi)) == Approx(1).epsilon(0.1)    ));
    }
}
