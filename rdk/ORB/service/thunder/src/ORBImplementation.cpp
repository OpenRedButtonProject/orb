/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#include "ORBImplementation.h"
#include "ORBLogging.h"

#define ORB_MAJOR_VERSION 1
#define ORB_MINOR_VERSION 0


namespace WPEFramework {
namespace Plugin {
SERVICE_REGISTRATION(ORBImplementation, ORB_MAJOR_VERSION, ORB_MINOR_VERSION);


/**
 * @brief ORBImplementation::ORBImplementation()
 *
 * Constructor of ORBImplentation. Initialise the _orbEventListener
 * in here, and create a signleton reference to use later
 */
ORBImplementation::ORBImplementation() :
    _adminLock(),
    _notificationClients({})
{
    ORB_LOG_NO_ARGS();
    _orbEventListener = std::make_shared<ORBEventListenerImpl>();
    instance(this);
}

/**
 * @brief ORBImplementation::~ORBImplementation()
 *
 * Destructor
 */

ORBImplementation::~ORBImplementation()
{
    ORB_LOG_NO_ARGS();
}

/**
 * @brief ORBImplementation::Register
 *
 * Register the callbacks for notifications. Whoever wants to receive notifications
 * needs to call this with the Exchange::IORB::INotification ref
 *
 * @param sink
 */
void ORBImplementation::Register(Exchange::IORB::INotification *sink)
{
    _adminLock.Lock();
    ORB_LOG("PID=%d", getpid());

    // Make sure a sink is not registered multiple times.
    if (std::find(_notificationClients.begin(), _notificationClients.end(), sink) ==
        _notificationClients.end())
    {
        _notificationClients.push_back(sink);
        sink->AddRef();
        ORB_LOG("Added a ref");
    }

    _adminLock.Unlock();

    ORB_LOG("Registered a sink on the ORB %p", sink);
}

/**
 * @brief ORBImplementation::Unregister
 *
 * Unregister callbacks
 *
 * @param sink
 */
void ORBImplementation::Unregister(Exchange::IORB::INotification *sink)
{
    ORB_LOG("PID=%d", getpid());
    _adminLock.Lock();
    auto itr = std::find(_notificationClients.begin(), _notificationClients.end(), sink);
    if (itr != _notificationClients.end())
    {
        (*itr)->Release();
        _notificationClients.erase(itr);
    }

    _adminLock.Unlock();
}

/**
 * @brief ORBImplementation::LoadPlatform
 *
 * Used to dlopen the ORBPlatform library. Called from ORB::Initialize
 */
bool ORBImplementation::LoadPlatform()
{
    bool result = false;
    ORB_LOG_NO_ARGS();
    _adminLock.Lock();
    result = ORBEngine::GetSharedInstance().Start(_orbEventListener);
    _adminLock.Unlock();
    return result;
}

/**
 * @brief ORBImplementation::UnLoadPlatform
 *
 * Used to unload the platform when exiting
 */
void ORBImplementation::UnLoadPlatform()
{
    ORB_LOG_NO_ARGS();
    _adminLock.Lock();
    ORBEngine::GetSharedInstance().Stop();
    _adminLock.Unlock();
}

/**
 * @brief ORBImplementation::ExecuteBridgeRequest
 * Execute the given WPE bridge request. Platform call
 *
 * @param request The request as a string
 * @return The response as a string
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
 *
 * @return The resulting token
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
 * @brief ORBImplementation::LoadDvbUrl
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
 * @param keyCode   The JavaScript key code
 * @param keyAction The JavaScript key action (0 = keyup , 1 = keydown)
 *
 * @return true
 * @return false
 */
bool ORBImplementation::SendKeyEvent(int keyCode, uint8_t keyAction)
{
    ORB_LOG_NO_ARGS();
    return ORBEngine::GetSharedInstance().SendKeyEvent(keyCode, keyAction);
}

/**
 * @brief ORBImplementation::SetPreferredUILanguage
 *
 * Set the preferred UI language.
 *
 * @param preferredUiLanguage The preferred UI language.
 *                            A comma-separated set of languages to be used for the user interface
 *                            of a service, in order of preference. Each language shall be indicated
 *                            by its ISO 639-2 language code as defined in [ISO639-2].
 */
void ORBImplementation::SetPreferredUILanguage(std::string preferredUiLanguage)
{
    ORB_LOG("preferredUiLanguage=%s", preferredUiLanguage.c_str());
    ORBEngine::GetSharedInstance().SetPreferredUILanguage(preferredUiLanguage);
}

/**
 * Get the User-Agent string.
 *
 * @return The User-Agent string
 */
std::string ORBImplementation::GetUserAgentString()
{
    ORB_LOG_NO_ARGS();
    std::string userAgentString = ORBEngine::GetSharedInstance().GetUserAgentString();
    return userAgentString;
}

/**
 * Get the current application URL.
 *
 * @return The current application URL
 */
std::string ORBImplementation::GetCurrentAppUrl()
{
    ORB_LOG_NO_ARGS();
    std::string currentAppUrl = ORBEngine::GetSharedInstance().GetCurrentAppUrl();
    return currentAppUrl;
}

/**
 * @brief ORBImplementation::JavaScriptEventDispatchRequest
 *
 * This method is used to notify each client for the 'JavaScriptEventDispatchRequest' event
 *
 * @param name
 * @param properties
 * @param broadcastRelated
 * @param targetOrigin
 */
void ORBImplementation::JavaScriptEventDispatchRequest(
    std::string name,
    std::string properties,
    bool broadcastRelated,
    std::string targetOrigin
    )
{
    ORB_LOG_NO_ARGS();

    // Loop through all the registered callbacks and fire off the notification
    std::lock_guard<std::mutex> locker(_notificationMutex);
    ORB_LOG("We have %d callbacks to trigger", _notificationClients.size());
    for (const auto client : _notificationClients)
    {
        client->JavaScriptEventDispatchRequest(name, properties, broadcastRelated, targetOrigin);
    }
}

/**
 * @brief ORBImplementation::DvbUrlLoaded
 *
 * This method is used to notify each client for the 'DvbUrlLoaded' event
 *
 * @param requestId
 * @param fileContent
 * @param fileContentLength
 */
void ORBImplementation::DvbUrlLoaded(
    int requestId,
    const uint8_t *fileContent,
    unsigned int fileContentLength
    )
{
    ORB_LOG_NO_ARGS();

    // Loop through all the registered callbacks and fire off the notification
    std::lock_guard<std::mutex> locker(_notificationMutex);
    ORB_LOG("We have %d callbacks to trigger", _notificationClients.size());
    for (const auto client : _notificationClients)
    {
        client->DvbUrlLoaded(requestId, fileContent, fileContentLength);
    }
}

/**
 * @brief ORBImplementation::DvbUrlLoadedNoData
 *
 * This method is used to notify each client for the 'DvbUrlLoadedNoData' event
 *
 * @param requestId
 * @param fileContentLength
 */
void ORBImplementation::DvbUrlLoadedNoData(int requestId, unsigned int fileContentLength)
{
    ORB_LOG_NO_ARGS();

    // Loop through all the registered callbacks and fire off the notification
    std::lock_guard<std::mutex> locker(_notificationMutex);
    ORB_LOG("We have %d callbacks to trigger", _notificationClients.size());
    for (const auto client : _notificationClients)
    {
        client->DvbUrlLoadedNoData(requestId, fileContentLength);
    }
}

/**
 * @brief ORBImplementation::EventInputKeyGenerated
 *
 * This method is used to notify each client for the 'EventInputKeyGenerated' event
 *
 * @param requestId
 * @param fileContent
 * @param fileContentLength
 */
void ORBImplementation::EventInputKeyGenerated(int keyCode, uint8_t keyAction)
{
    ORB_LOG_NO_ARGS();

    // Loop through all the registered callbacks and fire off the notification
    std::lock_guard<std::mutex> locker(_notificationMutex);
    ORB_LOG("We have %d callbacks to trigger", _notificationClients.size());
    for (const auto client : _notificationClients)
    {
        client->EventInputKeyGenerated(keyCode, keyAction);
    }
}

/**
 * @brief ORBImplementation::LaunchApplication
 *
 * Used to launch a Broadcast-INDEPENDENT application, the url could be an XML-AIT file
 *
 * @param url The url for the broadcast independent application
 * @return true Successful application creation
 * @return false Unsuccessful application creation
 */
bool ORBImplementation::LaunchApplication(std::string url)
{
    ORB_LOG_NO_ARGS();
    return ORBEngine::GetSharedInstance().GetApplicationManager()->CreateApplication(0, url);
}
}  // namespace Plugin
}  // namespace WPEFramework