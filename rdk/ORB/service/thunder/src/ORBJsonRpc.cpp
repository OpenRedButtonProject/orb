/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#include "Module.h"
#include "ORB.h"
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
   fprintf(stderr, "RegisterAll call %d\n", getpid());
   JSONRPC::Register<Core::JSON::DecUInt16, Core::JSON::Boolean>(_T("SendKeyEvent"), &ORB::SendKeyEvent, this);
}

/**
 * @brief ORB::UnregisterAll
 *
 * Unregister all JSON-RPC methods exposed by the ORB plugin.
 */
void ORB::UnregisterAll()
{
   fprintf(stderr, "UNRegisterAll call %d\n", getpid());
   JSONRPC::Unregister(_T("SendKeyEvent"));
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

} // namespace Plugin
} // namespace WPEFramework
