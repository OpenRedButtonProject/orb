/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 *
 * Application manager
 *
 * Note: This file is part of the platform-agnostic application manager library.
 */

#include "application_manager.h"

#include <memory>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

#include "ait.h"
#include "log.h"
#include "utils.h"
#include "xml_parser.h"


#define KEY_SET_RED 0x1
#define KEY_SET_GREEN 0x2
#define KEY_SET_YELLOW 0x4
#define KET_SET_BLUE 0x8
#define KEY_SET_NAVIGATION 0x10
#define KEY_SET_VCR 0x20
#define KEY_SET_SCROLL 0x40
#define KEY_SET_INFO 0x80
#define KEY_SET_NUMERIC 0x100
#define KEY_SET_ALPHA 0x200
#define KEY_SET_OTHER 0x400

#define VK_RED 403
#define VK_GREEN 404
#define VK_YELLOW 405
#define VK_BLUE 406
#define VK_UP 38
#define VK_DOWN 40
#define VK_LEFT 37
#define VK_RIGHT 39
#define VK_ENTER 13
#define VK_BACK 461
#define VK_PLAY 415
#define VK_STOP 413
#define VK_PAUSE 19
#define VK_FAST_FWD 417
#define VK_REWIND 412
#define VK_NEXT 425
#define VK_PREV 424
#define VK_PLAY_PAUSE 402
#define VK_PAGE_UP 33
#define VK_PAGE_DOWN 34
#define VK_INFO 457
#define VK_NUMERIC_START 48
#define VK_NUMERIC_END 57
#define VK_ALPHA_START 65
#define VK_ALPHA_END 90

static bool IsKeyNavigation(uint16_t code);
static bool IsKeyNumeric(uint16_t code);
static bool IsKeyAlpha(uint16_t code);
static bool IsKeyVcr(uint16_t code);
static bool IsKeyScroll(uint16_t code);

/**
 * Application manager
 *
 * @param sessionCallback Implementation of ApplicationManager::SessionCallback interface.
 */
ApplicationManager::ApplicationManager(std::unique_ptr<SessionCallback> sessionCallback) :
   session_callback_(std::move(sessionCallback)),
   next_app_id_(0),
   ait_timeout_([&] {
   OnSelectedServiceAitTimeout();
}, std::chrono::milliseconds(Utils::AIT_TIMEOUT))
{
   session_callback_->HideApplication();
}

/**
 *
 */
ApplicationManager::~ApplicationManager() = default;

/**
 * Create and run a new application. If called by an application, check it is allowed.
 *
 * @param calling_app_id The calling app ID or INVALID_APP_ID if not called by an app.
 * @param url A HTTP/HTTPS or DVB URL.
 *
 * A HTTP/HTTPS URL may refer to the entry page or XML AIT of a broadcast-independent app.
 *
 * A DVB URL may refer to a broadcast-related app signalled in the current service AIT. This
 * will result in the signalled URL being loaded, which may be HTTP/HTTPS for broadband or DVB
 * for carousel.
 *
 * @return true if the application can be created, otherwise false
 */
