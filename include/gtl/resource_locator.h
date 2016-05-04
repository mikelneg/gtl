#ifndef RYWIOOABZZSAWWWFE_GTL_RESOURCE_LOCATOR_H_
#define RYWIOOABZZSAWWWFE_GTL_RESOURCE_LOCATOR_H_

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)                                  
    
    class gtl::resource_locator

-----------------------------------------------------------------------------*/

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <string>
#include <type_traits>
#include <gtl/tags.h>

namespace gtl {
     
    class resource_locator {

        using pt = boost::property_tree::ptree;
        using data_type = pt::data_type;

        pt ptree_;
        std::string filename_;
        
    public:

        resource_locator(std::string filename, gtl::tags::xml_format)
        : ptree_{},
            filename_{std::move(filename)}
        {
            boost::property_tree::read_xml(filename_,ptree_);
        }

        template <typename T, typename ...Ts>
        decltype(auto) get(Ts&&...ts) noexcept(noexcept(ptree_.get<T>(std::forward<Ts>(ts)...)))
        {
            return ptree_.get<T>(std::forward<Ts>(ts)...);
        }

        template <typename P, typename F>
        decltype(auto) get(P&& param, F func) noexcept(noexcept(func(ptree_.get<data_type>(std::forward<P>(param)))))
        {   
            return func(ptree_.get<data_type>(std::forward<P>(param)));
        }
         
        //template <typename ...Ts>
        //decltype(auto) get(Ts&&...ts) noexcept(noexcept(ptree_.get(std::forward<Ts>(ts)...)))
        //{
        //    return ptree_.get(std::forward<Ts>(ts)...);
        //}

        template <typename T, typename...Ts>
        decltype(auto) get_optional(Ts&&...ts) noexcept(noexcept(ptree_.get_optional<T>(std::forward<Ts>(ts)...)))
        {
            return ptree_.get_optional<T>(std::forward<Ts>(ts)...);
        }        

    };

} // namespaces
#endif

