/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#include "HttpDownloader.h"
#include "ORBLogging.h"
#include <curl/curl.h>
#include <curl/easy.h>
#include <sstream>
#include <iostream>

/**
 * Callback method used by curl to write the downloaded data.
 *
 * @param ptr
 * @param size
 * @param nmemb
 * @param stream
 *
 * @return
 */
size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
   std::string data((const char *) ptr, (size_t) size * nmemb);
   *((std::stringstream *) stream) << data << std::endl;
   return size * nmemb;
}

namespace orb {
/**
 * Constructor.
 *
 * @param content The content of the downloaded object
 * @param contentType The content type of the downloaded object
 */
HttpDownloader::DownloadedObject::DownloadedObject(std::string content, std::string contentType)
   : m_content(content)
   , m_contentType(contentType)
{
}

/**
 * Destructor.
 */
HttpDownloader::DownloadedObject::~DownloadedObject()
{
}

/**
 * Gets the content of the downloaded object.
 *
 * @return The content
 */
std::string HttpDownloader::DownloadedObject::GetContent() const
{
   return m_content;
}

/**
 * Gets the content type of the downloaded object.
 *
 * @return The content type
 */
std::string HttpDownloader::DownloadedObject::GetContentType() const
{
   return m_contentType;
}

// HttpDownloader

/**
 * Constructor.
 */
HttpDownloader::HttpDownloader()
{
   m_curl = curl_easy_init();
}

/**
 * Destructor.
 */
HttpDownloader::~HttpDownloader()
{
   curl_easy_cleanup(m_curl);
}

/**
 * @brief HttpDownloader::Download
 *
 * Download the content of the specified URL.
 *
 * @param url The URL to download the content from
 *
 * @return The downloaded object
 */
std::shared_ptr<HttpDownloader::DownloadedObject> HttpDownloader::Download(const std::string& url)
{
   ORB_LOG("url=%s", url.c_str());

   std::shared_ptr<HttpDownloader::DownloadedObject> downloadedObject = nullptr;
   std::string content;
   std::string contentType;
   std::stringstream out;

   curl_easy_setopt(m_curl, CURLOPT_URL, url.c_str());
   curl_easy_setopt(m_curl, CURLOPT_FOLLOWLOCATION, 1L);
   curl_easy_setopt(m_curl, CURLOPT_NOSIGNAL, 1);
   curl_easy_setopt(m_curl, CURLOPT_ACCEPT_ENCODING, "deflate");
   curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, write_data);
   curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &out);

   // Perform the request and check for any errors
   CURLcode res = curl_easy_perform(m_curl);
   if (res != CURLE_OK)
   {
      ORB_LOG("curl_easy_perform() failed: %s", curl_easy_strerror(res));
      return downloadedObject;
   }
   content = out.str();

   // Extract and evaluate the content type
   char *ct = NULL;
   res = curl_easy_getinfo(m_curl, CURLINFO_CONTENT_TYPE, &ct);
   if (res != CURLE_OK || ct == NULL)
   {
      ORB_LOG("Could not resolve content type of downloaded object");
      return downloadedObject;
   }
   ORB_LOG("content type of downloaded object is: %s", ct);
   contentType = ct;

   downloadedObject = std::make_shared<HttpDownloader::DownloadedObject>(content, contentType);
   return downloadedObject;
}
} // namespace orb
