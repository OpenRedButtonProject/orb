/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#ifndef __IORB_H
#define __IORB_H

#include "Module.h"

namespace WPEFramework {
namespace Exchange {

    struct EXTERNAL IORB : virtual public Core::IUnknown {
        enum { ID = ID_ORB };

        struct INotification : virtual public Core::IUnknown {
            enum { ID = ID_ORB_NOTIFICATION};

            virtual ~INotification() {}
        };

        virtual ~IORB() {}

        virtual void Register(INotification* sink) = 0;
        virtual void Unregister(INotification* sink) = 0;
    };

} // Exchange
} // WPEFramework

#endif //__IORB_H