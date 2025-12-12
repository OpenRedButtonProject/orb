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

#include "HttpDownloader.h"
#include "log.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <cstring>
#include <sstream>

namespace orb {

namespace {
    constexpr size_t RECEIVE_BUFFER_SIZE = 8192;
    constexpr size_t MAX_RESPONSE_SIZE = 1024 * 1024; // 1MB
    constexpr uint16_t DEFAULT_HTTP_PORT = 80;
    constexpr uint16_t DEFAULT_HTTPS_PORT = 443;
}

// DownloadedObject implementation

DownloadedObject::DownloadedObject(
    const std::string& content,
    const std::string& contentType,
    int statusCode)
    : m_content(content)
    , m_contentType(contentType)
    , m_statusCode(statusCode)
{
}

// HttpDownloader implementation

HttpDownloader::HttpDownloader(int timeoutMs)
    : m_timeoutMs(timeoutMs)
    , m_acceptHeader("*/*")
{
}

std::string HttpDownloader::ResolveHostname(const std::string& hostname)
{
    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int status = getaddrinfo(hostname.c_str(), nullptr, &hints, &result);
    if (status != 0) {
        LOG(ERROR) << "Failed to resolve hostname " << hostname << ": " << gai_strerror(status);
        return "";
    }

    char ipStr[INET_ADDRSTRLEN];
    struct sockaddr_in* addr = reinterpret_cast<struct sockaddr_in*>(result->ai_addr);
    inet_ntop(AF_INET, &(addr->sin_addr), ipStr, INET_ADDRSTRLEN);

    freeaddrinfo(result);
    return std::string(ipStr);
}

bool HttpDownloader::ParseUrl(const std::string& url, std::string& host,
                               uint16_t& port, std::string& path)
{
    // Default values
    port = DEFAULT_HTTP_PORT;
    path = "/";

    std::string remaining = url;

    // Remove protocol prefix
    if (remaining.find("http://") == 0) {
        remaining = remaining.substr(7);
        port = DEFAULT_HTTP_PORT;
    } else if (remaining.find("https://") == 0) {
        remaining = remaining.substr(8);
        port = DEFAULT_HTTPS_PORT;
        // Note: HTTPS not supported with raw sockets
        LOG(WARNING) << "HTTPS not supported, attempting plain HTTP";
        port = DEFAULT_HTTP_PORT;
    }

    // Find path separator
    size_t pathStart = remaining.find('/');
    if (pathStart != std::string::npos) {
        path = remaining.substr(pathStart);
        remaining = remaining.substr(0, pathStart);
    }

    // Check for port
    size_t portStart = remaining.find(':');
    if (portStart != std::string::npos) {
        std::string portStr = remaining.substr(portStart + 1);
        host = remaining.substr(0, portStart);

        // Parse port manually (no exceptions)
        port = 0;
        for (char c : portStr) {
            if (c < '0' || c > '9') {
                LOG(ERROR) << "Invalid port in URL: " << url;
                return false;
            }
            port = port * 10 + (c - '0');
        }
    } else {
        host = remaining;
    }

    if (host.empty()) {
        LOG(ERROR) << "Empty host in URL: " << url;
        return false;
    }

    return true;
}

bool HttpDownloader::ParseResponseHeaders(const std::string& response, int& statusCode,
                                           std::string& contentType, size_t& bodyStart)
{
    // Find header/body separator
    bodyStart = response.find("\r\n\r\n");
    if (bodyStart != std::string::npos) {
        bodyStart += 4;
    } else {
        bodyStart = response.find("\n\n");
        if (bodyStart != std::string::npos) {
            bodyStart += 2;
        } else {
            LOG(ERROR) << "Invalid HTTP response: no header/body separator";
            return false;
        }
    }

    std::string headers = response.substr(0, bodyStart);

    // Parse status line (e.g., "HTTP/1.1 200 OK")
    size_t statusStart = headers.find(' ');
    if (statusStart == std::string::npos || statusStart + 4 > headers.length()) {
        LOG(ERROR) << "Invalid HTTP response: no status code";
        return false;
    }

    // Extract and validate status code
    std::string statusStr = headers.substr(statusStart + 1, 3);
    for (char c : statusStr) {
        if (c < '0' || c > '9') {
            LOG(ERROR) << "Invalid HTTP response: non-numeric status code";
            return false;
        }
    }
    statusCode = (statusStr[0] - '0') * 100 +
                 (statusStr[1] - '0') * 10 +
                 (statusStr[2] - '0');

    // Parse Content-Type header
    contentType = "";
    std::string contentTypeHeader = "Content-Type:";
    size_t ctPos = headers.find(contentTypeHeader);
    if (ctPos == std::string::npos) {
        // Try lowercase
        contentTypeHeader = "content-type:";
        ctPos = headers.find(contentTypeHeader);
    }

    if (ctPos != std::string::npos) {
        size_t valueStart = ctPos + contentTypeHeader.length();
        // Skip leading whitespace
        while (valueStart < headers.length() &&
               (headers[valueStart] == ' ' || headers[valueStart] == '\t')) {
            valueStart++;
        }
        size_t valueEnd = headers.find_first_of("\r\n", valueStart);
        if (valueEnd != std::string::npos) {
            contentType = headers.substr(valueStart, valueEnd - valueStart);
            // Remove optional parameters (e.g., "; charset=utf-8")
            size_t semicolon = contentType.find(';');
            if (semicolon != std::string::npos) {
                contentType = contentType.substr(0, semicolon);
            }
        }
    }

    return true;
}

std::shared_ptr<DownloadedObject> HttpDownloader::Download(const std::string& url)
{
    std::string host;
    uint16_t port;
    std::string path;

    if (!ParseUrl(url, host, port, path)) {
        return nullptr;
    }

    return Download(host, port, path);
}

std::shared_ptr<DownloadedObject> HttpDownloader::Download(
    const std::string& host, uint16_t port, const std::string& path)
{
    LOG(INFO) << "HttpDownloader: GET " << host << ":" << port << path;

    // Resolve hostname
    std::string ipAddress = ResolveHostname(host);
    if (ipAddress.empty()) {
        return nullptr;
    }

    // Create TCP socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        LOG(ERROR) << "Failed to create socket: " << strerror(errno);
        return nullptr;
    }

