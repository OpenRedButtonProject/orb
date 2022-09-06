/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#include "app2app_local_service.h"
#include "service_manager.h"
#include "log.h"

#include <algorithm>

#define LOCAL_TYPE "local"
#define REMOTE_TYPE "remote"
#define PAIRING_COMPLETED_MESSAGE "pairingcompleted"

namespace NetworkServices {
App2AppLocalService::App2AppLocalService(ServiceManager *manager, int local_port, int remote_port) :
    WebSocketService("", local_port, false, "lo"),
    manager_(manager),
    remote_service_(this, remote_port),
    service_stopped_(false),
    remote_service_stopped_(false)
{
    LOG(LOG_INFO, "App2AppLocalService ctor.\n");
    remote_service_.Start();
}

bool App2AppLocalService::OnConnection(WebSocketConnection *connection)
{
    std::string app_endpoint = GetAppEndPoint(connection->Uri());
    LOG(LOG_INFO, "OnConnection %s", app_endpoint.c_str());
    if (app_endpoint.empty())
    {
        return false;
    }
    // Maybe pair local connection with waiting remote connection
    mutex_.lock();
    if (service_stopped_ || remote_service_stopped_)
    {
        mutex_.unlock();
        return false;
    }
    WebSocketConnection *remote_connection = GetNextWaitingConnection(REMOTE_TYPE, app_endpoint);
    if (remote_connection != nullptr)
    {
        LOG(LOG_INFO, "Pair local (%p) to waiting remote (%p)",
            connection, remote_connection);
        remote_connection->paired_connection_ = connection;
        connection->paired_connection_ = remote_connection;
        connection->SendMessage(PAIRING_COMPLETED_MESSAGE);
        remote_connection->SendMessage(PAIRING_COMPLETED_MESSAGE);
    }
    else
    {
        LOG(LOG_INFO, "Add local waiting connection (%p)", connection);
        AddWaitingConnection(LOCAL_TYPE, app_endpoint, connection);
    }
    mutex_.unlock();

    return true;
}

void App2AppLocalService::OnFragmentReceived(WebSocketConnection *connection,
    std::vector<uint8_t> &&data, bool is_first, bool is_final, bool is_binary)
{
    mutex_.lock();
    if (service_stopped_ || remote_service_stopped_)
    {
        mutex_.unlock();
        return;
    }
    if (connection->paired_connection_ != nullptr)
    {
        connection->paired_connection_->SendFragment(std::move(data), is_first, is_final,
            is_binary);
    }
    mutex_.unlock();
}

void App2AppLocalService::OnDisconnected(WebSocketConnection *connection)
{
    std::string app_endpoint = GetAppEndPoint(connection->Uri());
    LOG(LOG_INFO, "OnDisconnected %s", app_endpoint.c_str());
    if (app_endpoint.empty())
    {
        return;
    }
    mutex_.lock();
    if (service_stopped_ || remote_service_stopped_)
    {
        mutex_.unlock();
        return;
    }
    if (connection->paired_connection_ != nullptr)
    {
        connection->paired_connection_->paired_connection_ = nullptr;
        connection->paired_connection_->Close();
        connection->paired_connection_ = nullptr;
    }
    else
    {
        RemoveWaitingConnection(LOCAL_TYPE, app_endpoint, connection);
    }
    mutex_.unlock();
}

bool App2AppLocalService::OnRemoteConnection(WebSocketConnection *connection)
{
    std::string app_endpoint = GetAppEndPoint(connection->Uri());
    LOG(LOG_INFO, "OnRemoteConnection %s", app_endpoint.c_str());
    if (app_endpoint.empty())
    {
        return false;
    }
    // Maybe pair remote connection with waiting local connection
    mutex_.lock();
    if (service_stopped_ || remote_service_stopped_)
    {
        mutex_.unlock();
        return false;
    }
    WebSocketConnection *local_connection = GetNextWaitingConnection(LOCAL_TYPE, app_endpoint);
    if (local_connection != nullptr)
    {
        LOG(LOG_INFO, "Pair remote (%p) to waiting local (%p)",
            connection, local_connection);
        local_connection->paired_connection_ = connection;
        connection->paired_connection_ = local_connection;
        connection->SendMessage(PAIRING_COMPLETED_MESSAGE);
        local_connection->SendMessage(PAIRING_COMPLETED_MESSAGE);
    }
    else
    {
        LOG(LOG_INFO, "Add remote waiting connection (%p)", connection);
        AddWaitingConnection(REMOTE_TYPE, app_endpoint, connection);
    }
    mutex_.unlock();
    return true;
}

void App2AppLocalService::OnRemoteFragmentReceived(WebSocketConnection *connection,
    std::vector<uint8_t> &&data, bool is_first, bool is_final, bool is_binary)
{
    mutex_.lock();
    if (service_stopped_ || remote_service_stopped_)
    {
        mutex_.unlock();
        return;
    }
    if (connection->paired_connection_ != nullptr)
    {
        connection->paired_connection_->SendFragment(std::move(data), is_first, is_final,
            is_binary);
    }
    mutex_.unlock();
}

void App2AppLocalService::OnRemoteDisconnected(WebSocketConnection *connection)
{
    std::string app_endpoint = GetAppEndPoint(connection->Uri());
    LOG(LOG_INFO, "OnRemoteDisconnected %s", app_endpoint.c_str());
    if (app_endpoint.empty())
    {
        return;
    }
    mutex_.lock();
    if (service_stopped_ || remote_service_stopped_)
    {
        mutex_.unlock();
        return;
    }
    if (connection->paired_connection_ != nullptr)
    {
        connection->paired_connection_->paired_connection_ = nullptr;
        connection->paired_connection_->Close();
        connection->paired_connection_ = nullptr;
    }
    else
    {
        RemoveWaitingConnection(REMOTE_TYPE, app_endpoint, connection);
    }
    mutex_.unlock();
}

void App2AppLocalService::Stop()
{
    mutex_.lock();
    if (service_stopped_ || remote_service_stopped_)
    {
        mutex_.unlock();
        return;
    }
    if (!remote_service_stopped_)
    {
        remote_service_stopped_ = true;
        remote_service_.Stop();
    }
    else
    {
        WebSocketService::Stop();
    }
    mutex_.unlock();
}

void App2AppLocalService::OnServiceStopped()
{
    mutex_.lock();
    service_stopped_ = true;
    if (!remote_service_stopped_)
    {
        remote_service_stopped_ = true;
        remote_service_.Stop();
    }
    else
    {
        WebSocketService::OnServiceStopped();
    }
    mutex_.unlock();
}

void App2AppLocalService::OnRemoteServiceStopped()
{
    mutex_.lock();
    remote_service_stopped_ = true;
    if (!service_stopped_)
    {
        service_stopped_ = true;
        WebSocketService::Stop();
    }
    else
    {
        manager_->OnServiceStopped(this);
    }
    mutex_.unlock();
}

std::string App2AppLocalService::GetAppEndPoint(const std::string &uri)
{
    if (uri.find("/hbbtv/") == 0)
    {
        return uri.substr(7);
    }
    return "";
}

void App2AppLocalService::AddWaitingConnection(const std::string &type,
    const std::string &app_endpoint, WebSocketConnection *connection)
{
    std::string key = type + app_endpoint;
    waiting_connections_[key].push_back(connection);
}

void App2AppLocalService::RemoveWaitingConnection(const std::string &type,
    const std::string &app_endpoint, WebSocketConnection *connection)
{
    std::string key = type + app_endpoint;
    auto it = waiting_connections_.find(key);
    if (it != waiting_connections_.end())
    {
        auto &queue = it->second;
        queue.erase(std::remove(queue.begin(), queue.end(), connection), queue.end());
        if (queue.empty())
        {
            waiting_connections_.erase(it);
        }
    }
}

WebSocketService::WebSocketConnection * App2AppLocalService::GetNextWaitingConnection(
    const std::string &type, const std::string &app_endpoint)
{
    WebSocketService::WebSocketConnection *connection = nullptr;
    std::string key = type + app_endpoint;
    auto it = waiting_connections_.find(key);
    if (it != waiting_connections_.end())
    {
        auto &queue = it->second;
        if (!queue.empty())
        {
            connection = queue.front();
            queue.pop_front();
            if (queue.empty())
            {
                waiting_connections_.erase(it);
            }
        }
    }
    return connection;
}
} // namespace NetworkServices
