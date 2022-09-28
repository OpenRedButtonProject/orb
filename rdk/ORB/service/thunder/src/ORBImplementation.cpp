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

/**
 * @brief ORBImplementation::LoadPlatform
 * 
 * Used to dlopen the ORBPlatform library. Called from ORB::Initialize
 */
void ORBImplementation::LoadPlatform()
{
   ORB_LOG_NO_ARGS();
   _adminLock.Lock();
   ORBEngine::GetSharedInstance().Start(_orbEventListener);
   _adminLock.Unlock();
}

/**
 * @brief ORB::ExecuteWpeBridgeRequest
 * Execute the given WPE bridge request. Platform call
 * 
 * @param request The request as a string
 * @return std::string The response as a string
 */
std::string ORBImplementation::ExecuteBridgeRequest(std::string request)
{
   ORB_LOG_NO_ARGS();
   return ORBEngine::GetSharedInstance().ExecuteBridgeRequest(request);
}

/**
 * @brief ORBImplementation::CreateToken
 * 
 * Create a new JSON token for the current application and the given uri.
 * Platform call
 * 
 * @param uri The given URI
 * @return std::string The resulting token
 */
std::string ORBImplementation::CreateToken(std::string uri)
{
   ORB_LOG_NO_ARGS();
   return ORBEngine::GetSharedInstance().CreateToken(uri);
}

/**
 * @brief ORBImplementation::NotifyApplicationLoadFailed
 *
 * Notify the application manager and the current JavaScript context that the specified HbbTV application
 * has failed to load. Platform call.
 *
 * @param  url
 * @param errorDescription
 *
 * @return Core::ERROR_NONE
 */
void ORBImplementation::NotifyApplicationLoadFailed(std::string url, std::string errorDescription)
{
   ORB_LOG_NO_ARGS();
   ORBEngine::GetSharedInstance().NotifyApplicationLoadFailed(url, errorDescription);
}

/**
 * @brief ORBImplementation::NotifyApplicationPageChanged
 * 
 * Notify the application manager that the page of the current HbbTV application has changed
 * and is about to load. Platform call.
 * 
 * @param url 
 */
void ORBImplementation::NotifyApplicationPageChanged(std::string url)
{
   ORB_LOG_NO_ARGS();
   ORBEngine::GetSharedInstance().NotifyApplicationPageChanged(url);
}

/**
 * @brief ORB::LoadDvbUrl
 *
 * Load the specified DVB URL through the DSM-CC implementation. Platform call.
 *
 * @param url The dvb url to be loaded
 * @param requestId The request identifier
 *
 * @return Core::ERROR_NONE
 */
void ORBImplementation::LoadDvbUrl(std::string url, int requestId)
{
   ORB_LOG_NO_ARGS();
   ORBEngine::GetSharedInstance().LoadDvbUrl(url, requestId);
}

/**
 * @brief ORBImplementation::SendKeyEvent
 * 
 * Send the specified key event to the current HbbTV application (if any).
 * Platform call.
 * 
 * @param keyCode 
 * @return true 
 * @return false 
 */
bool ORBImplementation::SendKeyEvent(int keyCode)
{
   ORB_LOG_NO_ARGS();
   return ORBEngine::GetSharedInstance().SendKeyEvent(keyCode);
}

}  // Plugin
}  // WPEFramework