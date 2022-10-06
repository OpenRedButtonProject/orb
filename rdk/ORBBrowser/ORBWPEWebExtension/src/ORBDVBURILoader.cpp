/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */
#include "ORBDVBURILoader.h"
#include "ORBWPEWebExtensionHelper.h"
#include "ORBLogging.h"

namespace orb
{
/**
 * Constructor.
 *
 * @param requestId The request identifier
 * @param request   The DVB URI scheme request
 */
ORBDVBURILoader::ORBDVBURILoader(int requestId, WebKitURISchemeRequest *request)
   : m_requestId(requestId)
   , m_request(request)
   , m_dataReady(false)
   , m_data(nullptr)
   , m_dataLength(0)
{
   ORB_LOG("requestId=%d requestUri=%s", requestId, webkit_uri_scheme_request_get_uri(request));
}

/**
 * Destructor.
 */
ORBDVBURILoader::~ORBDVBURILoader()
{
   ORB_LOG_NO_ARGS();
   m_request = nullptr;
   if (m_data)
   {
      free(m_data);
   }
}

/**
 * Start the load process by sending an asynchronous request to the ORB service.
 */
void ORBDVBURILoader::StartAsync()
{
   ORB_LOG("requestId=%d requestUri=%s", m_requestId, webkit_uri_scheme_request_get_uri(m_request));

   ORBWPEWebExtensionHelper::GetSharedInstance().GetORBClient()->LoadDvbUrl(
      webkit_uri_scheme_request_get_uri(m_request),
      m_requestId
      );

   return;
}

/**
 * Finish the load process by dispatching the retrieved content to the browser.
 */
void ORBDVBURILoader::Finish()
{
   ORB_LOG("requestId=%d requestUri=%s", m_requestId, webkit_uri_scheme_request_get_uri(m_request));

   GInputStream *stream = nullptr;
   gchar *data = g_strdup_printf((char *)m_data);
   int dataLength = strlen(data);
   stream = g_memory_input_stream_new_from_data(data, dataLength, g_free);

   ORB_LOG("GInputStream created");

   // TODO Resolve mime type
   const gchar *mimeType = nullptr;//"text/html";

   // Signal completion of the DVB URI scheme request
   webkit_uri_scheme_request_finish(m_request, stream, dataLength, mimeType);

   g_object_unref(stream);
   ORB_LOG("Completed");

   return;
}

/**
 * Set the loaded data.
 *
 * @param data       The data
 * @param dataLength The data length
 */
void ORBDVBURILoader::SetData(unsigned char *data, unsigned int dataLength)
{
   ORB_LOG("dataLength=%u", dataLength);
   if (m_data)
   {
      free(m_data);
   }
   m_data = (unsigned char *) malloc(dataLength);
   memcpy(m_data, data, dataLength);
   m_dataLength = dataLength;
}
} // namespace orb
