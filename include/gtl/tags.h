#ifndef KSAZDFFSSA_GTL_TAGS_H_
#define KSAZDFFSSA_GTL_TAGS_H_

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)                              
    
    namespace gtl::tags::    
    useful tag types
-----------------------------------------------------------------------------*/

namespace gtl {
namespace tags {

    struct no_initialization {};
    struct uninitialized {};
    struct debug {};
    struct release {};

    struct contiguous {};
    struct discontiguous {};

    struct xml_format {};

}} // namespaces
#endif
