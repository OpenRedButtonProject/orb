/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#include "Module.h"
#include "ORBBrowser.h"

#include <interfaces/json/JsonData_Browser.h>
#include <interfaces/json/JsonData_StateControl.h>

namespace WPEFramework {
namespace Plugin {
using namespace JsonData::Browser;
using namespace JsonData::StateControl;
using namespace WPEFramework::Exchange;

// Registration
//

void ORBBrowser::RegisterAll()
{
   Property<Core::JSON::EnumType<StateType> >(_T("state"), &ORBBrowser::get_state, &ORBBrowser::set_state, this);     /* StateControl */
   Property<Core::JSON::ArrayType<Core::JSON::String> >(_T("languages"), &ORBBrowser::get_languages, &ORBBrowser::set_languages, this);
   Property<Core::JSON::ArrayType<JsonData::WebKitBrowser::HeadersData> >(_T("headers"), &ORBBrowser::get_headers, &ORBBrowser::set_headers, this);
   Register<DeleteParamsData, void>(_T("delete"), &ORBBrowser::endpoint_delete, this);
}

void ORBBrowser::UnregisterAll()
{
   Unregister(_T("state"));
   Unregister(_T("headers"));
   Unregister(_T("languages"));
   Unregister(_T("delete"));
}

// API implementation
//

// Method: endpoint_delete - delete dir
// Return codes:
//  - ERROR_NONE: Success
uint32_t ORBBrowser::endpoint_delete(const DeleteParamsData& params)
{
   return DeleteDir(params.Path.Value());
}

// Property: languages - Browser prefered languages
// Return codes:
//  - ERROR_NONE: Success
uint32_t ORBBrowser::get_languages(Core::JSON::ArrayType<Core::JSON::String>& response) const
{
   ASSERT(_application != nullptr);

   string langs;
   static_cast<const IApplication *>(_application)->Language(langs);
   response.FromString(langs);

   return Core::ERROR_NONE;
}

// Property: languages - Browser prefered languages
// Return codes:
//  - ERROR_NONE: Success
uint32_t ORBBrowser::set_languages(const Core::JSON::ArrayType<Core::JSON::String>& param)
{
   ASSERT(_application != nullptr);

   string langs;
   if (param.IsSet())
   {
      param.ToString(langs);
   }
   _application->Language(static_cast<const string>(langs));

   return Core::ERROR_NONE;
}

// Property: headers - Headers to send on all requests that the browser makes
// Return codes:
//  - ERROR_NONE: Success
uint32_t ORBBrowser::get_headers(Core::JSON::ArrayType<JsonData::WebKitBrowser::HeadersData>& response) const
{
   ASSERT(_browser != nullptr);
   string headers;
   static_cast<const IWebBrowser *>(_browser)->HeaderList(headers);

   response.FromString(headers);
   return Core::ERROR_NONE;
}

// Property: headers - Headers to send on all requests that the browser makes
// Return codes:
//  - ERROR_NONE: Success
uint32_t ORBBrowser::set_headers(const Core::JSON::ArrayType<JsonData::WebKitBrowser::HeadersData>& param)
{
   ASSERT(_browser != nullptr);

   string headers;

   if (param.IsSet())
   {
      param.ToString(headers);
   }

   _browser->HeaderList(static_cast<const string>(headers));
   return Core::ERROR_NONE;
}

// Property: state - Running state of the service
// Return codes:
//  - ERROR_NONE: Success
uint32_t ORBBrowser::get_state(Core::JSON::EnumType<StateType>& response) const     /* StateControl */
{
   ASSERT(_browser != nullptr);

   PluginHost::IStateControl* stateControl(_browser->QueryInterface<PluginHost::IStateControl>());

   // In the mean time an out-of-process plugin might have crashed and thus return a nullptr.
   if (stateControl != nullptr)
   {
      PluginHost::IStateControl::state currentState = stateControl->State();
      response = (currentState == PluginHost::IStateControl::SUSPENDED ? StateType::SUSPENDED : StateType::RESUMED);

      stateControl->Release();
   }

   return Core::ERROR_NONE;
}

// Property: state - Running state of the service
// Return codes:
//  - ERROR_NONE: Success
uint32_t ORBBrowser::set_state(const Core::JSON::EnumType<StateType>& param)     /* StateControl */
{
   ASSERT(_browser != nullptr);

   uint32_t result = Core::ERROR_BAD_REQUEST;

   if (param.IsSet())
   {
      PluginHost::IStateControl* stateControl(_browser->QueryInterface<PluginHost::IStateControl>());

      // In the mean time an out-of-process plugin might have crashed and thus return a nullptr.
      if (stateControl != nullptr)
      {
         stateControl->Request(param == StateType::SUSPENDED ? PluginHost::IStateControl::SUSPEND : PluginHost::IStateControl::RESUME);

         stateControl->Release();
      }

      result = Core::ERROR_NONE;
   }

   return result;
}

// Event: statechange - Signals a state change of the service
void ORBBrowser::event_statechange(const bool& suspended)     /* StateControl */
{
   StatechangeParamsData params;
   params.Suspended = suspended;

   Notify(_T("statechange"), params);
}

// Event: bridgequery - A message from legacy $badger bridge
void ORBBrowser::event_bridgequery(const string& message)
{
   Core::JSON::String params;
   params = message;
   Notify(_T("bridgequery"), params);
}
} // namespace Plugin
} // namespace WPEFramework
