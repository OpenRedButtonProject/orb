#pragma once

#include "Module.h"
#include <interfaces/IORB.h>

#include "ORBGenericClient.h"

using namespace WPEFramework;

/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */
#ifndef __ORB_LOGGING_H__
#define __ORB_LOGGING_H__

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string>
#include <cstring>

/* Simple file name, i.e. without the full path */
#define SIMPLE_FILE_NAME strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__

/**
 * Produce a log entry with the specified message and arguments.
 */
#define ORB_LOG(msg, ...) do \
   { \
      fprintf(stderr, "ORB [%s]::[%s]::[%d] ", SIMPLE_FILE_NAME, __FUNCTION__, __LINE__); \
      fprintf(stderr, msg, ##__VA_ARGS__); \
      fprintf(stderr, "\n"); \
   } \
   while (0)

/**
 * Produce a log entry without any message or arguments.
 */
#define ORB_LOG_NO_ARGS() do \
   { \
      fprintf(stderr, "ORB [%s]::[%s]::[%d]\n", SIMPLE_FILE_NAME, __FUNCTION__, __LINE__); \
   } \
   while (0)


#endif // __ORB_LOGGING__

namespace orb
{
class ORBComRpcClient
{
   // We want do run our own custom code when the plugin raises a notification
   // Implement the INotification class to do what we want
   class NotificationHandler : public Exchange::IORB::INotification
   {
      virtual void JavaScriptEventDispatchRequest(
         std::string name,
         std::string properties,
         bool broadcastRelated,
         std::string targetOrigin
      ) override;

      virtual void DvbUrlLoaded(
         int requestId,
         const uint8_t* fileContent, 
         const uint16_t fileContentLength
      ) override;

      virtual void EventInputKeyGenerated(int keyCode) override;

      // Must define an interface map since we are implementing an interface on the exchange
      // so Thunder knows what type we are
      BEGIN_INTERFACE_MAP(NotificationHandler)
      INTERFACE_ENTRY(Exchange::IORB::INotification)
      END_INTERFACE_MAP
   };

   public:
      ORBComRpcClient();
      ~ORBComRpcClient();

   public:

      // actual calls
      std::string ExecuteBridgeRequest(std::string request);
      std::string CreateToken(std::string uri);
      void NotifyApplicationLoadFailed(std::string url, std::string errorDescription);
      void NotifyApplicationPageChanged(std::string url);
      bool SendKeyEvent(int keyCode);
      void LoadDvbUrl(std::string url, int requestId);
   
      bool IsValid();

   private:
    Core::NodeId GetConnectionEndpoint();

   private:
      Core::NodeId m_remoteConnection;
      
      // An engine that can serialize/deserialize the COMRPC messages. Can be configured
      // to tune performance:
      // 1 = Number of threads allocated to this connection
      // 0 = Stack size per thread
      // 4 = Message slots. 4 which means that if 4 messages have
      // been queued, the submission of the 5th element will be a
      // blocking call until there is a free slot again
      Core::ProxyType<RPC::InvokeServerType<1, 0, 4>> m_engine;

      Core::ProxyType<RPC::CommunicatorClient> m_client;

      // The implementation of the ISamplePlugin interface - this could be an in-process, out-of-process
      // or distributed plugin, but as a client we don't need to worry about that
      Exchange::IORB *_orb;
      PluginHost::IShell *m_controller;

      // used to store if valid after connection
      bool m_valid;

      // Instance of our notification handler
      Core::Sink<NotificationHandler> m_notification;

}; // class ORBComRpcClient 
}  // namespace orb