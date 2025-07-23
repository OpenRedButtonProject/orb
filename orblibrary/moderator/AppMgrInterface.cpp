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
#include <json/json.h>

#include "AppMgrInterface.hpp"
#include "app_mgr/application_manager.h"
#include "JsonUtil.h"
#include "log.hpp"

#define LINKED_APP_SCHEME_1_1 "urn:dvb:metadata:cs:LinkedApplicationCS:2019:1.1"

using namespace std;

namespace orb
{

const string MANAGER_CREATE_APP = "createApplication";
const string MANAGER_DESTROY_APP = "destroyApplication";
const string MANAGER_SHOW_APP = "showApplication";
const string MANAGER_HIDE_APP = "hideApplication";
const string MANAGER_GET_APP_IDS = "getRunningAppIds";
const string MANAGER_GET_APP_URL = "getApplicationUrl";

AppMgrInterface::AppMgrInterface(IOrbBrowser* browser, ApplicationType apptype)
    : mOrbBrowser(browser)
    , mAppType(apptype)
{
    // Set this AppMgrInterface instance as the callback for ApplicationManager
    ApplicationManager::instance().RegisterCallback(apptype, this);
}

string AppMgrInterface::executeRequest(string method, Json::Value token, Json::Value params)
{
    Json::CharReaderBuilder builder;
    string response;

    std::lock_guard<std::mutex> lock(mMutex);
    ApplicationManager::instance().SetCurrentInterface(mAppType);

    LOGI("Request with method [" + method + "] received");
    if (method == MANAGER_CREATE_APP)
    {
        int appId =
            ApplicationManager::instance().CreateApplication(
                JsonUtil::getIntegerValue(params, "id"),
                JsonUtil::getStringValue(params, "url"),
                JsonUtil::getBoolValue(params, "runAsOpApp"));

        LOGI("app type: " << mAppType << " new AppID" << appId);

        Json::Value responseObj;
        responseObj["result"] = appId;
        response = JsonUtil::convertJsonToString(responseObj);
    }
    else if (method == MANAGER_DESTROY_APP)
    {
        ApplicationManager::instance().DestroyApplication(JsonUtil::getIntegerValue(params, "id"));
        // no response needed
    }
    else if (method == MANAGER_SHOW_APP)
    {
        ApplicationManager::instance().ShowApplication(JsonUtil::getIntegerValue(params, "id"));
        // no response needed
    }
    else if (method == MANAGER_HIDE_APP)
    {
        ApplicationManager::instance().HideApplication(JsonUtil::getIntegerValue(params, "id"));
        // no response needed
    }
    else if (method == MANAGER_GET_APP_URL)
    {
        std::string url =
            ApplicationManager::instance().GetApplicationUrl(JsonUtil::getIntegerValue(params, "id"));

        Json::Value responseObj;
        responseObj["result"] = url;
        response = JsonUtil::convertJsonToString(responseObj);
    }
    else if (method == MANAGER_GET_APP_IDS)
    {
        // Get running app IDs from ApplicationManager
        std::vector<int> runningAppIds = ApplicationManager::instance().GetRunningAppIds();

        // Create JSON response with array of integers
        Json::Value resultArray = Json::Value(Json::arrayValue);
        for (int appId : runningAppIds) {
            resultArray.append(Json::Value(appId));
        }

        Json::Value responseObj;
        responseObj["result"] = resultArray;
        response = JsonUtil::convertJsonToString(responseObj);

        LOGI("getRunningAppIds: returned " << runningAppIds.size() << " app IDs");
    }
    else
    {
        LOGI("Unknown method: " << method);
        response = R"({"Response": "AppMgrInterface; method [)" + method + R"(] unknown"})";
    }

    return response;
}

void AppMgrInterface::onNetworkStatusChange(bool available) {
    std::lock_guard<std::mutex> lock(mMutex);
    ApplicationManager::instance().SetCurrentInterface(mAppType);
    ApplicationManager::instance().OnNetworkAvailabilityChanged(available);
}

void AppMgrInterface::onChannelChange(uint16_t onetId, uint16_t transId, uint16_t serviceId) {
    std::lock_guard<std::mutex> lock(mMutex);
    ApplicationManager::instance().SetCurrentInterface(mAppType);
    ApplicationManager::instance().OnChannelChanged(onetId, transId, serviceId);
}

void AppMgrInterface::processAitSection(int32_t aitPid, int32_t serviceId, const std::vector<uint8_t>& section) {
    std::lock_guard<std::mutex> lock(mMutex);
    ApplicationManager::instance().SetCurrentInterface(mAppType);
    ApplicationManager::instance().ProcessAitSection(aitPid, serviceId, section.data(), section.size());
}

void AppMgrInterface::processXmlAit(const std::vector<uint8_t>& xmlait) {
    std::string xmlString(xmlait.begin(), xmlait.end());
    std::lock_guard<std::mutex> lock(mMutex);
    ApplicationManager::instance().SetCurrentInterface(mAppType);
    ApplicationManager::instance().ProcessXmlAit(xmlString);
}

// ApplicationSessionCallback implementation
void AppMgrInterface::LoadApplication(const int appId, const char *entryUrl) {
    LOGI("Apptyp: " << mAppType << ", appID: " << appId << ", url: " << entryUrl);
    mOrbBrowser->loadApplication(std::to_string(appId), entryUrl);
}

void AppMgrInterface::LoadApplication(const int appId, const char *entryUrl, int size, const std::vector<uint16_t> graphics) {
    LOGI("Apptyp: " << mAppType << ", appID: " << appId << ", url: " << entryUrl);
    // TODO: need a different API to add extra params
    mOrbBrowser->loadApplication(std::to_string(appId), entryUrl);
}

void AppMgrInterface::ShowApplication(const int appId) {
    LOGI("Apptyp: " << mAppType << ", appID: " << appId);
    mOrbBrowser->showApplication();
}

void AppMgrInterface::HideApplication(const int appId) {
    LOGI("Apptyp: " << mAppType << ", appID: " << appId);
    mOrbBrowser->hideApplication();
}

void AppMgrInterface::StopBroadcast() {
    LOGI(" TODO ");
}

void AppMgrInterface::ResetBroadcastPresentation() {
    LOGI(" TODO ");
}

void AppMgrInterface::DispatchApplicationLoadErrorEvent() {
    mOrbBrowser->dispatchEvent("ApplicationLoadError", "{}");
}

void AppMgrInterface::DispatchTransitionedToBroadcastRelatedEvent(const int appId) {
    LOGI("appID: " << appId);
}

std::string AppMgrInterface::GetXmlAitContents(const std::string &url) {
    std::string result;
    return result;
}

int AppMgrInterface::GetParentalControlAge() {
    LOGI("");
    return 0;
}

std::string AppMgrInterface::GetParentalControlRegion() {
    std::string result;
    LOGI("");
    return result;
}

std::string AppMgrInterface::GetParentalControlRegion3() {
    std::string result;
    LOGI("");
    return result;
}

void AppMgrInterface::DispatchApplicationSchemeUpdatedEvent(const int appId, const std::string &scheme) {
    LOGI("appID: " << appId << ", Scheme: " << scheme);
    Json::Value prop;
    prop["scheme"] = scheme;
    mOrbBrowser->dispatchEvent("ApplicationSchemeUpdated", JsonUtil::convertJsonToString(prop));
}

void AppMgrInterface::DispatchOperatorApplicationStateChange(const int appId, const std::string &oldState, const std::string &newState) {
    LOGI("appID: " << appId);
}

void AppMgrInterface::DispatchOperatorApplicationStateChangeCompleted(const int appId, const std::string &oldState, const std::string &newState) {
    LOGI("appID: " << appId);
}

void AppMgrInterface::DispatchOperatorApplicationContextChange(const int appId, const std::string &startupLocation, const std::string &launchLocation) {
    LOGI("appID: " << appId);
}

void AppMgrInterface::DispatchOpAppUpdate(const int appId, const std::string &updateEvent) {
    LOGI("appID: " << appId);
}

bool AppMgrInterface::isInstanceInCurrentService(const Utils::S_DVB_TRIPLET &triplet) {
    LOGI("");
    return false;
}

bool AppMgrInterface::IsRequestAllowed(string token)
{
    // TODO implement
    return true;
}

} // namespace orb
