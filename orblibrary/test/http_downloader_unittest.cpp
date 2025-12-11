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

#include <string>

#include "testing/gtest/include/gtest/gtest.h"
#include "HttpDownloader.h"

using namespace orb;

class HttpDownloaderTest : public ::testing::Test {
protected:
    void SetUp() override {
    }

    void TearDown() override {
    }
};

// =============================================================================
// DownloadedObject Tests
// =============================================================================

TEST_F(HttpDownloaderTest, TestDownloadedObject_Construction)
{
    // GIVEN/WHEN: creating a DownloadedObject
    HttpDownloader::DownloadedObject obj("test content", "text/plain", 200);

    // THEN: all fields should be accessible
    EXPECT_EQ(obj.GetContent(), "test content");
    EXPECT_EQ(obj.GetContentType(), "text/plain");
    EXPECT_EQ(obj.GetStatusCode(), 200);
    EXPECT_TRUE(obj.IsSuccess());
}

TEST_F(HttpDownloaderTest, TestDownloadedObject_Success2xx)
{
    // GIVEN: various 2xx status codes
    HttpDownloader::DownloadedObject obj200("", "", 200);
    HttpDownloader::DownloadedObject obj201("", "", 201);
    HttpDownloader::DownloadedObject obj204("", "", 204);

    // THEN: all should report success
    EXPECT_TRUE(obj200.IsSuccess());
    EXPECT_TRUE(obj201.IsSuccess());
    EXPECT_TRUE(obj204.IsSuccess());
}

TEST_F(HttpDownloaderTest, TestDownloadedObject_FailureNon2xx)
{
    // GIVEN: various non-2xx status codes
    HttpDownloader::DownloadedObject obj301("", "", 301);
    HttpDownloader::DownloadedObject obj400("", "", 400);
    HttpDownloader::DownloadedObject obj404("", "", 404);
    HttpDownloader::DownloadedObject obj500("", "", 500);

    // THEN: all should report failure
    EXPECT_FALSE(obj301.IsSuccess());
    EXPECT_FALSE(obj400.IsSuccess());
    EXPECT_FALSE(obj404.IsSuccess());
    EXPECT_FALSE(obj500.IsSuccess());
}

// =============================================================================
// HttpDownloader Construction Tests
// =============================================================================

TEST_F(HttpDownloaderTest, TestConstruction_DefaultTimeout)
{
    // GIVEN/WHEN: creating HttpDownloader with default timeout
    HttpDownloader downloader;

    // THEN: should be constructed successfully
    // (No direct way to verify timeout, but construction should succeed)
    SUCCEED();
}

TEST_F(HttpDownloaderTest, TestConstruction_CustomTimeout)
{
    // GIVEN/WHEN: creating HttpDownloader with custom timeout
    HttpDownloader downloader(5000);

    // THEN: should be constructed successfully
    SUCCEED();
}

TEST_F(HttpDownloaderTest, TestSetAcceptHeader)
{
    // GIVEN: an HttpDownloader
    HttpDownloader downloader;

    // WHEN: setting a custom Accept header
    downloader.SetAcceptHeader("application/vnd.dvb.ait+xml");

    // THEN: should succeed (header will be used in subsequent requests)
    SUCCEED();
}

// =============================================================================
// URL Parsing Tests (tested indirectly through Download)
// =============================================================================

TEST_F(HttpDownloaderTest, TestDownload_EmptyUrl)
{
    // GIVEN: an HttpDownloader
    HttpDownloader downloader;

    // WHEN: downloading from an empty URL
    auto result = downloader.Download("");

    // THEN: result should be null
    EXPECT_EQ(result, nullptr);
}

TEST_F(HttpDownloaderTest, TestDownload_InvalidUrl_NoHost)
{
    // GIVEN: an HttpDownloader
    HttpDownloader downloader;

    // WHEN: downloading from a URL with no host
    auto result = downloader.Download("http:///path");

    // THEN: result should be null
    EXPECT_EQ(result, nullptr);
}

// =============================================================================
// Download Tests - Connection failures
// =============================================================================

TEST_F(HttpDownloaderTest, TestDownload_UnresolvableHost)
{
    // GIVEN: an HttpDownloader with short timeout
    HttpDownloader downloader(1000);

    // WHEN: downloading from an unresolvable host
    auto result = downloader.Download("http://this-host-does-not-exist-12345.invalid/");

    // THEN: result should be null
    EXPECT_EQ(result, nullptr);
}

TEST_F(HttpDownloaderTest, TestDownload_ConnectionRefused)
{
    // GIVEN: an HttpDownloader with short timeout
    HttpDownloader downloader(1000);

    // WHEN: downloading from a host/port that refuses connections
    // Using localhost on an unlikely port
    auto result = downloader.Download("localhost", 54321, "/");

    // THEN: result should be null (connection refused or timeout)
    EXPECT_EQ(result, nullptr);
}

// =============================================================================
// Disabled Tests - Useful for manual/integration testing
// =============================================================================

// Disabled - useful for manual testing with real server
TEST_F(HttpDownloaderTest, DISABLED_TestDownload_RealServer)
{
    // GIVEN: an HttpDownloader
    HttpDownloader downloader;

    // WHEN: downloading from a real server
    auto result = downloader.Download("http://example.com/");

    // THEN: should get a successful response
    ASSERT_NE(result, nullptr);
    EXPECT_TRUE(result->IsSuccess());
    EXPECT_EQ(result->GetStatusCode(), 200);
    EXPECT_FALSE(result->GetContent().empty());
}

// Disabled - useful for manual testing with host/port/path
TEST_F(HttpDownloaderTest, DISABLED_TestDownload_HostPortPath)
{
    // GIVEN: an HttpDownloader
    HttpDownloader downloader;

    // WHEN: downloading using host, port, path
    auto result = downloader.Download("example.com", 80, "/");

    // THEN: should get a successful response
    ASSERT_NE(result, nullptr);
    EXPECT_TRUE(result->IsSuccess());
}

// Disabled - useful for testing custom Accept header
TEST_F(HttpDownloaderTest, DISABLED_TestDownload_CustomAcceptHeader)
{
    // GIVEN: an HttpDownloader with custom Accept header
    HttpDownloader downloader;
    downloader.SetAcceptHeader("application/json");

    // WHEN: downloading from a server that respects Accept header
    auto result = downloader.Download("http://httpbin.org/get");

    // THEN: should get a successful response
    ASSERT_NE(result, nullptr);
    EXPECT_TRUE(result->IsSuccess());
}

