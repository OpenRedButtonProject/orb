/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */
#pragma once

#include "application_manager.h"
#include "BroadcastRequestHandler.h"
#include "ConfigurationRequestHandler.h"
#include "ManagerRequestHandler.h"
#include "ProgrammeRequestHandler.h"
#include "ParentalControlRequestHandler.h"
#include "TokenManager.h"
#include "ORBPlatform.h"
#include "ORBPlatformLoader.h"
#include "MetadataSearchTask.h"
#include "ORBEventListener.h"
#include "ORBBrowserApi.h"
#include "ORBPlatformEventHandlerImpl.h"

#include <map>
#include <memory>

namespace orb
{
/**
 * The ORB engine is the entry point to the ORB functionality, which is logically organised in
 * three parts:
 *
 * 1) Engine API
 *    Provides the methods necessary to maintain the ORB engine.
 *
 * 2) Browser-specific API
 *    Provides the methods that are expected to be called by the browser.
 *
 * 3) WebApp-specific API
 *    Provides the methods that are expected to be called by the resident (web) app.
 */
class ORBEngine : public ORBBrowserApi
{
public:

   /**
    * Singleton
    */
   static ORBEngine& GetSharedInstance()
   {
      static ORBEngine s_orbEngine;
      return s_orbEngine;
   }

private:

   /**
    * Constructor.
    */
   ORBEngine();

public:

   /**
    * Destructor.
    */
   ~ORBEngine();

public:


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
   bool Start(std::shared_ptr<ORBEventListener> eventListener);

   /**
    * Stop the ORB engine.
    *
    * @return true on success, false otherwise
    */
   bool Stop();


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
   virtual std::string ExecuteBridgeRequest(std::string jsonRequest) override;

   /**
    * Create a new JSON token for the current application and the given URI.
    *
    * @param uri The given URI
    *
    * @return A string representation of the resulting JSON token
    */
   virtual std::string CreateToken(std::string uri) override;

   /**
    * Load the specified DVB URL through the underlying platform DSM-CC implementation.
    *
    * @param url       The DVB URL
    * @param requestId The distinctive request id
    */
   virtual void LoadDvbUrl(std::string url, int requestId) override;

   /**
    * Notify the application manager and the current JavaScript context that the specified HbbTV
    * application has failed to load.
    *
    * @param url              The application URL
    * @param errorDescription The error description
    */
   virtual void NotifyApplicationLoadFailed(std::string url, std::string errorDescription) override;

   /**
    * Notify the application manager that the page of the current HbbTV application has changed
    * and is about to load.
    *
    * @param url The application page URL
    */
   virtual void NotifyApplicationPageChanged(std::string url) override;


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
   bool SendKeyEvent(int keyCode, uint8_t keyAction);

public:

   // orb component getters
   std::shared_ptr<ORBEventListener> GetEventListener()
   {
      return m_eventListener;
   }

   std::shared_ptr<ApplicationManager> GetApplicationManager()
   {
      return m_applicationManager;
   }

   std::shared_ptr<TokenManager> GetTokenManager()
   {
      return m_tokenManager;
   }

   std::shared_ptr<BroadcastRequestHandler> GetBroadcastRequestHandler()
   {
      return m_broadcastRequestHandler;
   }

   std::shared_ptr<ConfigurationRequestHandler> GetConfigurationRequestHandler()
   {
      return m_configurationRequestHandler;
   }

   std::shared_ptr<ManagerRequestHandler> GetManagerRequestHandler()
   {
      return m_managerRequestHandler;
   }

   std::shared_ptr<ProgrammeRequestHandler> GetProgrammeRequestHandler()
   {
      return m_programmeRequestHandler;
   }

   std::shared_ptr<ParentalControlRequestHandler> GetParentalControlRequestHandler()
   {
      return m_parentalControlRequestHandler;
   }

   std::shared_ptr<ORBPlatformEventHandler> GetPlatformEventHandler()
   {
      return m_platformEventHandler;
   }

   ORBPlatform* GetORBPlatform()
   {
      return m_orbPlatform;
   }

   // orb state getters/setters
   void SetCurrentAppId(uint16_t appId)
   {
      m_currentAppId = appId;
   }

   uint16_t GetCurrentAppId() const
   {
      return m_currentAppId;
   }

   // orb metadata search task pool handling
   void AddMetadataSearchTask(int queryId, std::shared_ptr<MetadataSearchTask> searchTask)
   {
      m_metadataSearchTasks[queryId] = searchTask;
   }

   void RemoveMetadataSearchTask(int queryId)
   {
      m_metadataSearchTasks.erase(queryId);
   }

   std::shared_ptr<MetadataSearchTask> GetMetadataSearchTask(int queryId)
   {
      return m_metadataSearchTasks[queryId];
   }

   void SetPreferredUILanguage(std::string preferredUiLanguage)
   {
      m_preferredUiLanguage = preferredUiLanguage;
   }

   std::string GetPreferredUILanguage() const
   {
      return m_preferredUiLanguage;
   }

private:

   // member variables
   std::shared_ptr<ORBEventListener> m_eventListener;
   std::shared_ptr<ORBPlatformLoader> m_orbPlatformLoader;
   std::shared_ptr<ApplicationManager> m_applicationManager;
   std::shared_ptr<TokenManager> m_tokenManager;
   std::shared_ptr<BroadcastRequestHandler> m_broadcastRequestHandler;
   std::shared_ptr<ConfigurationRequestHandler> m_configurationRequestHandler;
   std::shared_ptr<ManagerRequestHandler> m_managerRequestHandler;
   std::shared_ptr<ProgrammeRequestHandler> m_programmeRequestHandler;
   std::shared_ptr<ParentalControlRequestHandler> m_parentalControlRequestHandler;
   std::shared_ptr<ORBPlatformEventHandlerImpl> m_platformEventHandler;
   ORBPlatform *m_orbPlatform;
   std::map<int, std::shared_ptr<MetadataSearchTask> > m_metadataSearchTasks;
   uint16_t m_currentAppId;
   bool m_started;
   std::string m_preferredUiLanguage;
}; // class ORBEngine
} // namespace orb