bool ApplicationManager::CreateApplication(uint16_t calling_app_id, const std::string &url)
{
   bool result = false;
   const Ait::S_AIT_APP_DESC *app_description;
   std::lock_guard<std::recursive_mutex> lock(lock_);

   LOG(LOG_INFO, "CreateApplication");
   if (calling_app_id != INVALID_APP_ID)
   {
      if (!app_.is_running || app_.id != calling_app_id)
      {
         LOG(LOG_INFO, "Called by non-running app, early out");
         return false;
      }
   }
   if (url.empty())
   {
      LOG(LOG_INFO, "Called with empty URL, early out");
      session_callback_->DispatchApplicationLoadErrorEvent();
      return false;
   }

   Utils::CreateLocatorInfo info = Utils::ParseCreateLocatorInfo(url, current_service_);
   switch (info.type)
   {
      case Utils::CreateLocatorType::AIT_APPLICATION_LOCATOR:
      {
         LOG(LOG_INFO, "Create for AIT_APPLICATION_LOCATOR (url=%s)", url.c_str());
         if (ait_.Get() == nullptr)
         {
            LOG(LOG_INFO, "No AIT, early out");
            break;
         }
         app_description = Ait::FindApp(ait_.Get(), info.org_id, info.app_id);
         if (app_description)
         {
            auto new_app = App::CreateAppFromAitDesc(app_description, current_service_,
               is_network_available_, info.parameters, true, false);
            result = RunApp(new_app);
         }
         else
         {
            LOG(LOG_ERROR, "Could not find app (org_id=%d, app_id=%d)",
               info.org_id,
               info.app_id);
         }

         break;
      }

      case Utils::CreateLocatorType::ENTRY_PAGE_OR_XML_AIT_LOCATOR:
      {
         LOG(LOG_INFO, "Create for ENTRY_PAGE_OR_XML_AIT_LOCATOR (url=%s)", url.c_str());
         std::string contents = session_callback_->GetXmlAitContents(url);
         if (!contents.empty())
         {
            LOG(LOG_INFO, "Locator resource is XML AIT");
            result = ProcessXmlAit(contents);
         }
         else
         {
            LOG(LOG_INFO, "Locator resource is ENTRY PAGE");
            result = RunApp(App::CreateAppFromUrl(url));
         }
         break;
      }

      case Utils::CreateLocatorType::UNKNOWN_LOCATOR:
      {
         LOG(LOG_INFO, "Do not create for UNKNOWN_LOCATOR (url=%s)", url.c_str());
         result = false;
         break;
      }
   }

   if (!result)
   {
      session_callback_->DispatchApplicationLoadErrorEvent();
   }

   return result;
}

/**
 * Destroy the calling application.
 *
 * @param calling_app_id The calling app ID.
 */
void ApplicationManager::DestroyApplication(uint16_t calling_app_id)
{
   std::lock_guard<std::recursive_mutex> lock(lock_);

   LOG(LOG_ERROR, "DestroyApplication");

   if (!app_.is_running || app_.id != calling_app_id)
   {
      LOG(LOG_INFO, "Called by non-running app, early out");
      return;
   }

   KillRunningApp();
   OnRunningAppExited();
}

/**
 * Show the calling application.
 *
 * @param calling_app_id The calling app ID.
 */
void ApplicationManager::ShowApplication(uint16_t calling_app_id)
{
   std::lock_guard<std::recursive_mutex> lock(lock_);
   if (app_.id == calling_app_id)
   {
      app_.is_hidden = false;
      if (app_.is_running)
      {
         session_callback_->ShowApplication();
      }
   }
}

/**
 * Hide the calling application.
 *
 * @param calling_app_id The calling app ID.
 */
void ApplicationManager::HideApplication(uint16_t calling_app_id)
{
   std::lock_guard<std::recursive_mutex> lock(lock_);
   if (app_.id == calling_app_id)
   {
      app_.is_hidden = true;
      if (app_.is_running)
      {
         session_callback_->HideApplication();
      }
   }
}

/**
 * Set the key set mask for an application.
 *
 * @param app_id The application.
 * @param key_set_mask The key set mask.
 * @return The key set mask for the application.
 */
uint16_t ApplicationManager::SetKeySetMask(uint16_t app_id, uint16_t key_set_mask)
{
   std::lock_guard<std::recursive_mutex> lock(lock_);
   if (app_.id == app_id)
   {
      if (!app_.is_activated)
      {
         if ((key_set_mask & KEY_SET_VCR) != 0)
         {
            key_set_mask &= ~KEY_SET_VCR;
         }
         if ((key_set_mask & KEY_SET_NUMERIC) != 0)
         {
            key_set_mask &= ~KEY_SET_NUMERIC;
         }
      }
      app_.key_set_mask = key_set_mask;
   }
   else
   {
      key_set_mask = 0;
   }
   return key_set_mask;
}

/**
 * Get the key set mask for an application.
 *
 * @param app_id The application.
 * @return The key set mask for the application.
 */
