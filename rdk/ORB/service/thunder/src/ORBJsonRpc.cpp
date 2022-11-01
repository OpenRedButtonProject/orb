/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#include "Module.h"
#include "ORB.h"
#include "ORBLogging.h"

#include <vector>

namespace WPEFramework {
namespace Plugin {
/**
 * @brief ORB::RegisterAll
 *
 * Register all JSON-RPC methods exposed by the ORB plugin.
 */
void ORB::RegisterAll()
{
   ORB_LOG("PID=%d", getpid());
   JSONRPC::Register<Core::JSON::DecUInt16, Core::JSON::Boolean>(_T("SendKeyEvent"), &ORB::SendKeyEvent, this);
   JSONRPC::Register<Core::JSON::String, void>(_T("SetPreferredUILanguage"), &ORB::SetPreferredUILanguage, this);
}

/**
 * @brief ORB::UnregisterAll
 *
 * Unregister all JSON-RPC methods exposed by the ORB plugin.
 */
void ORB::UnregisterAll()
{
   ORB_LOG("PID=%d", getpid());
   JSONRPC::Unregister(_T("SendKeyEvent"));
   JSONRPC::Unregister(_T("SetPreferredUILanguage"));
}

/**
 * @brief ORB::SendKeyToCurrentApplication
 *
 * Send the specified key event to the current HbbTV application (if any).
 *
 * @param keyCode  The event's JavaScript key code
 * @param response True if the key event was consumed by the current HbbTV application, otherwise false
 *
 * @return Core::ERROR_NONE
 */
uint32_t ORB::SendKeyEvent(Core::JSON::DecUInt16 keyCode, Core::JSON::Boolean& response)
{
   uint32_t error = Core::ERROR_NONE;

   if (keyCode.IsSet())
   {
      SYSLOG(Logging::Notification, (_T("[ORB::SendKeyEvent] keyCode=%hu"), keyCode.Value()));
      response = _orb->SendKeyEvent(keyCode.Value());
   }
   else
   {
      error = Core::ERROR_BAD_REQUEST;
   }

   return error;
}

/**
 * @brief ORB::SetPreferredUILanguage
 *
 * Set the preferred UI language.
 *
 * @param preferredUiLanguage The preferred UI language.
 *                            A comma-separated set of languages to be used for the user interface
 *                            of a service, in order of preference. Each language shall be indicated
 *                            by its ISO 639-2 language code as defined in [ISO639-2].
 *
 * @return Core::ERROR_NONE
 */
uint32_t ORB::SetPreferredUILanguage(Core::JSON::String preferredUiLanguage)
{
   SYSLOG(Logging::Notification, (_T("[ORB::SetPreferredUILanguage] preferredUiLanguage=%s"), preferredUiLanguage.Value().c_str()));
   uint32_t result = Core::ERROR_NONE;
   _orb->SetPreferredUILanguage(preferredUiLanguage.Value());
   return result;
}

} // namespace Plugin
} // namespace WPEFramework
