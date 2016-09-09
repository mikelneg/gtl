#ifndef UHWLFWBVGG_GTL_FILE_UTILITIES_H_
#define UHWLFWBVGG_GTL_FILE_UTILITIES_H_

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)                              
    
    namespace gtl::file::
    
    file utilities
-----------------------------------------------------------------------------*/

#include <gtl/allocators.h>
#include <gtl/tags.h>
#include <vector>

namespace gtl {
namespace file {

    using blob_type = std::vector<char>;
    using efficient_blob_type = std::vector<char, ::gtl::allocators::no_trivial_value_initialization<char> >;

    blob_type get_file_blob(std::string full_path_filename);
    efficient_blob_type get_file_blob(tags::no_initialization, std::string full_path_filename);

    blob_type get_file_blob(std::wstring full_path_filename);
    efficient_blob_type get_file_blob(tags::no_initialization, std::wstring full_path_filename);
}
} // namespaces
#endif
