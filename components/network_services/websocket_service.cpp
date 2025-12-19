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

#define LOG_TAG "ORB/WS_Server"

#include "log.h"
#include "websocket_service.h"

#define         LWS_PROTOCOL_LIST_TERM   { NULL, NULL, 0, 0, 0, NULL, 0 }

namespace NetworkServices {

static int sNextConnectionId = 0;

// Implementation of WebSocketConnection methods

WebSocketService::WebSocketConnection::WebSocketConnection(struct lws *wsi, const std::string &uri)
    : mWsi(wsi), mUri(uri), mTextBuffer(""), mPairedConnection(nullptr)
{
    mId = sNextConnectionId++;
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
    lws_callback_on_writable(mWsi);
}

bool WebSocketService::WebSocketConnection::ClosePaired()
{
    if (mPairedConnection ==  nullptr)
    {
        return false;
    }
    mPairedConnection->mPairedConnection = nullptr;
    mPairedConnection->Close();
    mPairedConnection = nullptr;
    return true;
}

void WebSocketService::WebSocketConnection::Close()
{
    struct FragmentWriteInfo fragment = {
        .close = true,
    };
    mWriteQueue.emplace(fragment);
    lws_callback_on_writable(mWsi);
}

// Custom logging callback to route libwebsockets logs to Android logcat
static void lws_log_to_logcat(int level, const char *line)
{
    // Map libwebsockets log levels to Android log levels
    android_LogPriority android_level;
    switch (level) {
        case LLL_ERR:
            android_level = ANDROID_LOG_ERROR;
            break;
        case LLL_WARN:
            android_level = ANDROID_LOG_WARN;
            break;
        case LLL_NOTICE:
        case LLL_INFO:
            android_level = ANDROID_LOG_INFO;
            break;
        case LLL_DEBUG:
        case LLL_PARSER:
        case LLL_HEADER:
        case LLL_EXT:
        case LLL_CLIENT:
        case LLL_LATENCY:
        case LLL_USER:
        default:
            android_level = ANDROID_LOG_DEBUG;
            break;
    }

    // Remove trailing newline if present (libwebsockets includes it)
    size_t len = strlen(line);
    if (len > 0 && line[len - 1] == '\n') {
        len--;
    }
    __android_log_print(android_level, "libwebsockets", "%.*s", (int)len, line);
}

WebSocketService::WebSocketService(const std::string &protocol_name, int port, bool use_ssl,
                                   const std::string &interface_name) :
    mStop(true),
    mProtocolName(protocol_name),
    mUseSSL(use_ssl),
    mInterfaceName(interface_name),
#if LWS_LIBRARY_VERSION_NUMBER > 4000000
    mRetry{.secs_since_valid_ping = SECS_SINCE_VALID_PING, .secs_since_valid_hangup =
               SECS_SINCE_VALID_HANGUP},
#endif
    mProtocols{Protocol(mProtocolName.c_str()), LWS_PROTOCOL_LIST_TERM},
    mContext(nullptr)
{
    mContextInfo =
    {
        .protocols = mProtocols,
        .port = port,
        .options = LWS_SERVER_OPTION_HTTP_HEADERS_SECURITY_BEST_PRACTICES_ENFORCE
            /* | LWS_SERVER_OPTION_VHOST_UPG_STRICT_HOST_CHECK */,
        .vhost_name = VHOST_NAME.c_str(),
#if LWS_LIBRARY_VERSION_NUMBER > 4000000
        .retry_and_idle_policy = &mRetry,
#endif
    };
    if (mUseSSL)
    {
        mContextInfo.options |= LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
        mContextInfo.ssl_cert_filepath = SSL_CERT_FILEPATH.c_str();
        mContextInfo.ssl_private_key_filepath = SSL_PRIVATE_KEY_FILEPATH.c_str();
    }
    if (!mInterfaceName.empty())
    {
        mContextInfo.iface = mInterfaceName.c_str();
    }
    // Enable LibWebSockets logging and route to Android logcat via custom callback
    // Enable: ERR, WARN, NOTICE, INFO,
    // but not: DEBUG, CLIENT, PARSER, HEADER, EXT, LATENCY
    lws_set_log_level( LLL_ERR | LLL_WARN | LLL_NOTICE | LLL_INFO
                     // | LLL_DEBUG | LLL_CLIENT | LLL_PARSER | LLL_HEADER | LLL_EXT | LLL_LATENCY
                     , lws_log_to_logcat);

    LOGD("created WebSocketService")
}

bool WebSocketService::Start()
{
    bool ret = false;
    mConnectionsMutex.lock();
    if (mContext == nullptr
        && (mContext = lws_create_context(&mContextInfo)) != nullptr)
    {
        mStop = false;
        pthread_t thread;
        pthread_create(&thread, nullptr, EnterMainLooper, this);
        mConnectionsMutex.unlock();
        ret = true;
    }
    mConnectionsMutex.unlock();
    return ret;
}

void WebSocketService::Stop()
{
    LOGD("Stopping")
    mConnectionsMutex.lock();
    mStop = true;
    if (mConnections.size() > 0)
    {
        for (auto &it : mConnections)
        {
            it.second->Close();
        }
    }
    else if (mContext != nullptr)
    {
        lws_cancel_service(mContext);
    }
    mConnectionsMutex.unlock();
    LOGD("Stopped")
}

void * WebSocketService::EnterMainLooper(void *instance)
{
    static_cast<WebSocketService *>(instance)->MainLooper();
    return nullptr;
}

void WebSocketService::MainLooper()
{
    mConnectionsMutex.lock();
    while (!mStop || mConnections.size() > 0)
    {
        mConnectionsMutex.unlock();
        if (lws_service(mContext, 0) < 0)
        {
            mConnectionsMutex.lock();
            mStop = true;
            mConnections.clear();
            lws_cancel_service(mContext);
            break;
        }
        mConnectionsMutex.lock();
    }
    if (mContext != nullptr)
    {
        lws_context_destroy(mContext);
        mContext = nullptr;
    }
    mConnectionsMutex.unlock();
    OnServiceStopped();
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
    int result = 0;
    mConnectionsMutex.lock();
    switch (reason)
    {
        case LWS_CALLBACK_PROTOCOL_INIT: {
            break;
        }

        case LWS_CALLBACK_ESTABLISHED: {
            std::string uri = Header(wsi, WSI_TOKEN_GET_URI);
            std::string args = Header(wsi, WSI_TOKEN_HTTP_URI_ARGS);
            if (!args.empty())
            {
                uri = uri + "?" + args;
            }
            auto connection = std::unique_ptr<WebSocketConnection>(
                new WebSocketConnection(wsi, uri));
            if (!OnConnection(connection.get()))
            {
                result = -1;
                break;
            }
            mConnections[user] = std::move(connection);
            break;
        }

        case LWS_CALLBACK_CLOSED: {
            auto it = mConnections.find(user);
            if (it == mConnections.end())
            {
                result = -1;
                break;
            }
            OnDisconnected(it->second.get());
            mConnections.erase(it);

            if (mStop && mConnections.size() <= 0 && mContext != nullptr)
            {
                lws_cancel_service(mContext);
            }
            break;
        }

        case LWS_CALLBACK_SERVER_WRITEABLE: {
            auto it = mConnections.find(user);
            if (it == mConnections.end())
            {
                result = -1;
                break;
            }
            while (!it->second->mWriteQueue.empty())
            {
                auto fragment = std::move(it->second->mWriteQueue.front());
                it->second->mWriteQueue.pop();
                if (fragment.close)
                {
                    lws_close_reason(wsi, LWS_CLOSE_STATUS_GOINGAWAY, nullptr, 0);
                    result = -1;
                    break;
                }
                int size = fragment.data.size();
                fragment.data.insert(fragment.data.begin(), LWS_PRE, ' ');
                if (lws_write(wsi, fragment.data.data() + LWS_PRE, size,
                    fragment.write_protocol) != size)
                {
                    result = -1;
                    break;
                }
            }
            break;
        }

        case LWS_CALLBACK_RECEIVE: {
            auto it = mConnections.find(user);
            if (it == mConnections.end())
            {
                result = -1;
                break;
            }
            std::vector<uint8_t> data(static_cast<uint8_t *>(in), static_cast<uint8_t *>(in) + len);
            OnFragmentReceived(it->second.get(), std::move(data), lws_is_first_fragment(wsi),
                lws_is_final_fragment(wsi), lws_frame_is_binary(wsi));
            break;
        }

        default: {
            break;
        }
    }
    mConnectionsMutex.unlock();
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
            connection->mTextBuffer = std::string(reinterpret_cast<char *>(data.data()),
                data.size());
        }
        else
        {
            connection->mTextBuffer += std::string(reinterpret_cast<char *>(data.data()),
                data.size());
        }

        if (is_final)
        {
            OnMessageReceived(connection, connection->mTextBuffer);
        }
    }
    else
    {
        // Not implemented
        LOGI("Binary data received, but not handled.");
    }
}

void WebSocketService::OnMessageReceived(WebSocketConnection *connection, const std::string &text)
{
    // Possibly called by the implementation of OnFragmentReceived
}

void WebSocketService::UpdateClient(WebSocketConnection *connection)
{
    // Overriden by sub class
}

void WebSocketService::OnUpdateClients()
{
    // Overriden by sub class
}

void WebSocketService::UpdateClients()
{
    for (auto const &connection : mConnections)
    {
        UpdateClient(connection.second.get());
    }
    OnUpdateClients();
}

int WebSocketService::TotalClients() const
{
    int total;
    total = mConnections.size();
    return total;
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

void WebSocketService::WssMutexLock()
{
    mConnectionsMutex.lock();
}

void WebSocketService::WssMutexUnlock()
{
    mConnectionsMutex.unlock();
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


} // namespace NetworkServices
