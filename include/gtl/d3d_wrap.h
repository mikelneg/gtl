#ifndef YWWIOAASWAFAF_GTL_D3D_WRAP_H_
#define YWWIOAASWAFAF_GTL_D3D_WRAP_H_

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)                                  
    
    class gtl::d3d

    wrapper types for types that need wrapping

-----------------------------------------------------------------------------*/

//#include <gtl/d3d_types.h>

namespace gtl {
namespace d3d_wrap {  

    class rtv_descriptor_heap {
            
        descriptor_heap d;
        unsigned count;
        bool contiguous_descriptors;

    public:

        rtv_descriptor_heap() 
            : d{rtv_only,...}
        { 

        }

        template <typename CList>            
        friend 
        void bind(CList& c, rtv_descriptor_heap const& rtv) {
            c.OMSetRenderTargets(count, rtv.d.handle(), contiguous_descriptors, nullptr); 
        }

        template <typename CList, typename DSHandle>            
        friend
        void bind(CList& c, rtv_descriptor_heap const& rtv, DSHandle const& ds) {
            c.OMSetRenderTargets(count, rtv.d.handle(), contiguous_descriptors, std::addressof(ds)); 
        }

    };


    template <typename...Ts>
    class root_signature {

        other_rootsignature o_rs; 

    public:
                
        template <typename CList>
        friend
        auto bind_root_signature(CList& cl, root_signature const& rs) {                        
            
            return [&](Ts...ts){ 
                bind(cl,rs.o_rs);                                           
                using discard = int[];
                discard{0,(bind(cl,ts,vn::index_of<Ts>::value),0)...};                
            };
        }


    };

     

}} // namespaces
#endif