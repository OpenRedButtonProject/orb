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

// Full path of the ORB JavaScript to be injected
#define ORB_INJECTION_JS_PATH "/usr/share/WPEFramework/ORBBrowser/injection.js"

// Home directory of the ORB WPE web extension
#define ORB_WPE_WEB_EXTENSION_HOME "/usr/lib/orb"

// static properties for thread synchronisation
static std::mutex m;
static std::condition_variable cv;

// Map of currently active DVB URL loaders
static std::map<int, std::shared_ptr<ORBDVBURILoader> > s_dvbUriLoaders;

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

    {
        std::lock_guard<std::mutex> lk(m);
        s_dvbUriLoaders[requestId]->SetData(content, contentLength);
        s_dvbUriLoaders[requestId]->SetDataReady(true);
    }
    cv.notify_one();
}

/**
 * Callback for the inputKeyGenerated event coming from ORB.
 */
static void OnInputKeyGenerated(int keyCode)
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
    ORB_LOG("thread_id=%d", std::this_thread::get_id());

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
        OnInputKeyGenerated
        );

    m_orbClient->SubscribeToDvbUrlLoadedEvent();
}

/**
 * Destructor.
 */
ORBWPEWebExtensionHelper::~ORBWPEWebExtensionHelper()
{
    ORB_LOG_NO_ARGS();
    m_orbClient->UnsubscribeFromDvbUrlLoadedEvent();
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
    if (g_file_get_contents(ORB_INJECTION_JS_PATH, &source, NULL, NULL))
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
    webkit_web_context_register_uri_scheme(context, "dvb",
        (WebKitURISchemeRequestCallback) HandleDVBURISchemeRequest, nullptr, nullptr);
}
} // namespace orb
