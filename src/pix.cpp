/*-------------------------------------------------------------

Copyright (c) 2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/

#include "gtl/pix.h"

namespace gtl {
namespace pix {

    inline namespace pix_units_v0 {
        em operator"" _em(unsigned long long int v)
        {
            return em{static_cast<em::type>(v)};
        }
        px operator"" _px(unsigned long long int v)
        {
            return px{static_cast<px::type>(v)};
        }
        pt operator"" _pt(unsigned long long int v)
        {
            return pt{static_cast<pt::type>(v)};
        }
        lpx operator"" _lpx(unsigned long long int v)
        {
            return lpx{static_cast<lpx::type>(v)};
        }
        ppi operator"" _ppi(unsigned long long int v)
        {
            return ppi{static_cast<ppi::type>(v)};
        }
        inches operator"" _inches(long double v)
        {
            return inches{static_cast<inches::type>(v)};
        }
        inches operator"" _inches(unsigned long long int v)
        {
            return inches{static_cast<inches::type>(v)};
        }
        double operator"" _to_1(long double v)
        {
            return static_cast<double>(v / 1);
        }

        // inches --> px              [ 4' := 4 * ppi]
        // px --> inches              [ 640 := 640 / ppi]

        // lpx --> px                 [ 640 := 640 * (actual_ppi / base_ppi)]
        // px --> lpx                 [ 640 := 640 * (base_ppi / actual_ppi)]

        // pt --> px                     3/4 * pt
        // px --> pt                     4/3 * px

        template <>
        px convert_to(pt pt_)
        {
            return px{static_cast<px::type>(value(pt_) * 0.75)};
        }

        template <>
        pt convert_to(px px_)
        {
            return pt{static_cast<pt::type>(value(px_) * (4.0 / 3.0))};
        }

        template <>
        px convert_to(inches inches_, ppi actual_ppi_)
        {
            return px{static_cast<px::type>(value(inches_) * value(actual_ppi_))};
        }

        template <>
        inches convert_to(px px_, ppi actual_ppi_)
        {
            return inches{static_cast<inches::type>(value(px_) / value(actual_ppi_))};
        }

        template <>
        px convert_to(lpx lpx_, ppi actual_ppi_, ppi base_ppi_)
        {
            double ppi_ratio = value(actual_ppi_) / static_cast<double>(value(base_ppi_));
            return px{static_cast<px::type>(value(lpx_) * ppi_ratio)};
        }

        template <>
        lpx convert_to(px px_, ppi actual_ppi_, ppi base_ppi_)
        {
            double inv_ppi_ratio = static_cast<double>(value(base_ppi_)) / value(actual_ppi_);
            return lpx{static_cast<lpx::type>(value(px_) * inv_ppi_ratio)};
        }

        template pt convert_to(px);
        template px convert_to(pt);

        template px convert_to(inches, ppi);
        template inches convert_to(px, ppi);

        template px convert_to(lpx, ppi actual, ppi base);
        template lpx convert_to(px, ppi actual, ppi base);
    }
}
} // namespace