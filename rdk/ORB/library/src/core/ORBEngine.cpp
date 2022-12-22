/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */
#include "ORBEngine.h"
#include "ORBPlatformLoader.h"
#include "SessionCallbackImpl.h"
#include "ORBLogging.h"
#include <cstdint>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace orb
{
/**
 * Resolves the object and method from the specified input, which has the following form:
 *
 * <object>.<method>
 *
 * @param input  (in)  The input string
 * @param object (out) Holds the resolved object in success
 * @param method (out) Holds the resolved method in success
 *
 * @return true in success, otherwise false
 */
static
bool ResolveObjectAndMethod(std::string input, std::string& object, std::string& method)
{
   std::vector<std::string> tokens;
   for (auto i = strtok(&input[0], "."); i != NULL; i = strtok(NULL, "."))
   {
      tokens.push_back(i);
   }
   if (tokens.size() != 2)
   {
      return false;
   }

   object = tokens[0];
   method = tokens[1];

   return true;
}

/**
 * Constructor.
 *
 * @param eventListener The event listener
 */
ORBEngine::ORBEngine()
   : m_eventListener(nullptr)
   , m_orbPlatformLoader(std::make_shared<ORBPlatformLoader>())
   , m_tokenManager(std::make_shared<TokenManager>())
   , m_broadcastRequestHandler(std::make_shared<BroadcastRequestHandler>())
   , m_configurationRequestHandler(std::make_shared<ConfigurationRequestHandler>())
   , m_managerRequestHandler(std::make_shared<ManagerRequestHandler>())
   , m_programmeRequestHandler(std::make_shared<ProgrammeRequestHandler>())
   , m_parentalControlRequestHandler(std::make_shared<ParentalControlRequestHandler>())
   , m_platformEventHandler(std::make_shared<ORBPlatformEventHandlerImpl>())
   , m_orbPlatform(nullptr)
   , m_currentAppId(UINT16_MAX)
   , m_currentAppUrl("")
   , m_started(false)
   , m_preferredUiLanguage("")
{
   ORB_LOG_NO_ARGS();
}

/**
 * Destructor.
 */
ORBEngine::~ORBEngine()
{
   ORB_LOG_NO_ARGS();
   m_metadataSearchTasks.clear();
}

/************************************************************************************************
** Public Engine API
***********************************************************************************************/


/**
 * Start the ORB engine.
 *
 * @param eventListener The event listener
 *
 * @return true on success, false otherwise
 */
bool ORBEngine::Start(std::shared_ptr<ORBEventListener> eventListener)
{
   ORB_LOG_NO_ARGS();

   if (m_started)
   {
      return true;
   }

   m_eventListener = eventListener;

   // load the ORB platform
   m_orbPlatform = m_orbPlatformLoader->Load();
   if (!m_orbPlatform)
   {
      ORB_LOG("ERROR: Could not load the ORB platform");
      return false;
   }

   // initialise the ORB platform
   m_orbPlatform->Platform_Initialise(m_platformEventHandler);

   // initialise the application manager
   if (!m_applicationManager)
   {
      std::unique_ptr<SessionCallbackImpl> sessionCallback = std::make_unique<SessionCallbackImpl>();
      m_applicationManager = std::make_shared<ApplicationManager>(std::move(sessionCallback));
   }

   m_started = true;
   return true;
}

/**
 * Stop the ORB engine.
 *
 * @return true on success, false otherwise
 */
bool ORBEngine::Stop()
{
   ORB_LOG_NO_ARGS();

   if (!m_started)
   {
      return true;
   }

   // finalise and unload the ORB platform
   if (m_orbPlatform)
   {
      m_orbPlatform->Platform_Finalise();
      m_orbPlatformLoader->Unload(m_orbPlatform);
   }

   m_eventListener = nullptr;
   m_started = false;
   return true;
}

/************************************************************************************************
** Public Browser-specific API
***********************************************************************************************/


/**
 * Execute the given bridge request.
 * The request is a string representation of a JSON object with the following form:
 *
 * {
 *    "token": <token>
 *    "method": <method>
 *    "params": <params>
 * }
 *
 * The response is also a string representation of a JSON object containing the results, if any.
 *
 * @param jsonRequest String representation of the JSON request
 *
 * @return A string representation of the JSON response
 */
std::string ORBEngine::ExecuteBridgeRequest(std::string jsonRequest)
{
   ORB_LOG("jsonRequest=%s", jsonRequest.c_str());

   json response = "{}"_json;
   json request = json::parse(jsonRequest);
   json jsonToken = request["token"];

   // Extract token payload and perform security check
   json jsonTokenPayload = GetTokenManager()->GetTokenPayload(jsonToken);
   if (jsonTokenPayload.is_null() || jsonTokenPayload.empty())
   {
      response = RequestHandler::MakeErrorResponse("Forbidden");
      return response.dump();
   }

   ORB_LOG("tokenPayload=%s", jsonTokenPayload.dump().c_str());

   // Resolve object and method
   std::string object;
   std::string method;
   if (!ResolveObjectAndMethod(request["method"], object, method))
   {
      response = RequestHandler::MakeErrorResponse("UnknownMethod");
      return response;
   }

   ORB_LOG("object=%s method=%s", object.c_str(), method.c_str());

   // Execute requested method
   json params = request["params"];
   if (object == "Broadcast")
   {
      GetBroadcastRequestHandler()->Handle(jsonToken, method, params, response);
   }
   else if (object == "Configuration")
   {
      GetConfigurationRequestHandler()->Handle(jsonToken, method, params, response);
   }
   else if (object == "Manager")
   {
      GetManagerRequestHandler()->Handle(jsonToken, method, params, response);
   }
   else if (object == "Programme")
   {
      GetProgrammeRequestHandler()->Handle(jsonToken, method, params, response);
   }
   else if (object == "ParentalControl")
   {
      GetParentalControlRequestHandler()->Handle(jsonToken, method, params, response);
   }
   else
   {
      response = RequestHandler::MakeErrorResponse("UnknownMethod");
   }

   ORB_LOG("response=%s", response.dump().c_str());
   return response.dump();
}

/**
 * Create a new JSON token for the current application and the given URI.
 *
 * @param uri The given URI
 *
 * @return A string representation of the resulting JSON token
 */
std::string ORBEngine::CreateToken(std::string uri)
{
   ORB_LOG("uri=%s appId=%hu", uri.c_str(), m_currentAppId);
   json token = "{}"_json;
   if (m_currentAppId == UINT16_MAX)
   {
      ORB_LOG("No app is currently running");
      return token.dump();
   }

   token = m_tokenManager->CreateToken(m_currentAppId, uri);
   return token.dump();
}

/**
 * Load the specified DVB URL through the underlying platform DSM-CC implementation.
 *
 * @param url       The DVB URL
 * @param requestId The distinctive request id
 */
void ORBEngine::LoadDvbUrl(std::string url, int requestId)
{
   ORB_LOG("url=%s requestId=%d", url.c_str(), requestId);
   GetORBPlatform()->Dsmcc_RequestFile(url, requestId);
}

/**
 * Notify the application manager and the current JavaScript context that the specified HbbTV
 * application has failed to load.
 *
 * @param url              The application URL
 * @param errorDescription The error description
 */
void ORBEngine::NotifyApplicationLoadFailed(std::string url, std::string errorDescription)
{
   ORB_LOG("appId=%d url=%s error=%s", m_currentAppId, url.c_str(), errorDescription.c_str());

   // notify the application manager that the loading of the given application has failed
   bool isConnectedToInternet = GetORBPlatform()->Network_IsConnectedToInternet();
   GetApplicationManager()->OnNetworkAvailabilityChanged(isConnectedToInternet);
   GetApplicationManager()->OnLoadApplicationFailed(m_currentAppId);
}

/**
 * Notify the application manager that the page of the current HbbTV application has changed
 * and is about to load.
 *
 * @param url The application page URL
 */
void ORBEngine::NotifyApplicationPageChanged(std::string url)
{
   ORB_LOG("appId=%d url=%s", m_currentAppId, url.c_str());

   m_currentAppUrl = url;
   GetApplicationManager()->OnApplicationPageChanged(m_currentAppId, url);
   GetORBPlatform()->Platform_SetCurrentKeySetMask(0);
}

/**
 * Get the User-Agent string.
 *
 * @return The User-Agent string
 */
std::string ORBEngine::GetUserAgentString()
{
   ORB_LOG_NO_ARGS();
   std::string userAgentString = GetORBPlatform()->Configuration_GetUserAgentString();
   return userAgentString;
}

/**
 * Get the current application URL.
 *
 * @return The current application URL
 */
std::string ORBEngine::GetCurrentAppUrl()
{
   ORB_LOG("currentAppUrl = %s", m_currentAppUrl.c_str());
   return m_currentAppUrl;
}

/************************************************************************************************
** Public WebApp-specific API
***********************************************************************************************/


/**
 * Send the specified key event to the current HbbTV application (if any).
 * This method is intended to serve scenarios where the resident app is the main component
 * responsible for key event handling.
 *
 * @param keyCode   The event's JavaScript key code
 * @param keyAction The event's action (0 = keyup , 1 = keydown)
 *
 * @return True if the key event was generated on the current HbbTV application, otherwise false
 */
bool ORBEngine::SendKeyEvent(int keyCode, uint8_t keyAction)
{
   ORB_LOG("keyCode=%d keyAction=%d", keyCode, keyAction);

   KeyAction action = keyAction == 0 ? KeyAction::KEY_ACTION_UP : KeyAction::KEY_ACTION_DOWN;

   bool consumed = GetPlatformEventHandler()->OnInputKeyGenerated(keyCode, action);

   return consumed;
}
} // namespace orb
