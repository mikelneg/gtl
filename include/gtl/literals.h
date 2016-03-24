#ifndef TYWZXCWSW_GTL_LITERALS_H_
#define TYWZXCWSW_GTL_LITERALS_H_

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)                              
    
    Some user defined literals..
-----------------------------------------------------------------------------*/

namespace gtl {

    //namespace detail {    
    //    template <typename T>
    //    struct value_type { 
    //        const T value_; 
    //        constexpr explicit value_type(T value_type) noexcept : value_{value_type} {}
    //        constexpr explicit operator T() const { return value_; }
    //    };
    //}
    
    //namespace units {
    //    struct em : detail::value_type<int> { using detail::value_type<int>::value_type; };
    //    struct px : detail::value_type<int> { using detail::value_type<int>::value_type; };
    //    struct dpi : detail::value_type<int> { using detail::value_type<int>::value_type; };
    //    struct lpx : detail::value_type<int> { using detail::value_type<int>::value_type; };        
    //    struct inch : detail::value_type<double> { using detail::value_type<double>::value_type; };
    //}
    //
    //inline namespace literals {
    //    inline units::em operator "" _em(unsigned long long int value_type) { return units::em{static_cast<int>(value_type)}; }
    //    inline units::px operator "" _px(unsigned long long int value_type) { return units::px{static_cast<int>(value_type)}; }
    //    inline units::dpi operator "" _dpi(unsigned long long int value_type) { return units::dpi{static_cast<int>(value_type)}; }
    //    inline units::lpx operator "" _lpx(unsigned long long int value_type) { return units::lpx{static_cast<int>(value_type)}; }        
    //    inline units::inch operator "" _inch(long double value_type) { return units::inch{static_cast<double>(value_type)}; }        
    //    inline double operator "" _to_1(long double value_type) { return static_cast<double>(value_type / 1); }
    //}    

} // namespace
#endif
