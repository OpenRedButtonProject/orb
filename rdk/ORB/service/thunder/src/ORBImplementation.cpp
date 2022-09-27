#include "ORBImplementation.h"
#include "ORBLogging.h"

#define ORB_MAJOR_VERSION 1
#define ORB_MINOR_VERSION 0


namespace WPEFramework {
namespace Plugin {

SERVICE_REGISTRATION(ORBImplementation, ORB_MAJOR_VERSION, ORB_MINOR_VERSION);


ORBImplementation::ORBImplementation() : _adminLock()
{
   fprintf(stderr, "Orb implementation constructor\n");
   _orbEventListener = std::make_shared<ORBEventListenerImpl>();
}

ORBImplementation::~ORBImplementation()
{
   fprintf(stderr, "Orb implementation destr\n");
}


void ORBImplementation::Register(Exchange::IORB::INotification* sink)
{
   _adminLock.Lock();
   fprintf(stderr, "Hello from Register ORB %d\n", getpid());
   ORBEngine::GetSharedInstance().Start(_orbEventListener);

   // // Make sure a sink is not registered multiple times.
   // ASSERT(std::find(_notificationClients.begin(), _notificationClients.end(), sink) == _notificationClients.end());
   
   // _notificationClients.push_back(sink);
   // sink->AddRef();
   
   _adminLock.Unlock();

//   TRACE_L1("Registered a sink on the ORB %p", sink);
}

void ORBImplementation::Unregister(Exchange::IORB::INotification* sink)
{
   _adminLock.Lock();
   fprintf(stderr, "Hello from UNRegister ORB %d\n", getpid());
   // std::list<Exchange::IORB::INotification*>::iterator index(std::find(_notificationClients.begin(), _notificationClients.end(), sink));

   // // Make sure you do not unregister something you did not register !!!
   // ASSERT(index != _notificationClients.end());

   // if (index != _notificationClients.end()) {
   //    (*index)->Release();
   //    _notificationClients.erase(index);
   //    TRACE_L1("Unregistered a sink on the ORB %p", sink);
   // }

   _adminLock.Unlock();
}

std::string ORBImplementation::ExecuteBridgeRequest(std::string request)
{
   ORB_LOG_NO_ARGS();
   return ORBEngine::GetSharedInstance().ExecuteBridgeRequest(request);
}

std::string ORBImplementation::CreateToken(std::string uri)
{
   ORB_LOG_NO_ARGS();
   return ORBEngine::GetSharedInstance().CreateToken(uri);
}

}
}