    // Set timeouts
    struct timeval tv;
    tv.tv_sec = m_timeoutMs / 1000;
    tv.tv_usec = (m_timeoutMs % 1000) * 1000;

    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0 ||
        setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) < 0) {
        LOG(ERROR) << "Failed to set socket timeout: " << strerror(errno);
        close(sock);
        return nullptr;
    }

    // Connect
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, ipAddress.c_str(), &serverAddr.sin_addr);

    if (connect(sock, reinterpret_cast<struct sockaddr*>(&serverAddr), sizeof(serverAddr)) < 0) {
        LOG(ERROR) << "Failed to connect to " << host << ":" << port << ": " << strerror(errno);
        close(sock);
        return nullptr;
    }

    // Build and send request
    std::ostringstream request;
    request << "GET " << path << " HTTP/1.1\r\n";
    request << "Host: " << host << "\r\n";
    request << "Accept: " << m_acceptHeader << "\r\n";
    request << "Connection: close\r\n";
    request << "\r\n";

    std::string requestStr = request.str();
    if (send(sock, requestStr.c_str(), requestStr.length(), 0) < 0) {
        LOG(ERROR) << "Failed to send request: " << strerror(errno);
        close(sock);
        return nullptr;
    }

    // Receive response
    std::string response;
    char buffer[RECEIVE_BUFFER_SIZE];

    while (response.length() < MAX_RESPONSE_SIZE) {
        ssize_t received = recv(sock, buffer, sizeof(buffer) - 1, 0);

        if (received < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                LOG(ERROR) << "Request timed out";
            } else {
                LOG(ERROR) << "Failed to receive response: " << strerror(errno);
            }
            close(sock);
            return nullptr;
        }

        if (received == 0) {
            break; // Connection closed
        }

        response.append(buffer, received);
    }

    close(sock);

    if (response.empty()) {
        LOG(ERROR) << "Empty response";
        return nullptr;
    }

    // Parse response
    int statusCode;
    std::string contentType;
    size_t bodyStart;

    if (!ParseResponseHeaders(response, statusCode, contentType, bodyStart)) {
        return nullptr;
    }

    std::string body = (bodyStart < response.length()) ? response.substr(bodyStart) : "";

    LOG(INFO) << "HttpDownloader: status=" << statusCode
              << " contentType=" << contentType
              << " bodySize=" << body.length();

    return std::make_shared<DownloadedObject>(body, contentType, statusCode);
}

} // namespace orb
