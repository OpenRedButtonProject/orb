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

#pragma once

#include <memory>
#include <string>

namespace orb {
/**
 * @brief orb::HttpDownloader
 *
 * CURL-based implementation of a simple HTTP downloader.
 */
class HttpDownloader {
public:

    /**
     * Representation of downloaded objects.
     */
    class DownloadedObject {
public:
        DownloadedObject(std::string content, std::string contentType);
        ~DownloadedObject();
        std::string GetContent() const;
        std::string GetContentType() const;
private:
        std::string m_content;
        std::string m_contentType;
    }; // class DownloadedObject

    HttpDownloader();
    ~HttpDownloader();

    /**
     * @brief HttpDownloader::Download
     *
     * Download the content of the specified URL.
     *
     * @param url The URL to download the content from
     *
     * @return The downloaded object
     */
    std::shared_ptr<DownloadedObject> Download(const std::string& url);

private:

    void *m_curl;
}; // class HttpDownloader
} // namespace orb
