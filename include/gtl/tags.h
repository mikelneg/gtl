#ifndef KSAZDFFSSA_GTL_TAGS_H_
#define KSAZDFFSSA_GTL_TAGS_H_

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)                              
    
    namespace gtl::tags::    
    useful tag types
-----------------------------------------------------------------------------*/

namespace gtl {
namespace tags {
    
    template <typename T>
    struct construct {
        // TODO revisit this; used to prevent copy initialization from empty lists
        // e.g., prevents void blah(construct<T>); from being called with blah({});
        constexpr explicit construct() noexcept {} 
    };

    struct no_initialization {};
    struct uninitialized {};
    struct debug {};
    struct release {};

    struct contiguous {};
    struct discontiguous {};

    struct xml_format {};

}} // namespaces
#endif
