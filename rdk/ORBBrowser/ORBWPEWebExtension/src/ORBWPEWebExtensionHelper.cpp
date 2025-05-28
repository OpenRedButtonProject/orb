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
#include "ORBWPEWebExtensionHelper.h"
#include "ORBDVBURILoader.h"
#include "ORBLogging.h"

using namespace orb;

#include <thread>
#include <map>
#include <mutex>
#include <condition_variable>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <fstream>
#include <streambuf>

// JavaScript files to be injected
#define ORB_HBBTV_JS_PATH  "/usr/share/WPEFramework/ORBBrowser/hbbtv.js"
#define ORB_DASH_JS_PATH   "/usr/share/WPEFramework/ORBBrowser/dash.all.min.js"

// Home directory of the ORB WPE web extension
#define ORB_WPE_WEB_EXTENSION_HOME "/usr/lib/orb"

// static properties for thread synchronisation
static std::mutex m;
static std::condition_variable cv;

// Map of currently active DVB URL loaders
static std::map<int, std::shared_ptr<ORBDVBURILoader> > s_dvbUriLoaders;

/**
 * Helper function to get the file extension from the given url
 *
 * @param url The url to parse
 *
 * @return The extension or empty string if no extension given
 */
static std::string GetFileExtensionFromUrl(std::string url) {
    // skip scheme
    size_t protocolPos = url.find("://");
    if (protocolPos != std::string::npos)
    {
        url = url.substr(protocolPos + 3);
    }
    
    // only interested in the file ext
    size_t tokenPos = url.find("?");
    if (tokenPos != std::string::npos) 
    {
        url = url.substr(0, tokenPos);
    }
    
    // path/to/fileDir<lastSlashPos>file.ext
    size_t lastSlashPos = url.find_last_of('/');
    if (lastSlashPos != std::string::npos)
    {
        url = url.substr(lastSlashPos + 1);
        if (url.length() > 0)
        {
            size_t lastDotPos = url.find_last_of('.');
            if (lastDotPos != std::string::npos) 
            {
                url = url.substr(lastDotPos + 1);
            }
            else 
            {
                url = "";
            }
        }
    }
    else
    {
        url = "";
    }
    
    return url;
}

/**
 * Read the specified file contents into a string buffer.
 *
 * @param filePath The absolute file path
 *
 * @return The string buffer
 */
static std::string ReadFileContentsIntoString(std::string filePath)
{
    std::ifstream inputFileStream(filePath);
    std::string buffer((std::istreambuf_iterator<char>(inputFileStream)),
                       std::istreambuf_iterator<char>());
    return buffer;
}

/**
 * Read the available Dsmcc file from the shared memory.
 *
 * @param requestId The corresponding Dsmcc request id
 * @param fileSize  The Dsmcc file size
 *
 * @return The Dsmcc file content
 */
static unsigned char* ReadDsmccFileFromSharedMemory(int requestId, int fileSize)
{
    unsigned char *buffer = (unsigned char *) malloc(fileSize);

    std::string name = "orb-dsmcc-request-" + std::to_string(requestId);

    ORB_LOG("requestId=%d fileName=%s fileSize=%d", requestId, name.c_str(), fileSize);

    int shm_fd = shm_open(name.c_str(), O_RDONLY, 0666);
    uint8_t *ptr = (uint8_t *) mmap(0, fileSize, PROT_READ, MAP_SHARED, shm_fd, 0);

    memmove(buffer, ptr, fileSize);

    munmap(ptr, fileSize);
    shm_unlink(name.c_str());

    close(shm_fd);

    return buffer;
}

/**
 * Callback for the javaScriptEventDispatchRequested event coming from ORB.
 */
static void OnJavaScriptEventDispatchRequested(std::string name, std::string properties)
{
    ORB_LOG("<Not supported>");
}

/**
 * Callback for the dvbUrlLoaded event coming from ORB.
 */
static void OnDvbUrlLoaded(int requestId, unsigned char *content, unsigned int contentLength)
{
    ORB_LOG("requestId=%d contentLength=%u content is %s", requestId, contentLength, content ?
        "NOT null" : "null");
}

/**
 * Callback for the dvbUrlLoadedNoData event coming from ORB.
 */