uint16_t ApplicationManager::GetKeySetMask(uint16_t app_id)
{
   std::lock_guard<std::recursive_mutex> lock(lock_);
   if (app_.id == app_id)
   {
      return app_.key_set_mask;
   }
   return 0;
}

/**
 * Check the key code is accepted by the current key mask. Activate the app as a result if the
 * key is accepted.
 *
 * @param app_id The application.
 * @param key_code The key code to check.
 * @return The supplied key_code is accepted by the current app's key set.
*/
bool ApplicationManager::InKeySet(uint16_t app_id, uint16_t key_code)
{
   std::lock_guard<std::recursive_mutex> lock(lock_);
   if (app_.id == app_id)
   {
      if ((app_.key_set_mask & GetKeySet(key_code)) != 0)
      {
          if (!app_.is_activated) {
             app_.is_activated = true;
          }
          return true;
      }
   }
   return false;
}

/**
 * Process an AIT section. The table will be processed when it is completed or updated.
 *
 * @param ait_pid The section PID.
 * @param service_id The service this section was received for.
 * @param section_data The section section_data.
 * @param section_data_bytes The size of section_data in bytes.
 */
void ApplicationManager::ProcessAitSection(uint16_t ait_pid, uint16_t service_id, uint8_t *section_data, uint32_t section_data_bytes)
{
   std::lock_guard<std::recursive_mutex> lock(lock_);

   LOG(LOG_INFO, "ProcessAitSection");

   if (service_id != current_service_.service_id)
   {
      LOG(LOG_INFO, "The AIT is not for the current service, early out");
      return;
   }

   if (ait_pid != current_service_ait_pid_)
   {
      if (current_service_ait_pid_ != 0)
      {
         LOG(LOG_INFO, "The AIT comes in a different PID, now=%d before= %d", ait_pid, current_service_ait_pid_);
         ait_.Clear();
      }
      current_service_ait_pid_ = ait_pid;
   }

   if (!ait_.ProcessSection(section_data, section_data_bytes))
   {
      LOG(LOG_INFO, "The AIT was not completed and/or updated, early out");
      return;
   }

   const Ait::S_AIT_TABLE *updated_ait = ait_.Get();
   if (updated_ait == nullptr)
   {
      LOG(LOG_ERROR, "No AIT, early out");
      return;
   }

   if (!current_service_received_first_ait_)
   {
      ait_timeout_.stop();
      current_service_received_first_ait_ = true;
      OnSelectedServiceAitReceived();
   }
   else
   {
      OnSelectedServiceAitUpdated();
   }
}

/**
 * Process an XML AIT and create and run a new broadcast-independent application.
 *
 * @param xml_ait The XML AIT contents.
 * @return true if the application can be created, otherwise false
 */
bool ApplicationManager::ProcessXmlAit(const std::string &xml_ait)
{
   const Ait::S_AIT_APP_DESC *app_description;
   bool result = false;

   std::lock_guard<std::recursive_mutex> lock(lock_);

   LOG(LOG_INFO, "ProcessXmlAit");

   if (xml_ait.empty())
   {
      return false;
   }

   xml_ait_ = std::move(XmlParser::ParseAit(xml_ait.c_str(), xml_ait.length()));
   if (nullptr == xml_ait_ || xml_ait_->num_apps == 0)
   {
      // No AIT or apps parsed, early out
      return false;
   }

   Ait::PrintInfo(xml_ait_.get());
   app_description = GetAutoStartApp(xml_ait_.get());

   if (app_description)
   {
      auto new_app = App::CreateAppFromAitDesc(app_description, current_service_,
         is_network_available_, "", false, false);
      result = RunApp(new_app);
      if (!result)
      {
         LOG(LOG_ERROR, "Could not find app (org_id=%d, app_id=%d)",
            app_description->org_id,
            app_description->app_id);
      }
   }


   return result;
}

/**
 * Check whether a Teletext application is signalled.
 *
 * @return true if a Teletext application is signalled, otherwise false
 */
bool ApplicationManager::IsTeletextApplicationSignalled()
{
   std::lock_guard<std::recursive_mutex> lock(lock_);
   if (ait_.Get() == nullptr)
   {
      return false;
   }
   return Ait::TeletextApp(ait_.Get()) != nullptr;
}

