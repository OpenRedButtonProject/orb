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

#include "SessionCallbackImpl.h"
#include "HttpDownloader.h"
#include "ORBEngine.h"
#include "ORBPlatformEventHandler.h"
#include "ORBPlatform.h"
#include "ORBLogging.h"

namespace orb {
/**
 * Constructor.
 */
SessionCallbackImpl::SessionCallbackImpl()
{
}

/**
 * Destructor.
 */
SessionCallbackImpl::~SessionCallbackImpl()
{
}

/**
 * @brief SessionCallbackImpl::LoadApplication
 *
 * Tell the browser to load an application. If the entry page fails to load, the browser
 * should call ApplicationManager::OnLoadApplicationFailed.
 *
 * @param app_id    The application ID
 * @param url       The entry page URL
 */
void SessionCallbackImpl::LoadApplication(uint16_t app_id, const char *url)
{
    ORB_LOG("app_id=%hu url=%s", app_id, url);
    ORBEngine::GetSharedInstance().SetCurrentAppId(app_id);
    ORBEngine::GetSharedInstance().SetCurrentAppUrl(url);
    ORBEngine::GetSharedInstance().GetORBPlatform()->Application_Load(url);
}

/**
 * @brief SessionCallbackImpl::ShowApplication
 *
 * Tell the browser to show the loaded application.
 */
void SessionCallbackImpl::ShowApplication()
{
    ORB_LOG_NO_ARGS();
    ORBEngine::GetSharedInstance().GetORBPlatform()->Application_SetVisible(true);
}

/**
 * @brief SessionCallbackImpl::HideApplication
 *
 * Tell the browser to hide the loaded application.
 */
void SessionCallbackImpl::HideApplication()
{
    ORB_LOG_NO_ARGS();
    ORBEngine::GetSharedInstance().GetORBPlatform()->Application_SetVisible(false);
}

/**
 * @brief SessionCallbackImpl::GetXmlAitContents
 *
 * Perform an HTTP GET request and return the contents, which should be an XML AIT resource.
 *
 * @param url The URL to get
 *
 * @return The contents of the resource at URL
 */
std::string SessionCallbackImpl::GetXmlAitContents(const std::string &url)
{
    ORB_LOG("url=%s", url.c_str());
    std::unique_ptr<HttpDownloader> httpDownloader = std::make_unique<HttpDownloader>();
    std::shared_ptr<HttpDownloader::DownloadedObject> downloadedObject = httpDownloader->Download(
        url);
    if (downloadedObject != nullptr)
    {
        if (downloadedObject->GetContentType().rfind("application/vnd.dvb.ait+xml", 0) == 0)
        {
            return downloadedObject->GetContent();
        }
    }
    return "";
}

/**
 * @brief SessionCallbackImpl::StopBroadcast
 *
 * Tell the broadcast-integration to stop presenting any broadcast component, equivalent to
 * selecting a null service.
 */
void SessionCallbackImpl::StopBroadcast()
{
    ORB_LOG_NO_ARGS();
    ORBEngine::GetSharedInstance().GetORBPlatform()->Broadcast_Stop();
}

/**
 * @brief SessionCallbackImpl::DispatchTransitionedToBroadcastRelatedEvent
 *
 * Tell the bridge to dispatch TransitionedToBroadcastRelated to the loaded application.
 */
void SessionCallbackImpl::DispatchTransitionedToBroadcastRelatedEvent()
{
    ORB_LOG_NO_ARGS();
    ORBEngine::GetSharedInstance().GetPlatformEventHandler()->OnAppTransitionedToBroadcastRelated();
}

/**
 * @brief SessionCallbackImpl::ResetBroadcastPresentation
 *
 * Tell the broadcast-integration to reset any calls by HbbTV to suspend presentation, set
 * the video rectangle or set the presented components.
 */
void SessionCallbackImpl::ResetBroadcastPresentation()
{
    ORB_LOG_NO_ARGS();
    ORBEngine::GetSharedInstance().GetORBPlatform()->Broadcast_Reset();
}

/**
 * @brief SessionCallbackImpl::DispatchApplicationLoadErrorEvent
 */
void SessionCallbackImpl::DispatchApplicationLoadErrorEvent()
{
    ORB_LOG_NO_ARGS();
    json properties;
    ORBEngine::GetSharedInstance().GetEventListener()->OnJavaScriptEventDispatchRequested(
        "ApplicationLoadError", properties.dump(), "", false);
}

/**
 * Get the currently set parental control age.
 *
 * @return The currently set parental control age
 */
int SessionCallbackImpl::GetParentalControlAge()
{
    return ORBEngine::GetSharedInstance().GetORBPlatform()->ParentalControl_GetAge();
}

/**
 * Get the 2-character country code of the current parental control.
 *
 * @return The 2-character country code
 */
std::string SessionCallbackImpl::GetParentalControlRegion()
{
    return ORBEngine::GetSharedInstance().GetORBPlatform()->ParentalControl_GetRegion();
}

/**
 * Get the 3-character country code of the current parental control.
 *
 * @return The 3-character country code
 */
std::string SessionCallbackImpl::GetParentalControlRegion3()
{
    return ORBEngine::GetSharedInstance().GetORBPlatform()->ParentalControl_GetRegion3();
}
} // namespace orb
