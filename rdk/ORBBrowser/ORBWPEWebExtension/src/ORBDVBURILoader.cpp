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
    ORB_LOG("requestId=%d requestUri=%s", m_requestId, webkit_uri_scheme_request_get_uri(
        m_request));

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
        GError *error = g_error_new(g_quark_from_string(failedUri.c_str()), 0,
            errorDescription.c_str());
        webkit_uri_scheme_request_finish_error(m_request, error);
        g_error_free(error);

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