/**
 * Run the signalled Teletext application.
 *
 * @return true if the Teletext application can be created, otherwise false
 */
bool ApplicationManager::RunTeletextApplication()
{
   const Ait::S_AIT_APP_DESC *app_description;
   std::lock_guard<std::recursive_mutex> lock(lock_);

   LOG(LOG_INFO, "RunTeletextApplication");

   if (ait_.Get() == nullptr)
   {
      return false;
   }
   app_description = Ait::TeletextApp(ait_.Get());
   if (app_description == nullptr)
   {
      LOG(LOG_ERROR, "Could not find Teletext app");

      return false;
   }

   auto new_app = App::CreateAppFromAitDesc(app_description, current_service_, is_network_available_,
      "", true, false);
   return RunApp(new_app);
}

/**
 * Check whether a request from the polyfill is allowed.
 *
 * @param calling_app_id The app ID making the request.
 * @param calling_page_url The page URL making the request.
 * @param method_requirement Any additional requirement of the method.
 * @return true if the request is allowed, otherwise false
 */
bool ApplicationManager::IsRequestAllowed(uint16_t calling_app_id, const std::string &calling_page_url,
   MethodRequirement method_requirement)
{
   std::lock_guard<std::recursive_mutex> lock(lock_);

   if (!app_.is_running || app_.id != calling_app_id)
   {
      return false;
   }

   if (calling_page_url.empty() || Utils::CompareUrls(calling_page_url, "about:blank"))
   {
      return false;
   }

   switch (method_requirement)
   {
      case MethodRequirement::FOR_RUNNING_APP_ONLY:
      {
         return true;
      }
      case MethodRequirement::FOR_BROADCAST_APP_ONLY:
      {
         return app_.is_broadcast;
      }
      case MethodRequirement::FOR_BROADCAST_OR_TRANSITIONING_APP_ONLY:
      {
         return !Utils::IsInvalidDvbTriplet(current_service_);
      }
      case MethodRequirement::FOR_TRUSTED_APP_ONLY:
      {
         // Check document URL is inside app boundaries
         if (!Utils::CheckBoundaries(calling_page_url, app_.entry_url, app_.boundaries))
         {
            return false;
         }
         return app_.is_trusted;
      }
      default:
      {
         return false;
      }
   }
}

/**
 * Called when broadcast is stopped (for example when v/b object setChannel is called with null).
 *
 * If a broadcast-related application is running, it will transition to broadcast-independent or
 * be killed depending on the signalling.
 */
void ApplicationManager::OnBroadcastStopped()
{
   LOG(LOG_DEBUG, "OnBroadcastStopped");
   std::lock_guard<std::recursive_mutex> lock(lock_);
   current_service_received_first_ait_ = false;
   current_service_ait_pid_ = 0;
   ait_.Clear();
   current_service_ = Utils::MakeInvalidDvbTriplet();
   if (!TransitionRunningAppToBroadcastIndependent())
   {
      LOG(LOG_INFO, "Kill running app (could not transition to broadcast-independent)");
      KillRunningApp();
   }
}

/**
 * Called when the selected broadcast channel is changed (e.g. by the user or by v/b object).
 *
 * Once the first complete AIT is received or times out:
 *
 * If a broadcast-related application is running, it will continue to run or be killed depending
 * on the signalling.
 *
 * If a broadcast-independent application is running, it will transition to broadcast-related or
 * be killed depending on the signalling.
 */
void ApplicationManager::OnChannelChanged(uint16_t original_network_id,
   uint16_t transport_stream_id, uint16_t service_id)
{
   LOG(LOG_DEBUG, "OnChannelChanged (current service: %d)", current_service_.service_id);
   std::lock_guard<std::recursive_mutex> lock(lock_);
   current_service_received_first_ait_ = false;
   current_service_ait_pid_ = 0;
   ait_.Clear();
   ait_timeout_.start();
   current_service_ = {
      .original_network_id = original_network_id,
      .transport_stream_id = transport_stream_id,
      .service_id = service_id,
   };
}

