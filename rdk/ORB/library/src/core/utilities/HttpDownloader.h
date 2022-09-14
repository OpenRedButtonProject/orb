/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
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
