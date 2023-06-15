/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */


#include "websocket_service.h"

#define         LWS_PROTOCOL_LIST_TERM   { NULL, NULL, 0, 0, 0, NULL, 0 }

namespace NetworkServices {
#define VHOST_NAME "localhost"
#define SSL_CERT_FILEPATH "todo.cert"
#define SSL_PRIVATE_KEY_FILEPATH "todo.key"
#define SECS_SINCE_VALID_PING 3
#define SECS_SINCE_VALID_HANGUP 10
#define RX_BUFFER_SIZE 4096

static int next_unique_id_ = 0;

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
    write_queue_.emplace(fragment);
    lws_callback_on_writable(wsi_);
}

void WebSocketService::WebSocketConnection::Close()
{
    struct FragmentWriteInfo fragment = {
        .close = true,
    };
    write_queue_.emplace(fragment);
    lws_callback_on_writable(wsi_);
}

WebSocketService::WebSocketService(const std::string &protocol_name, int port, bool use_ssl,
                                   const std::string &interface_name) :
    stop_(true),
    protocol_name_(protocol_name),
    use_ssl_(use_ssl),
    interface_name_(interface_name),
    retry_{.secs_since_valid_ping = SECS_SINCE_VALID_PING, .secs_since_valid_hangup =
               SECS_SINCE_VALID_HANGUP},
    protocols_{Protocol(protocol_name_.c_str()), LWS_PROTOCOL_LIST_TERM},
    context_(nullptr)
{
    info_ =
    {
        .protocols = protocols_,
        .port = port,
        .options = LWS_SERVER_OPTION_HTTP_HEADERS_SECURITY_BEST_PRACTICES_ENFORCE
            /* | LWS_SERVER_OPTION_VHOST_UPG_STRICT_HOST_CHECK */,
        .vhost_name = VHOST_NAME,
        .retry_and_idle_policy = &retry_,
    };
    if (use_ssl_)
    {
        info_.options |= LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
        info_.ssl_cert_filepath = SSL_CERT_FILEPATH;
        info_.ssl_private_key_filepath = SSL_PRIVATE_KEY_FILEPATH;
    }
    if (!interface_name_.empty())
    {
        info_.iface = interface_name_.c_str();
    }
    lws_set_log_level(LLL_ERR | LLL_WARN /*| LLL_NOTICE*/, nullptr);
}

bool WebSocketService::Start()
{
    bool ret = false;
    connections_mutex_.lock();
    if (context_ == nullptr && (context_ = lws_create_context(&info_)) != nullptr)
    {
        stop_ = false;
        pthread_t thread;
        pthread_create(&thread, nullptr, EnterMainLooper, this);
        connections_mutex_.unlock();
        ret = true;
    }
    connections_mutex_.unlock();
    return ret;
}

void WebSocketService::Stop()
{
    connections_mutex_.lock();
    stop_ = true;
    if (connections_.size() > 0)
    {
        for (auto &it : connections_)
        {
            it.second->Close();
        }
    }
    else if (context_ != nullptr)
    {
        lws_cancel_service(context_);
    }
    connections_mutex_.unlock();
}

void * WebSocketService::EnterMainLooper(void *instance)
{
    static_cast<WebSocketService *>(instance)->MainLooper();
    return nullptr;
}

void WebSocketService::MainLooper()
{
    connections_mutex_.lock();
    while (!stop_ || connections_.size() > 0)
    {
        connections_mutex_.unlock();
        if (lws_service(context_, 0) < 0)
        {
            connections_mutex_.lock();
            stop_ = true;
            connections_.clear();
            lws_cancel_service(context_);
            break;
        }
        connections_mutex_.lock();
    }
    if (context_ != nullptr)
    {
        lws_context_destroy(context_);
        context_ = nullptr;
    }
    connections_mutex_.unlock();
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
    connections_mutex_.lock();
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
            connections_[user] = std::move(connection);
            break;
        }

        case LWS_CALLBACK_CLOSED: {
            auto it = connections_.find(user);
            if (it == connections_.end())
            {
                result = -1;
                break;
            }
            OnDisconnected(it->second.get());
            connections_.erase(it);

            if (stop_ && connections_.size() <= 0 && context_ != nullptr)
            {
                lws_cancel_service(context_);
            }
            break;
        }

        case LWS_CALLBACK_SERVER_WRITEABLE: {
            auto it = connections_.find(user);
            if (it == connections_.end())
            {
                result = -1;
                break;
            }
            while (!it->second->write_queue_.empty())
            {
                auto fragment = std::move(it->second->write_queue_.front());
                it->second->write_queue_.pop();
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
            auto it = connections_.find(user);
            if (it == connections_.end())
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
    connections_mutex_.unlock();
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
            connection->text_buffer_ = std::string(reinterpret_cast<char *>(data.data()),
                data.size());
        }
        else
        {
            connection->text_buffer_ += std::string(reinterpret_cast<char *>(data.data()),
                data.size());
        }

        if (is_final)
        {
            OnMessageReceived(connection, connection->text_buffer_);
        }
    }
    else
    {
        // Not implemented
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

WebSocketService::WebSocketConnection::WebSocketConnection(struct lws *wsi, const std::string &uri)
    : wsi_(wsi), uri_(uri), paired_connection_(nullptr)
{
    unique_id_ = next_unique_id_++;
}

} // namespace NetworkServices