/**
 * Called when the network availability has changed.
 *
 * @param available true if the network is available, otherwise false
 */
void ApplicationManager::OnNetworkAvailabilityChanged(bool available)
{
   std::lock_guard<std::recursive_mutex> lock(lock_);
   LOG(LOG_DEBUG, "OnNetworkAvailabilityChanged available=%d", available);
   is_network_available_ = available;
}

/**
 * Notify the application manager that a call to loadApplication failed.
 *
 * @param app_id The application ID of the application that failed to load.
 */
void ApplicationManager::OnLoadApplicationFailed(uint16_t app_id)
{
   std::lock_guard<std::recursive_mutex> lock(lock_);

   // TODO If a call to createApplication has failed, set app_ back to old_app_ and send event?

   if (Utils::IsInvalidDvbTriplet(current_service_))
   {
      LOG(LOG_ERROR, "Unhandled condition (failed to load application while broadcast-independent)");
      return;
   }

   if (!app_.is_running || app_.id != app_id)
   {
      return;
   }
   auto ait = ait_.Get();
   if (ait != nullptr && app_.is_running && app_.app_id != 0 && app_.org_id != 0)
   {
      Ait::S_AIT_APP_DESC *app = Ait::FindApp(ait, app_.org_id, app_.app_id);
      if (app != nullptr)
      {
         Ait::AppSetTransportFailedToLoad(app, app_.protocol_id);
      }
   }
   KillRunningApp();
   OnPerformBroadcastAutostart();
}

/**
 * Notify the application manager of application page changed, before the new page is
 * loaded. For example, when the user follows a link.
 *
 * @param app_id The application ID.
 * @param url The URL of the new page.
 */
void ApplicationManager::OnApplicationPageChanged(uint16_t app_id, const std::string &url)
{
   std::lock_guard<std::recursive_mutex> lock(lock_);
   if (app_.is_running && app_.id == app_id)
   {
      app_.loaded_url = url;
      if (!Utils::IsInvalidDvbTriplet(current_service_))
      {
         // For broadcast-related applications we reset the broadcast presentation on page change,
         // as dead JS objects may have suspended presentation, set the video rectangle or set
         // the presented components.
         session_callback_->ResetBroadcastPresentation();
      }
   }
}

// Private methods...

/**
 * Called when the AIT for the selected service is received.
 */
void ApplicationManager::OnSelectedServiceAitReceived()
{
   LOG(LOG_INFO, "OnSelectedServiceAitReceived");
   auto ait = ait_.Get();
   if (ait != nullptr)
   {
      LOG(LOG_INFO, "New service selected and first AIT received");

      if (app_.is_running)
      {
         if (app_.is_broadcast)
         {
            LOG(LOG_INFO, "OnSelectedServiceAitReceived: Pre-existing broadcast-related app already running");
            if (app_.is_service_bound)
            {
               LOG(LOG_INFO, "Kill running app (is service bound)");
               KillRunningApp();
            }
            else
            {
               auto signalled = Ait::FindApp(ait, app_.org_id, app_.app_id);
               if (signalled == nullptr)
               {
                  LOG(LOG_INFO, "Kill running app (is not signalled in the new AIT)");
                  KillRunningApp();
               }
               else if (signalled->control_code == Ait::APP_CTL_KILL)
               {
                  LOG(LOG_INFO, "Kill running app (signalled with control code KILL)");
                  KillRunningApp();
               }
               else if (!Ait::AppHasTransport(signalled, app_.protocol_id))
               {
                  LOG(LOG_INFO, "Kill running app (is not signalled in the new AIT with the same transport protocol)");
                  KillRunningApp();
               }
            }
         }
         else
         {
            LOG(LOG_INFO, "Pre-existing broadcast-independent app already running");
            if (!TransitionRunningAppToBroadcastRelated())
            {
               LOG(LOG_INFO, "Kill running app (could not transition to broadcast-related)");
               KillRunningApp();
            }
         }
      }
      if (!app_.is_running)
      {
         OnPerformBroadcastAutostart();
      }
   }
}

/**
 * Called when the AIT for the selected service is not received after some timeout.
 */
