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
 * @brief EncodeUrl
 *
 * Helper function that creates an hbbtv-carousel:// url if a dvb:// url was given as input
 *
 * @param url       The input url which can be http(s):// or dvb://
 * @return          The resulting url
 */
static std::string EncodeUrl(const char *url)
{
    // in case of dvb url, create an hbbtv-carousel scheme url
    // dvb://<triplet and component id>/path/to/resource/file.ext
    // hbbtv-carousel://orgid:carouselid/path/to/resource/file.ext?dvburl=dvb://<triplet and component id>
    ORB_LOG("%s", url);
    std::string urlStr(url);
    size_t found;
    if ((found = urlStr.rfind("dvb://", 0)) == 0)
    {
        std::string baseUrl = urlStr;
        std::string path = "";
        found = urlStr.find('/', found + 6);
        if (found != std::string::npos)
        {
            path = urlStr.substr(found);
            baseUrl = urlStr.substr(0, found);
        }
        
        uint32_t componentTag=0;
        sscanf(baseUrl.c_str(), "dvb://%*x.%*x.%*x.%x", &componentTag);
      
        uint32_t carouselId =
            ORBEngine::GetSharedInstance().GetORBPlatform()->Dsmcc_RequestCarouselId(componentTag);

        uint32_t orgId = 
            ORBEngine::GetSharedInstance().GetApplicationManager()->GetOrganizationId();

        
        // check if parameter or fragment already exists
        std::string dvburl = "?dvburl=" + baseUrl;
        if (path.find('?') != std::string::npos || path.find('#') != std::string::npos)
        {
            dvburl = "&dvburl=" + baseUrl;
        }
        
        std::string carouselUrl = "hbbtv-carousel://" +
            std::to_string(orgId) + ":" +
            std::to_string(carouselId) +
            path +
            dvburl;

        ORB_LOG("The carousel url is: %s", carouselUrl.c_str());
        urlStr = carouselUrl;
    }
    return urlStr;  
}

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

    std::string urlStr = EncodeUrl(url);

    ORBEngine::GetSharedInstance().GetORBPlatform()->Application_Load(urlStr.c_str());
}

/**
 * Tell the browser to load an application. If the entry page fails to load, the browser
 * should call ApplicationManager::OnLoadApplicationFailed.
 *
 * @param appId The application ID.
 * @param entryUrl The entry page URL.
 * @param size The number of the co-ordinate graphics
 * @param graphics The list of the co-ordinate graphics supported by the application
 */
void SessionCallbackImpl::LoadApplication(uint16_t appId, const char *entryUrl, int size, const std::vector<uint16_t> graphics)
{
    ORB_LOG("HbbTV Version: %d\n",ORB_HBBTV_VERSION);

    if (ORB_HBBTV_VERSION == 203)
    {
        LoadApplication(appId, entryUrl);
    } 
    else
    {
        // TODO: Support 204
        ORB_LOG("204 not supported\n");
    }
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

void SessionCallbackImpl::DispatchApplicationSchemeUpdatedEvent(const std::string &scheme) { }

/**
 * Returns true if the provided triplet is in an instance within the
 * currently playing service, otherwise false.
 */
bool isInstanceInCurrentService(const Utils::S_DVB_TRIPLET &triplet) { return false; }

} // namespace orb
