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

#ifndef OBS_NS_WEBSOCKET_SERVICE_H
#define OBS_NS_WEBSOCKET_SERVICE_H

#include "service_manager.h"

#include <string>
#include <unordered_map>
#include <memory>
#include <queue>
#include <mutex>
#include <libwebsockets.h>
#include <thread> 

namespace orb {
namespace networkServices {

const std::string VHOST_NAME = "localhost";
const std::string SSL_CERT_FILEPATH = "todo.cert";
const std::string SSL_PRIVATE_KEY_FILEPATH = "todo.key";
constexpr int SECS_SINCE_VALID_PING = 3;
constexpr int SECS_SINCE_VALID_HANGUP = 10;
constexpr int RX_BUFFER_SIZE = 4096;

class WebSocketService : public ServiceManager::Service {
public:
    class WebSocketConnection
    {
        friend class WebSocketService;

public:
        
        WebSocketConnection(struct lws *wsi, const std::string &uri);
        virtual ~WebSocketConnection();
        
        std::string Uri() const
        {
            return mUri;
        }

        int Id() const
        {
            return mId;
        }

        void SendMessage(const std::string &text);
        void SendFragment(std::vector<uint8_t> &&data, bool is_first, bool is_final, bool
            is_binary);
        void Close();
        int GetQueueSize() const;
     
protected:
        struct FragmentWriteInfo
        {
            lws_write_protocol write_protocol;
            std::vector<uint8_t> data;
            bool close;
        };
        // Disallow copy and assign
        WebSocketConnection(const WebSocketConnection&) = delete;
        WebSocketConnection& operator=(const WebSocketConnection&) = delete;

 private:
        struct lws* mWsi;
        std::string mUri;
        std::string mTextBuffer;
        std::queue<struct FragmentWriteInfo> mWriteQueue;
        int mId;
       
    }; // class WebSocketConnection

    WebSocketService(const std::string &server_name, int port, bool use_ssl, const
        std::string &interface_name);
    virtual ~WebSocketService();
    virtual bool Start();
    virtual void Stop();
    virtual bool OnConnection(WebSocketConnection *connection) = 0;
    virtual void OnDisconnected(WebSocketConnection *connection) = 0;
    virtual void OnFragmentReceived(WebSocketConnection *connection, std::vector<uint8_t> &&data,
        bool is_first, bool is_final, bool is_binary);
    virtual void OnMessageReceived(WebSocketConnection *connection, const std::string &text);

protected:
    std::recursive_mutex mConnectionsMutex;
    std::unordered_map<void *, std::unique_ptr<WebSocketConnection> > mConnections;
    WebSocketConnection* GetConnection(int id);
    void ReleaseService();
 
private:
    static void* EnterMainLooper(void *instance);
    void MainLooper();
    static int EnterLwsCallback(struct lws *wsi, enum lws_callback_reasons reason, void *user,
        void *in, size_t len);
    int LwsCallback(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t
        len);
    struct lws_protocols Protocol(const char *protocol_name);
    std::string Header(struct lws *wsi, enum lws_token_indexes header);

    bool mStop;
    std::string mProtocolName;
    bool mUseSSL;
    std::string mInterfaceName;
 #if LWS_VERSION_4 == 1   
    lws_retry_bo_t retry_;
 #endif   
    struct lws_protocols mProtocols[2];
    struct lws_context_creation_info mContextInfo;
    struct lws_context *mContext;
    std::unique_ptr<std::thread> mMainThread;
};
} // namespace networkServices
} // namespace orb

#endif // OBS_NS_WEBSOCKET_SERVICE_H