static void OnDvbUrlLoadedNoData(int requestId, unsigned int contentLength)
{
    ORB_LOG("requestId=%d contentLength=%u", requestId, contentLength);

    unsigned char *content = nullptr;

    // Read file content from shared memory only if the DVB URL was successfully loaded
    if (contentLength > 0)
    {
        ORB_LOG("Read dsmcc file content from shared memory");
        content = ReadDsmccFileFromSharedMemory(requestId, contentLength);
    }

    {
        std::lock_guard<std::mutex> lk(m);
        s_dvbUriLoaders[requestId]->SetData(content, contentLength);
        s_dvbUriLoaders[requestId]->SetDataReady(true);
        if (content)
        {
            free(content);
        }
    }
    cv.notify_one();
}

/**
 * Callback for the inputKeyGenerated event coming from ORB.
 */
static void OnInputKeyGenerated(int keyCode, unsigned char keyAction)
{
    ORB_LOG("<Not supported");
}

/**
 * Callback for the exitButtonPressed event coming from ORB.
 */
static void OnExitButtonPressed()
{
    ORB_LOG("<Not supported>");
}

/**
 * Handle the specified DVB URI scheme request.
 *
 * @param request  The DVB URI scheme request
 * @param userData The user data or nullptr
 */
static void HandleDVBURISchemeRequest(WebKitURISchemeRequest *request, gpointer user_data)
{
    static int requestId = 0;

    ORB_LOG("uri=%s requestId=%d", webkit_uri_scheme_request_get_uri(request), requestId);

    // Create and persist a new loader into the static (shared) map of loaders
    s_dvbUriLoaders[requestId] = std::make_shared<ORBDVBURILoader>(requestId,
        (WebKitURISchemeRequest *) g_object_ref(request));

    // Start the loading process, which is of asynchronous nature
    s_dvbUriLoaders[requestId]->StartAsync();

    // Wait until the ORB notifies us that the DVB URL was loaded
    std::unique_lock<std::mutex> lk(m);
    cv.wait(lk, [] {
        return s_dvbUriLoaders[requestId]->IsDataReady();
    });
    s_dvbUriLoaders[requestId]->SetDataReady(false);
    lk.unlock();
    cv.notify_one();

    // Allow the loading process to finish and cleanup
    s_dvbUriLoaders[requestId]->Finish();
    s_dvbUriLoaders.erase(requestId);

    requestId++;
}

