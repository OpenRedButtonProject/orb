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
 * ORB Application Manager
 *
 */

#include <sys/sysinfo.h>

#include "AppManager.hpp"
#include "app_mgr/application_manager.h"

#include "log.h"

#define LINKED_APP_SCHEME_1_1 "urn:dvb:metadata:cs:LinkedApplicationCS:2019:1.1"

using namespace std;

namespace orb
{

class AppSessionCallback : public ApplicationSessionCallback {
public:
    /**
     * Tell the browser to load an application. If the entry page fails to load, the browser
     * should call ApplicationManager::OnLoadApplicationFailed.
     *
     * @param appId The application ID.
     * @param entryUrl The entry page URL.
     */
    void LoadApplication(const int appId, const char *entryUrl) override {
        LOGI("appID: " << appId << ", url: " << entryUrl);
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
    void LoadApplication(const int appId, const char *entryUrl, int size, const
        std::vector<uint16_t> graphics) override {
        LOGI("appID: " << appId << ", url: " << entryUrl);
    }

    /**
     * Tell the browser to show the loaded application.
     */
    void ShowApplication(const int appId) override {
        LOGI("appID: " << appId);
    }

    /**
     * Tell the browser to hide the loaded application.
     */
    void HideApplication(const int appId) override {
        LOGI("appID: " << appId);
    }

    /**
     * Tell the broadcast-integration to stop presenting any broadcast component, equivalent to
     * selecting a null service.
     */
    void StopBroadcast() override {
        LOGI("");
    }

    /**
     * Tell the broadcast-integration to reset any calls by HbbTV to suspend presentation, set
     * the video rectangle or set the presented components.
     */
    void ResetBroadcastPresentation() override {
        LOGI("");
    }

    /**
     *  Tell the bridge to dispatch ApplicationLoadError to the loaded application.
     */
    void DispatchApplicationLoadErrorEvent() override {
        LOGI("");
    }

    /**
     *  Tell the bridge to dispatch TransitionedToBroadcastRelated to the loaded application.
     */
    void DispatchTransitionedToBroadcastRelatedEvent(const int appId) override {
        LOGI("appID: " << appId);
    }

    /**
     * Perform a HTTP GET request and return the contents, which should be an XML AIT resource.
     *
     * @param url The URL to get.
     * @return The contents of the resource at URL.
     */
    std::string GetXmlAitContents(const std::string &url) override {
        std::string result;
        return result;
    }

    int GetParentalControlAge() override {
        LOGI("");
        return 0;
    }

    std::string GetParentalControlRegion() override {
        std::string result;
        LOGI("");
        return result;
    }

    std::string GetParentalControlRegion3() override {
        std::string result;
        LOGI("");
        return result;
    }

    void DispatchApplicationSchemeUpdatedEvent(const int appId, const std::string &scheme) override {
        LOGI("appID: " << appId);
    }

    void DispatchOperatorApplicationStateChange(const int appId, const std::string &oldState, const std::string &newState) override {
        LOGI("appID: " << appId);
    }

    void DispatchOperatorApplicationStateChangeCompleted(const int appId, const std::string &oldState, const std::string &newState) override {
        LOGI("appID: " << appId);
    }

    void DispatchOperatorApplicationContextChange(const int appId, const std::string &startupLocation, const std::string &launchLocation = "") override {
        LOGI("appID: " << appId);
    }

    void DispatchOpAppUpdate(const int appId, const std::string &updateEvent) override {
        LOGI("appID: " << appId);
    }

    /**
     * Returns true if the provided triplet is in an instance within the
     * currently playing service, otherwise false.
     */
    bool isInstanceInCurrentService(const Utils::S_DVB_TRIPLET &triplet) override {
        LOGI("");
        return false;
    }
};


AppManager& AppManager::instance()
{
    static AppManager s_interface;
    return s_interface;
}

AppManager::AppManager()
    : mApplicationManager(
        std::make_unique<ApplicationManager>(std::make_unique<AppSessionCallback>())) {
}

string AppManager::executeRequest(string method, Json::Value token, Json::Value params, ApplicationType apptype)
{
    // TODO Set up proper responses
    string response = R"({"Response": "AppManager request [)" + method + R"(] not implemented"})";

    LOGI("Request with method [" + method + "] received");
    if (method == "createApplication")
    {
        LOGI("app type: ") << apptype;
    }
    else if (method == "destroyApplication")
    {
        // no response
        LOGI("");
    }
    else if (method == "showApplication")
    {
        // no response
        LOGI("");
    }
    else if (method == "hideApplication")
    {
        // no response
        LOGI("");
    }
    else if (method == "searchOwner")
    {
        LOGI("");
    }
    else if (method == "getFreeMem")
    {
        // TODO: ask DVB client for this
        LOGI("");
    }
    else if (method == "getKeyIcon")
    {
        LOGI("");
    }
    else if (method == "setKeyValue")
    {
        LOGI("");
    }
    else if (method == "getKeyMaximumValue")
    {
        LOGI("");
    }
    else if (method == "getKeyValues")
    {
        LOGI("");
    }
    else if (method == "getApplicationScheme")
    {
        LOGI("");
    }
    else if (method == "getApplicationUrl")
    {
        LOGI("");
    }
    else if (method == "getRunningAppIds")
    {
        // TODO: string array?
        LOGI("");
    }
    else // Unknown Method
    {
        response = R"({"error": "AppManager request [)" + method + R"(] invalid method"})";
        LOGE("Invalid Method [" + method +"]");
    }

    LOGI("Response: " << response);

    return response;
}


void AppManager::processAitSection(int32_t aitPid, int32_t serviceId, const std::vector<uint8_t>& section) {

    mApplicationManager->ProcessAitSection((uint16_t)aitPid, (uint16_t)serviceId, section.data(), section.size());
}

void AppManager::processXmlAit(const std::vector<uint8_t>& xmlait) {
    const std::string xmlstr(reinterpret_cast<const char*>(xmlait.data()), xmlait.size());
    mApplicationManager->ProcessXmlAit(xmlstr);
}

bool AppManager::IsRequestAllowed(string token)
{
    return false;
}

} // namespace orb
