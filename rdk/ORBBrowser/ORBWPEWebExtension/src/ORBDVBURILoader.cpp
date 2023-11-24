/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "ORBDVBURILoader.h"
#include "ORBWPEWebExtensionHelper.h"
#include "ORBLogging.h"
#include <string>

namespace orb
{
static std::string dvbUrlBase = "";

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
    ORB_LOG("requestId=%d requestUri=%s", m_requestId, webkit_uri_scheme_request_get_uri(
        m_request));

    std::string uri = webkit_uri_scheme_request_get_uri(m_request);

    // handle hbbtv-carousel:// scheme
    if (uri.rfind("hbbtv-carousel://", 0) == 0)
    {
        // in case of initial loading, get the base dvburl
        std::size_t pos = uri.find("?dvburl");
        if (pos != std::string::npos)
        {
            dvbUrlBase = uri.substr(pos + 8, uri.length() - 1);
        }

        std::string path = "";
        pos = uri.find('/', 17);
        if (pos != std::string::npos)
        {
            path = uri.substr(pos);
        }
        uri = dvbUrlBase + path;
        ORB_LOG("Requesting dvburi: %s", uri.c_str());
    }

    ORBWPEWebExtensionHelper::GetSharedInstance().GetORBClient()->LoadDvbUrl(
        uri,
        m_requestId
        );

    return;
}

/**
 * Finish the load process by dispatching the retrieved content to the browser.
 */
void ORBDVBURILoader::Finish()
{
    ORB_LOG("requestId=%d requestUri=%s", m_requestId, webkit_uri_scheme_request_get_uri(
        m_request));

    if (m_data)
    {
        ORB_LOG("DVB URI scheme request completed with data");

        GInputStream *stream = nullptr;
        unsigned char *data = (unsigned char *)malloc(sizeof(unsigned char) * m_dataLength);
        memmove(data, m_data, m_dataLength);
        stream = g_memory_input_stream_new_from_data((const void *)data, m_dataLength, free);

        if (stream)
        {
            ORB_LOG("GInputStream created with dataLength=%d", m_dataLength);
        }

        const gchar *mimeType = nullptr;

        // Signal completion of the DVB URI scheme request
        webkit_uri_scheme_request_finish(m_request, stream, m_dataLength, mimeType);

        g_object_unref(stream);
    }
    else
    {
        std::string errorDescription = "DVB URI scheme request failed";
        std::string failedUri(webkit_uri_scheme_request_get_uri(m_request));
        if (failedUri.back() == '/')
        {
            failedUri.pop_back();
        }

        ORB_LOG("DVB URI scheme request completed without any data");

        GInputStream *inputStream = g_memory_input_stream_new();
        WebKitURISchemeResponse *response = webkit_uri_scheme_response_new(inputStream, 0);
        webkit_uri_scheme_response_set_status(response, 404, "DVB URI scheme request failed");
        webkit_uri_scheme_request_finish_with_response(m_request, response);
        g_object_unref(inputStream);

        std::string currentAppUrl =
            ORBWPEWebExtensionHelper::GetSharedInstance().GetORBClient()->GetCurrentAppUrl();
        if (currentAppUrl.back() == '/')
        {
            currentAppUrl.pop_back();
        }

        if (currentAppUrl == failedUri)
        {
            ORBWPEWebExtensionHelper::GetSharedInstance().GetORBClient()->
            NotifyApplicationLoadFailed(failedUri, errorDescription);
        }
    }

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
    if (data != nullptr && dataLength > 0)
    {
        m_data = (unsigned char *) malloc(sizeof(unsigned char) * (dataLength + 1));
        for (int i = 0; i < dataLength; i++)
        {
            m_data[i] = data[i];
        }
        m_data[dataLength] = '\0';
        m_dataLength = dataLength;
    }
}
} // namespace orb
