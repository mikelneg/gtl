/*-------------------------------------------------------------

Copyright (c) 2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/

#ifndef BNOWOAIBOASLDFF_GTL_D3D_OSTREAM_H_
#define BNOWOAIBOASLDFF_GTL_D3D_OSTREAM_H_

#include <gtl/d3d_version.h>
#include <ostream>

namespace gtl {
namespace d3d {

    inline std::ostream& operator<<(std::ostream& str, DXGI_ADAPTER_DESC1 const& obj)
    {
        str << "DXGI_ADAPTER_DESC1:\n"
            << "Description: " << obj.Description << "\n"
            << "VendorId: " << obj.VendorId << "\n"
            << "DeviceId: " << obj.DeviceId << "\n"
            << "SubSysId: " << obj.SubSysId << "\n"
            << "Revision: " << obj.Revision << "\n"
            << "DedicatedVideoMemory: " << obj.DedicatedVideoMemory << "\n"
            << "DedicatedSystemMemory: " << obj.DedicatedSystemMemory << "\n"
            << "SharedSystemMemory: " << obj.SharedSystemMemory << "\n"
            << "AdapterLuid: {HighPart, LowPart} = " << obj.AdapterLuid.HighPart << "," << obj.AdapterLuid.LowPart
            << "\n"
            << "Flags: " << obj.Flags << "\n";
        return str;
    }
}
} // namespaces
#endif