void ApplicationManager::OnSelectedServiceAitTimeout()
{
   std::lock_guard<std::recursive_mutex> lock(lock_);
   LOG(LOG_INFO, "OnSelectedServiceAitTimeout");
   KillRunningApp();
}

/**
 * Called when the AIT for the selected service is updated.
 */
void ApplicationManager::OnSelectedServiceAitUpdated()
{
   auto ait = ait_.Get();
   LOG(LOG_INFO, "OnSelectedServiceAitUpdated");
   if (ait == nullptr)
   {
      LOG(LOG_ERROR, "Unexpected condition (AIT updated but is missing)");
      return;
   }

   if (app_.is_running)
   {
      if (!app_.is_broadcast)
      {
         // If the running app is not broadcast-related, we should not be tuned to broadcast
         LOG(LOG_ERROR, "Unexpected condition (AIT updated but app is not broadcast-related)");
         return;
      }

      LOG(LOG_INFO, "OnSelectedServiceAitUpdated: Pre-existing broadcast-related app already running");
      auto signalled = Ait::FindApp(ait, app_.org_id, app_.app_id);
      if (signalled == nullptr)
      {
         LOG(LOG_INFO, "Kill running app (is not signalled in the updated AIT)");
         KillRunningApp();
      }
      else if (!Ait::AppHasTransport(signalled, app_.protocol_id))
      {
         LOG(LOG_INFO, "Kill running app (is not signalled in the updated AIT with the same transport protocol)");
         KillRunningApp();
      }
      else if (signalled->control_code == Ait::APP_CTL_KILL)
      {
         LOG(LOG_INFO, "Kill running app (signalled has control code KILL)");
         KillRunningApp();
      }
   }

   if (!app_.is_running)
   {
      OnPerformBroadcastAutostart();
   }
}

/**
 * Called when the running app has exited.
 */
void ApplicationManager::OnRunningAppExited()
{
   std::lock_guard<std::recursive_mutex> lock(lock_);
   LOG(LOG_ERROR, "OnRunningAppExited");
   if (!Utils::IsInvalidDvbTriplet(current_service_))
   {
      OnPerformBroadcastAutostart();
   }
   else
   {
      // TODO This behaviour is implementation specific
      LOG(LOG_ERROR, "Unhandled condition (broadcast-independent app exited)");
   }
}

/**
 * Called at a time when the broadcast autostart app should be started.
 */
void ApplicationManager::OnPerformBroadcastAutostart()
{
   std::lock_guard<std::recursive_mutex> lock(lock_);
   LOG(LOG_ERROR, "OnPerformAutostart");

   // Find autostart app_desc
   auto ait = ait_.Get();
   if (!current_service_received_first_ait_ || ait == nullptr)
   {
      LOG(LOG_INFO, "OnPerformAutostart No service selected/AIT, early out");
      return;
   }
   auto app_desc = GetAutoStartApp(ait);

   if (app_desc != nullptr)
   {
      LOG(LOG_ERROR, "OnPerformAutostart Start autostart app.");

      auto new_app = App::CreateAppFromAitDesc(app_desc, current_service_, is_network_available_,
         "", true, false);
      if (!RunApp(new_app))
      {
         LOG(LOG_ERROR, "OnPerformAutostart Failed to create autostart app.");
      }
   }
   else
   {
      LOG(LOG_INFO, "OnPerformAutostart No autostart app found.");
   }
}

/**
 * Run the app.
 *
 * @param app The app to run.
 * @return True on success, false on failure.
 */
