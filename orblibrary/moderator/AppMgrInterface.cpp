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
const string MANAGER_GET_APP_SCHEME = "getApplicationScheme";
const string MANAGER_GET_FREE_MEM = "getFreeMem";
const string MANAGER_GET_KEY_VALUES = "getKeyValues";
const string MANAGER_GET_OKEY_VALUES = "getOtherKeyValues";
const string MANAGER_GET_KEY_MAX_VAL = "getKeyMaximumValue";
const string MANAGER_GET_MAX_OKEYS = "getKeyMaximumOtherKeys";
const string MANAGER_SET_KEY_VALUE = "setKeyValue";
const string MANAGER_GET_KEY_ICON = "getKeyIcon";

const int KEY_OTHERS_MAX = 0x416; // temporary value based on v1.0

AppMgrInterface::AppMgrInterface(IOrbBrowser* browser, ApplicationType apptype)
    : mOrbBrowser(browser)
    , mAppType(apptype)
{
    // Set this AppMgrInterface instance as the callback for ApplicationManager
    ApplicationManager::instance().RegisterCallback(apptype, this);
}

string AppMgrInterface::executeRequest(string method, Json::Value token, Json::Value params)
{
    auto buildJsonResponse = [](const Json::Value &value) -> std::string {
      Json::Value responseObj;
      responseObj["result"] = value;
      return JsonUtil::convertJsonToString(responseObj);
    };

    std::lock_guard<std::mutex> lock(mMutex);
    string response = buildJsonResponse(""); // default response

    auto &appMgr = ApplicationManager::instance();
    int appId = JsonUtil::getIntegerValue(params, "id");

    appMgr.SetCurrentInterface(mAppType);

    LOGI("Request with method [" + method + "] received");
    if (method == MANAGER_CREATE_APP)
    {
        int newAppId = appMgr.CreateApplication(
          appId, JsonUtil::getStringValue(params, "url"), JsonUtil::getBoolValue(params, "runAsOpApp"));

        if (newAppId == INVALID_APP_ID)
        {
          LOGE("Failed to create application with ID " << appId);
          response = buildJsonResponse("Failed to create application with ID " + std::to_string(appId));
        }
        else
        {
          LOGI("app type: " << mAppType << " new AppID" << newAppId);
          response = buildJsonResponse(newAppId);
        }
    }
    else if (method == MANAGER_DESTROY_APP)
    {
        appMgr.DestroyApplication(appId);
        // no response needed
    }
    else if (method == MANAGER_SHOW_APP)
    {
        appMgr.ShowApplication(appId);
        // no response needed
    }
    else if (method == MANAGER_HIDE_APP)
    {
        appMgr.HideApplication(appId);
        // no response needed
    }
    else if (method == MANAGER_GET_APP_IDS)
    {
        // Get running app IDs from ApplicationManager
        std::vector<int> runningAppIds = appMgr.GetRunningAppIds();

        // Create JSON response with array of integers
        Json::Value resultArray = Json::Value(Json::arrayValue);
        for (int id : runningAppIds) {
            resultArray.append(Json::Value(id));
        }

        LOGI("getRunningAppIds: returned " << runningAppIds.size() << " app IDs");
        response = buildJsonResponse(resultArray);
    }
    else if (method == MANAGER_GET_APP_URL)
    {
        response = buildJsonResponse(appMgr.GetApplicationUrl(appId));
    }
    else if (method == MANAGER_GET_APP_SCHEME)
    {
        response = buildJsonResponse(appMgr.GetApplicationScheme(appId));
    }
    else if (method == MANAGER_SET_KEY_VALUE)
    {
        uint16_t keyset = JsonUtil::getIntegerValue(params, "value");
        std::vector<uint16_t> otherkeys = JsonUtil::getIntegerArray(params, "otherKeys");
        uint16_t kMask = appMgr.SetKeySetMask(appId, keyset, otherkeys);
        if (kMask > 0) {
            mOrbBrowser->notifyKeySetChange(keyset, otherkeys);
        }
    }
    else if (method == MANAGER_GET_KEY_VALUES)
    {
        uint16_t keyset = appMgr.GetKeySetMask(appId);

        response = buildJsonResponse(keyset);
    }
    else if (method == MANAGER_GET_OKEY_VALUES)
    {
        std::vector<uint16_t> otherkeys = appMgr.GetOtherKeyValues(appId);

        // Create JSON response with array of other key values
        Json::Value resultArray = Json::Value(Json::arrayValue);
        for (uint16_t keyValue : otherkeys) {
            resultArray.append(Json::Value(keyValue));
        }

        response = buildJsonResponse(resultArray);

        LOGI("return: " << otherkeys.size() << " other key values");
    }
    else if (method == MANAGER_GET_KEY_MAX_VAL)
    {
        int maxval = KEY_SET_RED | KEY_SET_GREEN | KEY_SET_YELLOW | KEY_SET_BLUE |
                KEY_SET_NAVIGATION | KEY_SET_VCR | KEY_SET_NUMERIC;
        response = buildJsonResponse(maxval);
    }
    else if (method == MANAGER_GET_MAX_OKEYS)
    {
        response = buildJsonResponse(KEY_OTHERS_MAX);
    }
    else if (method == MANAGER_GET_KEY_ICON)
    {
        response = buildJsonResponse("AppMgrInterface; method [" + method + "] unsupported");
    }
    else if (method == MANAGER_GET_FREE_MEM)
    {
        response = buildJsonResponse("AppMgrInterface; method [" + method + "] unsupported");
    }
    else
    {
        LOGI("Unknown method: " << method);
        response = buildJsonResponse("AppMgrInterface; method [" + method + "] unknown");
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
