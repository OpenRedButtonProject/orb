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
 *
 * HTTP Downloader Interface
 */
#ifndef I_HTTP_DOWNLOADER_H
#define I_HTTP_DOWNLOADER_H

#include <memory>
#include <string>
#include <filesystem>

namespace orb
{

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
 * @brief Interface for HTTP downloading, enabling dependency injection and testing.
 */
class IHttpDownloader {
public:
    virtual ~IHttpDownloader() = default;

    /**
     * @brief Download content from a URL.
     *
     * @param url The URL to download (supports http:// and https://)
     * @return The downloaded object, or nullptr on failure
     */
    virtual std::shared_ptr<DownloadedObject> Download(const std::string& url) = 0;

    /**
     * @brief Download content from a URL to a file.
     *
     * @param url The URL to download
     * @param outputPath The path to save the downloaded content
     * @return The downloaded object (with metadata), or nullptr on failure
     */
    virtual std::shared_ptr<DownloadedObject> DownloadToFile(
        const std::string& url, const std::filesystem::path& outputPath) = 0;
};

} // namespace orb

#endif // I_HTTP_DOWNLOADER_H

