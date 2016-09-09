#ifndef WLSFBWIFW_GTL_PIX_H_
#define WLSFBWIFW_GTL_PIX_H_

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)                              
    
    namespace gtl::pix
    
        + not being used -- might revisit..

-----------------------------------------------------------------------------*/

#include "literals.h"

#include <tuple>
#include <utility>

namespace gtl {
namespace pix {

    namespace detail {
        template <typename T>
        class value_base {
            const T value;

        public:
            using type = T;
            constexpr explicit value_base(T const& v) noexcept : value{ v } {}
            constexpr explicit operator T() const noexcept { return value; }
            friend constexpr T value(value_base const& t) noexcept { return t.operator T(); }
        };
    }

    inline namespace pix_units_v0 {

        struct em : detail::value_base<int> {
            using detail::value_base<int>::value_base;
        };
        struct px : detail::value_base<int> {
            using detail::value_base<int>::value_base;
        };
        struct pt : detail::value_base<int> {
            using detail::value_base<int>::value_base;
        };
        struct ppi : detail::value_base<int> {
            using detail::value_base<int>::value_base;
        };
        struct lpx : detail::value_base<int> {
            using detail::value_base<int>::value_base;
        };
        struct inches : detail::value_base<int> {
            using detail::value_base<int>::value_base;
        };

        em operator"" _em(unsigned long long int);
        px operator"" _px(unsigned long long int);
        pt operator"" _pt(unsigned long long int);
        ppi operator"" _ppi(unsigned long long int);
        lpx operator"" _lpx(unsigned long long int);
        inches operator"" _inches(long double);
        inches operator"" _inches(unsigned long long int);

        template <typename T, typename... Args>
        T convert_to(Args...);

        extern template px convert_to(inches, ppi);
        extern template inches convert_to(px, ppi);

        extern template pt convert_to(px);
        extern template px convert_to(pt);

        extern template lpx convert_to(px, ppi, ppi);
        extern template px convert_to(lpx, ppi, ppi);
    }

    namespace literals {
        using namespace pix_units_v0;
    }
}
} // namespace
#endif
