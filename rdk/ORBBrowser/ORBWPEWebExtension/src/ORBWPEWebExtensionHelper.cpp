/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
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
#define ORB_IFRAME_JS_PATH "/usr/share/WPEFramework/ORBBrowser/iframe.js"
#define ORB_DASH_JS_PATH   "/usr/share/WPEFramework/ORBBrowser/dash.all.min.js"

// HTML player page to be returned for ORB URI scheme requests
#define ORB_PLAYER_PAGE_PATH "/usr/share/WPEFramework/ORBBrowser/playerpage.html"

// Home directory of the ORB WPE web extension
#define ORB_WPE_WEB_EXTENSION_HOME "/usr/lib/orb"

// static properties for thread synchronisation
static std::mutex m;
static std::condition_variable cv;

// Map of currently active DVB URL loaders
static std::map<int, std::shared_ptr<ORBDVBURILoader> > s_dvbUriLoaders;

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

/**
 * Handle the specified ORB URI scheme request.
 *
 * @param request  The ORB URI scheme request
 * @param userData The user data or nullptr
 */
static void HandleORBURISchemeRequest(WebKitURISchemeRequest *request, gpointer userData)
{
    std::string uri(webkit_uri_scheme_request_get_uri(request));
    ORB_LOG("uri=%s", uri.c_str());

    // Return immediately if request URI does not start with orb://player
    if (uri.rfind("orb://player", 0) != 0)
    {
        std::string errorDescription = "The given ORB URI does not start with 'orb://player'";
        ORB_LOG("%s", errorDescription.c_str());
        GError *error = g_error_new(g_quark_from_string(uri.c_str()), 0, errorDescription.c_str());
        webkit_uri_scheme_request_finish_error(request, error);
        g_error_free(error);
        return;
    }

    // Read playerpage.html
    std::string playerPageHtml = ReadFileContentsIntoString(ORB_PLAYER_PAGE_PATH);

    // Read iframe.js
    std::string iframeJs = "<script type=\"text/javascript\">\n//<![CDATA[\n";
    iframeJs.append(ReadFileContentsIntoString(ORB_IFRAME_JS_PATH));
    iframeJs.append("\n//]]>\n</script>");

    // Read dash.all.min.js
    std::string dashJs = "<script type=\"text/javascript\">\n//<![CDATA[\n";
    dashJs.append(ReadFileContentsIntoString(ORB_DASH_JS_PATH));
    dashJs.append("\n//]]>\n</script>");

    // Prepare document.token injection
    std::string token = ((ORBGenericClient *)userData)->CreateToken(uri);
    std::string tokenJs = "<script type=\"text/javascript\">\n//<![CDATA[\n";
    tokenJs.append("document.token = " + token + ";");
    tokenJs.append("\n//]]>\n</script>");
    userData = nullptr;

    std::string::size_type pos = playerPageHtml.find("</head>");
    std::string playerPageHtmlStart = playerPageHtml.substr(0, pos);
    std::string playerPageHtmlEnd = playerPageHtml.substr(pos, std::string::npos);

    // Construct response
    std::string response;
    response.append(playerPageHtmlStart);
    response.append("\n");
    response.append(tokenJs);
    response.append("\n");
    response.append(iframeJs);
    response.append("\n");
    response.append(dashJs);
    response.append("\n");
    response.append(playerPageHtmlEnd);

    // Finish request with response
    const gchar *mimeType = "text/html";
    GInputStream *stream = nullptr;
    char *data = (char *) malloc(sizeof(char) * response.length());
    memmove(data, response.c_str(), response.length());
    stream = g_memory_input_stream_new_from_data((const void *)data, response.length(), free);
    webkit_uri_scheme_request_finish(request, stream, response.length(), mimeType);
    g_object_unref(stream);
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
        OnInputKeyGenerated
        );

    m_orbClient->SubscribeToDvbUrlLoadedNoDataEvent();
}

/**
 * Destructor.
 */
ORBWPEWebExtensionHelper::~ORBWPEWebExtensionHelper()
{
    ORB_LOG_NO_ARGS();
    m_orbClient->UnsubscribeFromDvbUrlLoadedNoDataEvent();
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
    webkit_security_manager_register_uri_scheme_as_cors_enabled(securityManager, "dvb");

    webkit_web_context_register_uri_scheme(context, "dvb",
        (WebKitURISchemeRequestCallback) HandleDVBURISchemeRequest, nullptr, nullptr);
}

/**
 * Register the ORB URL scheme handler.
 *
 * @param context Pointer to the WebKit web context
 */
void ORBWPEWebExtensionHelper::RegisterORBURLSchemeHandler(WebKitWebContext *context)
{
    ORB_LOG_NO_ARGS();

    WebKitSecurityManager *securityManager = webkit_web_context_get_security_manager(context);
    webkit_security_manager_register_uri_scheme_as_cors_enabled(securityManager, "orb");

    webkit_web_context_register_uri_scheme(context, "orb",
        (WebKitURISchemeRequestCallback) HandleORBURISchemeRequest, (gpointer) m_orbClient.get(),
        nullptr);
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
}
} // namespace orb
