#ifndef ORB_HTTP_DOWNLOADER_H
#define ORB_HTTP_DOWNLOADER_H

#include <memory>
#include <string>

namespace orb {

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
 * @brief Simple HTTP/HTTPS downloader using raw sockets and BoringSSL.
 *
 * Provides basic HTTP GET functionality. Supports both HTTP and HTTPS.
 */
class HttpDownloader {
public:
    /**
     * @brief Constructor.
     * @param timeoutMs Connection and receive timeout in milliseconds (default: 10000)
     * @param userAgent HTTP User-Agent header value (default: empty, no header sent)
     */
    explicit HttpDownloader(int timeoutMs = 10000, const std::string& userAgent = "");
    ~HttpDownloader() = default;

    // Prevent copying
    HttpDownloader(const HttpDownloader&) = delete;
    HttpDownloader& operator=(const HttpDownloader&) = delete;

    /**
     * @brief Download content from a URL.
     *
     * @param url The URL to download (supports http:// and https://)
     * @return The downloaded object, or nullptr on failure
     */
    std::shared_ptr<DownloadedObject> Download(const std::string& url);

    /**
     * @brief Download content from a host, port, and path.
     *
     * @param host The hostname
     * @param port The port number
     * @param path The path (default: "/")
     * @param useHttps Whether to use HTTPS (default: false)
     * @return The downloaded object, or nullptr on failure
     */
    std::shared_ptr<DownloadedObject> Download(const std::string& host, uint16_t port,
                                                const std::string& path = "/",
                                                bool useHttps = false);

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
     * @param useHttps Output: true if https scheme
     * @return true if parsing succeeded
     */
    bool ParseUrl(const std::string& url, std::string& host, uint16_t& port,
                  std::string& path, bool& useHttps);

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

    /**
     * @brief Perform HTTP download (no TLS).
     */
    std::shared_ptr<DownloadedObject> DownloadHttp(const std::string& host, uint16_t port,
                                                    const std::string& path,
                                                    const std::string& ipAddress);

    /**
     * @brief Perform HTTPS download (with TLS).
     */
    std::shared_ptr<DownloadedObject> DownloadHttps(const std::string& host, uint16_t port,
                                                     const std::string& path,
                                                     const std::string& ipAddress);

    /**
     * @brief Create a TCP socket and connect to server.
     * @param ipAddress The IP address to connect to
     * @param port The port number
     * @return Socket file descriptor, or -1 on failure
     */
    int CreateAndConnectSocket(const std::string& ipAddress, uint16_t port, const std::string& host);

    /**
     * @brief Build HTTP GET request string.
     * @param host The hostname (for Host header)
     * @param path The request path
     * @return The HTTP request string
     */
    std::string BuildHttpRequest(const std::string& host, const std::string& path);

    /**
     * @brief Parse response and create DownloadedObject.
     * @param response The raw HTTP response
     * @return The downloaded object, or nullptr on failure
     */
    std::shared_ptr<DownloadedObject> ParseAndCreateResponse(const std::string& response);

    int m_timeoutMs;
    std::string m_acceptHeader;
    std::string m_userAgent;
};

} // namespace orb

#endif // ORB_HTTP_DOWNLOADER_H