bool ApplicationManager::RunApp(const App &app)
{
   std::lock_guard<std::recursive_mutex> lock(lock_);
   if (!app.entry_url.empty())
   {
      /* Note: XML AIt uses the alpha-2 region codes as defined in ISO 3166-1.
       * DVB's parental_rating_descriptor uses the 3-character code as specified in ISO 3166. */
      std::string parental_control_region = session_callback_->GetParentalControlRegion();
      std::string parental_control_region3 = session_callback_->GetParentalControlRegion3();
      int parental_control_age = session_callback_->GetParentalControlAge();
      //if none of the parental ratings provided in the broadcast AIT or XML AIT are supported
      //by the terminal), the request to launch the application shall fail.
      if (Ait::IsAgeRestricted(app.parental_ratings, parental_control_age, parental_control_region, parental_control_region3))
      {
         LOG(LOG_ERROR, "%s, Parental Control Age RESTRICTED for %s: only %d content accepted",
            app.loaded_url.c_str(), parental_control_region.c_str(), parental_control_age);
         return false;
      }

      if (++next_app_id_ == 0)
      {
         ++next_app_id_;
      }

      app_ = app;
      app_.id = next_app_id_;
      app_.is_running = true;

      if (app_.is_hidden)
      {
         session_callback_->HideApplication();
      }

      if (!app.is_broadcast)
      {
         // The app is broadcast-independent (e.g. created from a URL), stop the broadcast if there
         // is a current service.
         if (!Utils::IsInvalidDvbTriplet(current_service_))
         {
            session_callback_->StopBroadcast();
            current_service_ = Utils::MakeInvalidDvbTriplet();
         }
      }

      session_callback_->LoadApplication(app_.id, app_.entry_url.c_str());

      if (!app_.is_hidden)
      {
         session_callback_->ShowApplication();
      }

      return true;
   }
   return false;
}

/**
 * Kill the running app.
 */
void ApplicationManager::KillRunningApp()
{
   std::lock_guard<std::recursive_mutex> lock(lock_);
   session_callback_->HideApplication();
   if (++next_app_id_ == 0)
   {
      ++next_app_id_;
   }
   session_callback_->LoadApplication(next_app_id_, "about:blank");
   app_.is_running = false;
}

/**
 * Transition the running app to broadcast related, if conditions permit.
 *
 * @return True on success, false on failure.
 */
bool ApplicationManager::TransitionRunningAppToBroadcastRelated()
{
   std::lock_guard<std::recursive_mutex> lock(lock_);
   LOG(LOG_INFO, "TransitionRunningAppToBroadcastRelated");
   auto ait = ait_.Get();
   if (ait == nullptr)
   {
      LOG(LOG_INFO, "Cannot transition to broadcast (no broadcast AIT)");
      return false;
   }
   if (!app_.is_running || (app_.app_id == 0) || (app_.org_id == 0))
   {
      LOG(LOG_INFO, "Cannot transition to broadcast (no running app or app/org id is 0)");
      return false;
   }
   const Ait::S_AIT_APP_DESC *app = Ait::FindApp(ait, app_.org_id, app_.app_id);
   if (app == nullptr)
   {
      LOG(LOG_INFO, "Cannot transition to broadcast (app is not signalled in the new AIT)");
      return false;
   }
   if (app->control_code != Ait::APP_CTL_AUTOSTART && app->control_code != Ait::APP_CTL_PRESENT)
   {
      LOG(LOG_INFO, "Cannot transition to broadcast (app is not signalled in the new AIT as AUTOSTART or PRESENT)");
      return false;
   }

   // Try and find entry URL in boundaries
   uint8_t i;
   bool entry_url_in_boundaries = false;
   for (i = 0; i < app->num_transports; i++)
   {
      if (app->transport_array[i].protocol_id == AIT_PROTOCOL_HTTP)
      {
         if (Utils::CheckBoundaries(app_.entry_url, app->transport_array[i].url.base_url,
            app->boundaries))
         {
            entry_url_in_boundaries = true;
            break;
         }
      }
   }
   if (!entry_url_in_boundaries)
   {
      LOG(LOG_INFO, "Cannot transition to broadcast (entry URL is not in boundaries)");
      return false;
   }

   // Try and find loaded URL in boundaries
   bool loaded_url_in_boundaries = false;
   for (i = 0; i < app->num_transports; i++)
   {
      if (app->transport_array[i].protocol_id == AIT_PROTOCOL_HTTP)
      {
         if (Utils::CheckBoundaries(app_.loaded_url, app->transport_array[i].url.base_url,
            app->boundaries))
         {
            loaded_url_in_boundaries = true;
            break;
         }
      }
   }
   if (!loaded_url_in_boundaries)
   {
      LOG(LOG_INFO, "Cannot transition to broadcast (loaded URL is not in boundaries)");
      return false;
   }

   app_.is_broadcast = true;
   app_.is_service_bound = app->app_desc.service_bound;
   /* Note: what about app.is_trusted, app.parental_ratings, ... */
   session_callback_->DispatchTransitionedToBroadcastRelatedEvent();

   return true;
}

