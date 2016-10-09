/*-------------------------------------------------------------

Copyright (c) 2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/

#ifndef UHGIWUAHBASFEF_GTL_PHYSICS_GENERATOR_INTERFACE_H_
#define UHGIWUAHBASFEF_GTL_PHYSICS_GENERATOR_INTERFACE_H_

/*-------------------------------------------------------------

---------------------------------------------------------------*/

// forward
class b2World;  // 

namespace gtl {
namespace physics {        

    class generator {
    public:
        virtual void apply(b2World&) const {}
        virtual ~generator() {}
    };

}} // namespaces
#endif