namespace orb
{
/**
 * Constructor.
 */
ORBWPEWebExtensionHelper::ORBWPEWebExtensionHelper()
{
    ORB_LOG_NO_ARGS();
    m_orbClient = CreateORBClient(
        OnJavaScriptEventDispatchRequested,
        OnDvbUrlLoaded,
        OnDvbUrlLoadedNoData,
        OnInputKeyGenerated,
        OnExitButtonPressed
        );

    m_orbClient->SubscribeToDvbUrlLoadedNoDataEvent();
    
    if (m_mimetypeMap.empty()) {
        m_mimetypeMap["txt"] = "text/plain";
        m_mimetypeMap["pdf"] = "application/pdf";
        m_mimetypeMap["ps"] = "application/postscript";
        m_mimetypeMap["css"] = "text/css";
        m_mimetypeMap["html"] = "text/html";
        m_mimetypeMap["htm"] = "text/html";
        m_mimetypeMap["xml"] = "text/xml";
        m_mimetypeMap["xsl"] = "text/xsl";
        m_mimetypeMap["js"] = "application/x-javascript";
        m_mimetypeMap["xht"] = "application/xhtml+xml";
        m_mimetypeMap["xhtml"] = "application/xhtml+xml";
        m_mimetypeMap["rss"] = "application/rss+xml";
        m_mimetypeMap["webarchive"] = "application/x-webarchive";
        m_mimetypeMap["svg"] = "image/svg+xml";
        m_mimetypeMap["svgz"] = "image/svg+xml";
        m_mimetypeMap["jpg"] = "image/jpeg";
        m_mimetypeMap["jpeg"] = "image/jpeg";
        m_mimetypeMap["png"] = "image/png";
        m_mimetypeMap["tif"] = "image/tiff";
        m_mimetypeMap["tiff"] = "image/tiff";
        m_mimetypeMap["ico"] = "image/ico";
        m_mimetypeMap["cur"] = "image/ico";
        m_mimetypeMap["bmp"] = "image/bmp";
        m_mimetypeMap["wml"] = "text/vnd.wap.wml";
        m_mimetypeMap["wmlc"] = "application/vnd.wap.wmlc";
        m_mimetypeMap["m4a"] = "audio/x-m4a";
    }
}

/**
 * Destructor.
 */
ORBWPEWebExtensionHelper::~ORBWPEWebExtensionHelper()
{
    ORB_LOG_NO_ARGS();
    m_orbClient->UnsubscribeFromDvbUrlLoadedNoDataEvent();
    m_mimetypeMap.clear();
}

/**
 * Perform initialisation tasks related to the ORB WPE web extension.
 *
 * @param context Pointer to the WebKit web context
 */
void ORBWPEWebExtensionHelper::InitialiseWebExtension(WebKitWebContext *context)
{
    ORB_LOG_NO_ARGS();
    webkit_web_context_set_web_extensions_directory(context, ORB_WPE_WEB_EXTENSION_HOME);
}

/**
 * Create and properly set up the WebKit user content manager for injecting the ORB JavaScript.
 *
 * @return Pointer to the WebKit user content manager
 */
WebKitUserContentManager * ORBWPEWebExtensionHelper::CreateWebKitUserContentManager()
{
    ORB_LOG_NO_ARGS();
    WebKitUserContentManager *userContentManager = webkit_user_content_manager_new();
    webkit_user_content_manager_remove_all_scripts(userContentManager);
    gchar *source = nullptr;
    WebKitUserScript *script = nullptr;
    if (g_file_get_contents(ORB_HBBTV_JS_PATH, &source, NULL, NULL))
    {
        script = webkit_user_script_new(
            source,
            WEBKIT_USER_CONTENT_INJECT_TOP_FRAME,
            WEBKIT_USER_SCRIPT_INJECT_AT_DOCUMENT_START,
            NULL,
            NULL
            );
        webkit_user_content_manager_add_script(userContentManager, script);
        webkit_user_script_unref(script);
        g_free(source);
    }
    return userContentManager;
}

/**
 * Register the DVB URL scheme handler.
 *
 * @param context Pointer to the WebKit web context
 */
void ORBWPEWebExtensionHelper::RegisterDVBURLSchemeHandler(WebKitWebContext *context)
{
    ORB_LOG_NO_ARGS();

    WebKitSecurityManager *securityManager = webkit_web_context_get_security_manager(context);
    webkit_security_manager_register_uri_scheme_as_cors_enabled(securityManager, "hbbtv-carousel");
    webkit_security_manager_register_uri_scheme_as_cors_enabled(securityManager, "dvb");

    webkit_web_context_register_uri_scheme(context, "hbbtv-carousel",
        (WebKitURISchemeRequestCallback) HandleDVBURISchemeRequest, nullptr, nullptr);

    webkit_web_context_register_uri_scheme(context, "dvb",
        (WebKitURISchemeRequestCallback) HandleDVBURISchemeRequest, nullptr, nullptr);
}

/**
 * Set custom preferences for the ORB browser.
 *
 * @param preferences        Pointer to the browser's global settings
 * @param jsonConfigAsString String containing the JSON representation of the browser config
 */
void ORBWPEWebExtensionHelper::SetORBWPEWebExtensionPreferences(WebKitSettings *preferences,
    std::string jsonConfigAsString)
{
    JsonObject jsonConfig;
    jsonConfig.FromString(jsonConfigAsString);
    if (jsonConfig.HasLabel("logtosystemconsoleenabled"))
    {
        webkit_settings_set_enable_write_console_messages_to_stdout(preferences,
            jsonConfig["logtosystemconsoleenabled"].Boolean());
    }

    std::string userAgentString = GetORBClient()->GetUserAgentString();
    webkit_settings_set_user_agent(preferences, userAgentString.c_str());

    webkit_settings_set_enable_plugins(preferences, false);
    webkit_settings_set_allow_display_of_insecure_content(preferences, true);
}

/**
 * Get the mimetype based on the given url
 *
 * @param url The url to parse
 * @return The mimetype or * / * for unkown
 */
std::string ORBWPEWebExtensionHelper::GetMimeTypeFromUrl(std::string url)
{
    std::string type = "*/*";

    std::string extension = GetFileExtensionFromUrl(url);
    if (extension != "") 
    {
        std::map<std::string, std::string>::iterator it = m_mimetypeMap.find(extension);
        if (it != m_mimetypeMap.end())
        {
            type = m_mimetypeMap[extension];
        } 
        else if (extension == "html5" || extension == "cehtml")
        {
            type = "text/html";
        }
    }
    return type;
}

} // namespace orb