/**
 * Transition the running app to broadcast-independent, if conditions permit.
 *
 * @return true on success, false on failure.
 */
bool ApplicationManager::TransitionRunningAppToBroadcastIndependent()
{
   app_.is_broadcast = false;
   return true;
}

/**
 * Whether the app should be trusted or not TODO
 *
 * @param is_broadcast Whether the app is broadcast-related
 * @return True if the app is trusted, false otherwise
 */
bool ApplicationManager::IsAppTrusted(bool)
{
   // TODO See specification. Probably need to add more parameters to this method
   return false;
}

/**
 * Call to Ait::AutoStartApp() passing the parental restrictions.
 *
 * @param ait_table AIT table.
 * @return The App to auto start.
 */
const Ait::S_AIT_APP_DESC * ApplicationManager::GetAutoStartApp(const Ait::S_AIT_TABLE *ait_table)
{
   int index;
   LOG(LOG_ERROR, "GetAutoStartApp");

   /* Note: XML AIt uses the alpha-2 region codes as defined in ISO 3166-1.
    * DVB's parental_rating_descriptor uses the 3-character code as specified in ISO 3166. */
   std::string parental_control_region = session_callback_->GetParentalControlRegion();
   std::string parental_control_region3 = session_callback_->GetParentalControlRegion3();
   int parental_control_age = session_callback_->GetParentalControlAge();
   return Ait::AutoStartApp(ait_table, parental_control_age, parental_control_region, parental_control_region3);
}

/**
* Return the KeySet a key code belongs to.
*
* @param code The key code.
* @return The key set.
*/
uint16_t ApplicationManager::GetKeySet(const uint16_t code)
{
   if (IsKeyNavigation(code))
   {
      return KEY_SET_NAVIGATION;
   }
   else if (IsKeyNumeric(code))
   {
      return KEY_SET_NUMERIC;
   }
   else if (IsKeyAlpha(code))
   {
      return KEY_SET_ALPHA;
   }
   else if (IsKeyVcr(code))
   {
      return KEY_SET_VCR;
   }
   else if (IsKeyScroll(code))
   {
      return KEY_SET_SCROLL;
   }
   else if (code == KEY_SET_RED)
   {
      return KEY_SET_RED;
   }
   else if (code == VK_GREEN)
   {
      return KEY_SET_GREEN;
   }
   else if (code == VK_YELLOW)
   {
      return KEY_SET_YELLOW;
   }
   else if (code == VK_BLUE)
   {
      return KET_SET_BLUE;
   }
   else if (code == VK_INFO)
   {
      return KEY_SET_INFO;
   }

   return KEY_SET_OTHER;
}

static bool IsKeyNavigation(uint16_t code) {
   return code == VK_UP ||
      code == VK_DOWN ||
      code == VK_LEFT ||
      code == VK_RIGHT ||
      code == VK_ENTER ||
      code == VK_BACK;
}

static bool IsKeyNumeric(uint16_t code) {
   return code >= VK_NUMERIC_START && code <= VK_NUMERIC_END;
}

static bool IsKeyAlpha(uint16_t code) {
   return code >= VK_ALPHA_START && code <= VK_ALPHA_END;
}

static bool IsKeyVcr(uint16_t code) {
   return code == VK_PLAY ||
      code == VK_STOP ||
      code == VK_PAUSE ||
      code == VK_FAST_FWD ||
      code == VK_REWIND ||
      code == VK_NEXT ||
      code == VK_PREV ||
      code == VK_PLAY_PAUSE;
}

static bool IsKeyScroll(uint16_t code) {
   return code == VK_PAGE_UP ||
       code == VK_PAGE_DOWN;
}
