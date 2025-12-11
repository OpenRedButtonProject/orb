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

#ifndef ORB_HTTP_DOWNLOADER_H
#define ORB_HTTP_DOWNLOADER_H

#include <memory>
#include <string>

namespace orb {

/**
 * @brief Simple HTTP downloader using raw sockets.
 *
 * Provides basic HTTP GET functionality without external dependencies.
 */
class HttpDownloader {
public:
    /**
     * @brief Representation of downloaded content.
     */
    class DownloadedObject {
    public:
        DownloadedObject(const std::string& content, const std::string& contentType, int statusCode);
        ~DownloadedObject() = default;

        std::string GetContent() const { return m_content; }
        std::string GetContentType() const { return m_contentType; }
        int GetStatusCode() const { return m_statusCode; }
        bool IsSuccess() const { return m_statusCode >= 200 && m_statusCode < 300; }

    private:
        std::string m_content;
        std::string m_contentType;
        int m_statusCode;
    };

    /**
     * @brief Constructor.
     * @param timeoutMs Connection and receive timeout in milliseconds (default: 10000)
     */
    explicit HttpDownloader(int timeoutMs = 10000);
    ~HttpDownloader() = default;

    // Prevent copying
    HttpDownloader(const HttpDownloader&) = delete;
    HttpDownloader& operator=(const HttpDownloader&) = delete;

    /**
     * @brief Download content from a URL.
     *
     * @param url The URL to download (e.g., "http://example.com/path")
     * @return The downloaded object, or nullptr on failure
     */
    std::shared_ptr<DownloadedObject> Download(const std::string& url);

    /**
     * @brief Download content from a host, port, and path.
     *
     * @param host The hostname
     * @param port The port number
     * @param path The path (default: "/")
     * @return The downloaded object, or nullptr on failure
     */
    std::shared_ptr<DownloadedObject> Download(const std::string& host, uint16_t port,
                                                const std::string& path = "/");

    /**
     * @brief Set custom Accept header value.
     * @param acceptHeader The Accept header value
     */
    void SetAcceptHeader(const std::string& acceptHeader) { m_acceptHeader = acceptHeader; }

private:
    /**
     * @brief Resolve hostname to IP address.
     * @param hostname The hostname to resolve
     * @return IP address string, or empty on failure
     */
    std::string ResolveHostname(const std::string& hostname);

    /**
     * @brief Parse URL into components.
     * @param url The URL to parse
     * @param host Output: hostname
     * @param port Output: port number
     * @param path Output: path
     * @return true if parsing succeeded
     */
    bool ParseUrl(const std::string& url, std::string& host, uint16_t& port, std::string& path);

    /**
     * @brief Parse HTTP response headers and extract metadata.
     * @param response The raw HTTP response
     * @param statusCode Output: HTTP status code
     * @param contentType Output: Content-Type header value
     * @param bodyStart Output: offset where body begins
     * @return true if parsing succeeded
     */
    bool ParseResponseHeaders(const std::string& response, int& statusCode,
                              std::string& contentType, size_t& bodyStart);

    int m_timeoutMs;
    std::string m_acceptHeader;
};

} // namespace orb

#endif // ORB_HTTP_DOWNLOADER_H

