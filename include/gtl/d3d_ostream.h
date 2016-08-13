#ifndef BNOWOAIBOASLDFF_GTL_D3D_OSTREAM_H_
#define BNOWOAIBOASLDFF_GTL_D3D_OSTREAM_H_

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)                                      
    
    namespace gtl::d3d::
        
    std::ostream formatting for various d3d types
-----------------------------------------------------------------------------*/

#include <gtl/d3d_version.h>
#include <ostream>

namespace gtl {    
namespace d3d {      

    inline std::ostream& operator<<(std::ostream& str, DXGI_ADAPTER_DESC1 const& obj) {        
        str << "DXGI_ADAPTER_DESC1:\n" \
            << "Description: " << obj.Description << "\n" 
            << "VendorId: " << obj.VendorId << "\n" \
            << "DeviceId: " << obj.DeviceId << "\n" \
            << "SubSysId: " << obj.SubSysId << "\n" \
            << "Revision: " << obj.Revision << "\n" \
            << "DedicatedVideoMemory: " << obj.DedicatedVideoMemory << "\n" \
            << "DedicatedSystemMemory: " << obj.DedicatedSystemMemory << "\n" \
            << "SharedSystemMemory: " << obj.SharedSystemMemory << "\n" \
            << "AdapterLuid: {HighPart, LowPart} = " << obj.AdapterLuid.HighPart << "," << obj.AdapterLuid.LowPart << "\n" \
            << "Flags: " << obj.Flags << "\n"; 
        return str; 
    }  

}} // namespaces
#endif
