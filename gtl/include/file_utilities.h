#ifndef UHWLFWBVGG_GTL_FILE_UTILITIES_H_
#define UHWLFWBVGG_GTL_FILE_UTILITIES_H_

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)                              
    
    namespace gtl::file::
    
    file utilities
-----------------------------------------------------------------------------*/

#include <vector>
#include <gtl/include/allocators.h>
#include <gtl/include/tags.h>

namespace gtl {
namespace file {
    
    using tags::no_initialization;

    using blob_type           = std::vector<char>;
    using efficient_blob_type = std::vector<char,::gtl::allocators::no_trivial_value_initialization<char>>; 

    blob_type get_file_blob(std::string const& full_path_filename);
    efficient_blob_type get_file_blob(no_initialization, std::string const& full_path_filename);    
    
}} // namespaces                    
#endif
