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

#include <log.h>
#include "websocket_service.h"

#define         LWS_PROTOCOL_LIST_TERM   { NULL, NULL, 0, 0, 0, NULL, 0 }

namespace orb {
namespace networkServices {

static int sNextConnectionId = 0;

// Implementation of WebSocketConnection methods

WebSocketService::WebSocketConnection::WebSocketConnection(struct lws *wsi, const std::string &uri)
    : mWsi(wsi), mUri(uri), mTextBuffer("")
{
    mId = sNextConnectionId++;
}

WebSocketService::WebSocketConnection::~WebSocketConnection() {
    while (!mWriteQueue.empty()) { mWriteQueue.pop(); }
}

void WebSocketService::WebSocketConnection::SendMessage(const std::string &text)
{
    std::vector<uint8_t> data(text.begin(), text.end());
    SendFragment(std::move(data), true, true, false);
}

void WebSocketService::WebSocketConnection::SendFragment(std::vector<uint8_t> &&data,
    bool is_first, bool is_final, bool is_binary)
{
    int protocol = (is_first) ? ((is_binary) ? LWS_WRITE_BINARY : LWS_WRITE_TEXT)
        : LWS_WRITE_CONTINUATION;
    if (!is_final)
    {
        protocol |= LWS_WRITE_NO_FIN;
    }
    struct FragmentWriteInfo fragment = {
        .write_protocol = static_cast<lws_write_protocol>(protocol),
        .data = std::move(data),
    };
    mWriteQueue.emplace(fragment);
    if (mWsi != nullptr)
    {
        lws_callback_on_writable(mWsi);
    } else 
    {
        LOGE("Wsi is null, cannot send data.");
    }
}

void WebSocketService::WebSocketConnection::Close()
{
    struct FragmentWriteInfo fragment = {
        .close = true,
    };
    mWriteQueue.emplace(fragment);
    if (mWsi != nullptr)
    {
        lws_callback_on_writable(mWsi);
    } else 
    {
        LOGE("Wsi is null, cannot send data.");
    }
}

int WebSocketService::WebSocketConnection::GetQueueSize() const
{
    return static_cast<int>(mWriteQueue.size());
}


// Implementation of WebSocketService methods

WebSocketService::WebSocketService(const std::string &protocol_name, int port, bool use_ssl,
                                   const std::string &interface_name) :
    mStop(true),
    mProtocolName(protocol_name),
    mUseSSL(use_ssl),
    mInterfaceName(interface_name),
#if LWS_VERSION_4 == 1
    retry_{.secs_since_valid_ping = SECS_SINCE_VALID_PING, .secs_since_valid_hangup =
               SECS_SINCE_VALID_HANGUP},
#endif
    mProtocols{Protocol(mProtocolName.c_str()), LWS_PROTOCOL_LIST_TERM},
    mContext(nullptr)
{
    LOGI(ENTER);
    mContextInfo =
    {
        .protocols = mProtocols,
        .port = port,
        .options = LWS_SERVER_OPTION_HTTP_HEADERS_SECURITY_BEST_PRACTICES_ENFORCE
            /* | LWS_SERVER_OPTION_VHOST_UPG_STRICT_HOST_CHECK */,
        .vhost_name = VHOST_NAME.c_str(),
#if LWS_VERSION_4 == 1
        .retry_and_idle_policy = &retry_,
#endif
    };
    if (mUseSSL)
    {
        LOGI("Using SSL for WebSocketService");
        mContextInfo.options |= LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
        mContextInfo.ssl_cert_filepath = SSL_CERT_FILEPATH.c_str();
        mContextInfo.ssl_private_key_filepath = SSL_PRIVATE_KEY_FILEPATH.c_str();
    }
    if (!mInterfaceName.empty())
    {
        mContextInfo.iface = mInterfaceName.c_str();
    }
    lws_set_log_level(LLL_ERR | LLL_WARN, nullptr);
    LOGI(LEAVE);
}

bool WebSocketService::Start()
{
    LOGI(ENTER);
    bool ret = false;
    if (mContext == nullptr 
        && (mContext = lws_create_context(&mContextInfo)) != nullptr)
    {
        mStop = false;
        // Create a new thread for Main loop
        mMainThread = std::make_unique<std::thread>(EnterMainLooper, this);
        ret = true;
    }
    LOGI(LEAVE);
    return ret;
}

void WebSocketService::Stop()
{
    LOGI(ENTER);
    {
        LOGI("Stopping ALl WebSocketService Connections...");
        std::lock_guard<std::recursive_mutex> lock(mConnectionsMutex);
        // Close all connectionsClose
        if (mConnections.size() > 0)
        {
            for (auto &it : mConnections)
            {
                it.second->Close();
            }
        }
        mConnections.clear();
    }

    // Wait for the main thread to finish
    LOGI("Stopping WebSocketService main thread...");
    mStop = true;
    // cancel the service
    if (mContext != nullptr)
    {
        lws_cancel_service(mContext);
    }

    // Join the main thread if it is running
    if (mMainThread != nullptr && mMainThread->joinable())
    {
        mMainThread->join();
        mMainThread.reset();
    }
    LOGI(LEAVE);
}

void * WebSocketService::EnterMainLooper(void *instance)
{
    static_cast<WebSocketService *>(instance)->MainLooper();
    return nullptr;
}

void WebSocketService::MainLooper()
{
    LOGI(ENTER);
    while (!mStop)
    {
        LOGI("WebSocketService main loop running (200ms)...");
        // The timeout value is ignored since 4.2
        if (lws_service(mContext, 0) < 0)
        {
            LOGE("lws_service failed, stopping service.");
            std::lock_guard<std::recursive_mutex> lock(mConnectionsMutex);
            mStop = true;
            mConnections.clear();
            lws_cancel_service(mContext);
            break;
        }
     }
    OnServiceStopped();
    LOGI(LEAVE);
}

int WebSocketService::EnterLwsCallback(struct lws *wsi, enum lws_callback_reasons reason,
    void *user, void *in, size_t len)
{
    const struct lws_protocols *protocol = lws_get_protocol(wsi);
    if (protocol != nullptr)
    {
        WebSocketService *server = static_cast<WebSocketService *>(protocol->user);
        return server->LwsCallback(wsi, reason, user, in, len);
    }
    return 0;
}

int WebSocketService::LwsCallback(struct lws *wsi, enum lws_callback_reasons reason,
    void *user, void *in, size_t len)
{
    LOGI("LwsCallback: " << reason);
    int result = 0;
    std::lock_guard<std::recursive_mutex> lock(mConnectionsMutex);
    
    std::unordered_map<void *, std::unique_ptr<WebSocketConnection>>::iterator it;

    // Check if conection exists
    if (reason == LWS_CALLBACK_CLOSED 
        || reason == LWS_CALLBACK_RECEIVE
        || reason == LWS_CALLBACK_SERVER_WRITEABLE)
    {
        // For these reasons, we need to find the connection by user pointer
        it = mConnections.find(user);
        if (it == mConnections.end())
        {
            LOGE("LwsCallback: Connection not found for user: " << user);
            return -1; // User not found
        }
    }
    
    //handle the callback reason
    switch (reason)
    {
        case LWS_CALLBACK_ESTABLISHED: {
            std::string uri = Header(wsi, WSI_TOKEN_GET_URI);
            std::string args = Header(wsi, WSI_TOKEN_HTTP_URI_ARGS);
            if (!args.empty())
            {
                uri = uri + "?" + args;
            }
            auto connection = std::make_unique<WebSocketConnection>(wsi, uri);
            if (!OnConnection(connection.get()))
            {
                result = -1;
                break;
            }
            mConnections[user] = std::move(connection);
            break;
        }

        case LWS_CALLBACK_CLOSED: {
            OnDisconnected(it->second.get());
            mConnections.erase(it);
            break;
        }

        case LWS_CALLBACK_SERVER_WRITEABLE: {
            while (!it->second->mWriteQueue.empty())
            {
                auto fragment = std::move(it->second->mWriteQueue.front());
                it->second->mWriteQueue.pop();
                if (fragment.close)
                {
                    lws_close_reason(wsi, LWS_CLOSE_STATUS_GOINGAWAY, nullptr, 0);
                    result = -1;
                } else {
                    int size = fragment.data.size();
                    fragment.data.insert(fragment.data.begin(), LWS_PRE, ' ');
                    if (lws_write(wsi, fragment.data.data() + LWS_PRE, size,
                        fragment.write_protocol) != size)
                    {
                       result = -1;
                    }
                }
            }
            break;
        }

        case LWS_CALLBACK_RECEIVE: {
            std::vector<uint8_t> data(static_cast<uint8_t *>(in), static_cast<uint8_t *>(in) + len);
            OnFragmentReceived(it->second.get(), std::move(data), lws_is_first_fragment(wsi),
                lws_is_final_fragment(wsi), lws_frame_is_binary(wsi));
            break;
        }

        default: {
            break;
        }
    }
    LOGI("LwsCallback result: " << result);
    return result;
}

void WebSocketService::OnFragmentReceived(WebSocketConnection *connection,
    std::vector<uint8_t> &&data, bool is_first, bool is_final, bool is_binary)
{
    // Convenience default that concatenates fragments before calling OnMessageReceived
    if (!is_binary)
    {
        if (is_first)
        {
            // Clear the text buffer if this is the first fragment
            connection->mTextBuffer.clear();
        }
        connection->mTextBuffer += std::string(reinterpret_cast<char *>(data.data()),
                data.size());

        if (is_final)
        {
            OnMessageReceived(connection, connection->mTextBuffer);
        }
    }
    else
    {
        // Not implemented
        LOGI("WebSocketService::OnFragmentReceived: Binary data received, but not handled.");
    }
}

void WebSocketService::OnMessageReceived(WebSocketConnection *connection, const std::string &text)
{
    // Possibly called by the implementation of OnFragmentReceived
}

struct lws_protocols WebSocketService::Protocol(const char *protocol_name)
{
    return
        {
            .name = protocol_name,
            .callback = EnterLwsCallback,
            .per_session_data_size = 1,
            .rx_buffer_size = RX_BUFFER_SIZE,
            .id = 0,
            .user = this,
            .tx_packet_size = 0
        };
}

std::string WebSocketService::Header(struct lws *wsi, enum lws_token_indexes header)
{
    int length = lws_hdr_total_length(wsi, header);
    if (length > 0)
    {
        auto buffer = std::unique_ptr<char[]>(new char[length + 1]);
        if (lws_hdr_copy(wsi, buffer.get(), length + 1, header) == length)
        {
            return std::string(buffer.get(), length);
        }
    }
    return "";
}

void WebSocketService::ReleaseService() 
{
    if (mContext != nullptr)
    {
        LOGI("WebSocketService main loop stopped, destroying context.");
        lws_context_destroy(mContext);
        mContext = nullptr;
    }
}

WebSocketService::~WebSocketService() {
    LOGI(ENTER);
    // Stop the service if it is running
    if (!mStop)
    {
        Stop();
    }
    ReleaseService();
    LOGI(LEAVE);
}

WebSocketService::WebSocketConnection* WebSocketService::GetConnection(int id)
{
    for (auto &connection : mConnections)
    {
        if (connection.second->mId == id)
        {
            return connection.second.get();
        }
    }
    return nullptr;
}

} // namespace networkServices
   
} // namespace orb
