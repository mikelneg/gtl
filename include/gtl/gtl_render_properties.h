#ifndef BIWOAISAVVZXFAWW_GTL_GTL_RENDER_PROPERTIES_H_
#define BIWOAISAVVZXFAWW_GTL_GTL_RENDER_PROPERTIES_H_

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)                              
        
    namespace gtl::gui::
    
-----------------------------------------------------------------------------*/

#include <string> 

#include <boost/variant.hpp>
#include <vn/boost_variant_utilities.h>
#include <vn/boost_visitors.h>

namespace gtl {
namespace render {

    inline namespace properties {                                    
        struct empty {};
        struct shape { int id; };
        struct texture { int id; };
        struct animation { int id; };
        struct transform { float x,y,z,w; };
    }                

    using render_property_base_ = boost::variant<empty,shape,texture,animation,transform
                                                 >;
    class render_property : public render_property_base_ {                

    public:   
        template <typename ...Args>
        render_property(Args&&...args) noexcept(noexcept(render_property_base_(std::forward<Args>(args)...)))
            : render_property_base_(std::forward<Args>(args)...) {}
                
        friend bool same_type(render_property const& lhs, render_property const& rhs) {
            using boost::apply_visitor;
            return apply_visitor(vn::visitors::same_type{},lhs,rhs); 
        }

        friend bool operator==(render_property const& lhs, render_property const& rhs) {
            using boost::apply_visitor;
            return apply_visitor(vn::visitors::weak_equality{},lhs,rhs);
        }

        template <typename T>
        friend bool has_variant_type(render_property const& e) { 
            using boost::apply_visitor;
            return apply_visitor(vn::visitors::has_variant_type<T>{},e);
        }
    };



}} // namespace
#endif
