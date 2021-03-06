/*-------------------------------------------------------------

Copyright (c) 2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/

#include "gtl/file_utilities.h"

#include <gtl/allocators.h>
#include <gtl/tags.h>

#include <fstream>
#include <ios>
#include <stdexcept>
#include <string>
#include <vector>

namespace gtl {
namespace file {

    template <typename R, typename S>
    static inline R get_file_blob_helper(S const& full_path_filename)
    {
        std::ifstream file_{full_path_filename, std::ios::in | std::ios::binary};
        if (!file_)
        {
            throw std::runtime_error{__func__};
        }

        file_.seekg(0, std::ios::end);
        std::streamsize size_ = file_.tellg();
        file_.seekg(0, std::ios::beg);

        R buffer_(size_);
        if (!file_.read(buffer_.data(), size_))
        {
            throw std::runtime_error{__func__};
        }

        return buffer_;
    }

    std::vector<char> get_file_blob(std::string full_path_filename)
    {
        return get_file_blob_helper<std::vector<char>>(full_path_filename);
    }

    std::vector<char, gtl::allocators::no_trivial_init<char>> get_file_blob(gtl::tags::no_initialization, std::string full_path_filename)
    {
        return get_file_blob_helper<std::vector<char, gtl::allocators::no_trivial_init<char>>>(full_path_filename);
    }

    std::vector<char> get_file_blob(std::wstring full_path_filename)
    {
        return get_file_blob_helper<std::vector<char>>(full_path_filename);
    }

    std::vector<char, gtl::allocators::no_trivial_init<char>> get_file_blob(gtl::tags::no_initialization, std::wstring full_path_filename)
    {
        return get_file_blob_helper<std::vector<char, gtl::allocators::no_trivial_init<char>>>(full_path_filename);
    }
}
} // namespaces
