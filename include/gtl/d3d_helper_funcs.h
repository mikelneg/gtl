/*-------------------------------------------------------------

Copyright (c) 2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/

#ifndef IRWFGHOSFF_GTL_d3d_helper_funcs_H_
#define IRWFGHOSFF_GTL_d3d_helper_funcs_H_

#include <gtl/d3d_types.h>
#include <gtl/d3d_version.h>
#include <gtl/intrusive_ptr.h>
#include <gtl/tags.h>
#include <gtl/win_tools.h>

#include <ostream>
#include <vector>

namespace gtl {
namespace d3d {
    namespace version_12_0 {

        inline int frame_count() noexcept
        {
            return 3;
        } // HACK decentralize this..

        std::vector<raw::AdapterDesc> enumerate_adaptors();

        void report_live_objects(device&);

        void wait_for_gpu(device&, command_queue&);
        void wait_for_gpu(command_queue&);

        void initialize_null_descriptor_srv(device&,raw::CpuDescriptorHandle);
        void initialize_null_descriptor_uav(device&,raw::CpuDescriptorHandle);

        template <typename T>
        device get_device_from(T& t)
        {
            device dev{gtl::tags::uninitialized{}};
            win::throw_on_fail(t->GetDevice(__uuidof(device::type), expose_as_void_pp(dev)), __func__);
            return dev;
        }

        template <typename T>
        device get_device_from(T* t)
        {
            device dev{gtl::tags::uninitialized{}};
            win::throw_on_fail(t->GetDevice(__uuidof(device::type), expose_as_void_pp(dev)), __func__);
            return dev;
        }

        template <typename T>
        device get_device(T& t)
        {
            device dev{gtl::tags::uninitialized{}};
            win::throw_on_fail(t->GetDevice(__uuidof(device::type), expose_as_void_pp(dev)), __func__);
            return dev;
        }
    }
}
} // namespaces
#endif
