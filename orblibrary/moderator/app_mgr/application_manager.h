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
 * Application manager
 *
 * Note: This file is part of the platform-agnostic application manager library.
 */

#ifndef HBBTV_SERVICE_MANAGER_H
#define HBBTV_SERVICE_MANAGER_H

#include <climits>
#include <memory>
#include <vector>
#include <cstdint>
#include <alloca.h>
#include <mutex>
#include <unordered_map>

#include "utils.h"
#include "ait.h"
#include "hbbtv_app.h"
#include "opapp.h"
#include "application_session_callback.h"
#include "OrbConstants.h"
#include "xml_parser.h"

namespace orb
{

class ApplicationManager {
public:
    enum class MethodRequirement
    {
        FOR_RUNNING_APP_ONLY = 0,
        FOR_BROADCAST_APP_ONLY = 1,
        FOR_BROADCAST_OR_TRANSITIONING_APP_ONLY = 2,
        FOR_TRUSTED_APP_ONLY = 3
    };

    // Number of supported application types - BaseApp and OpApp
    static constexpr size_t MAX_CBS = 2;

    ApplicationManager(std::unique_ptr<IXmlParser> xmlParser = {});

    ~ApplicationManager();

    static ApplicationManager& instance();

    void SetXmlParser(std::unique_ptr<IXmlParser> xmlParser);

    /**
     * Register an interface callback for this ApplicationManager
     *
     * @param apptype App interface type
     * @param callback The callback to set.
     */
    void RegisterCallback(ApplicationType apptype, ApplicationSessionCallback* callback);

    /**
     * Set current interface callback
     *
     * @param apptype App interface type
     */
    void SetCurrentInterface(ApplicationType apptype);

    /**
     * Create and run a new application. If called by an application, check it is allowed.
     *
     * @param callingAppId The calling app ID or INVALID_APP_ID if not called by an app.
     * @param url A HTTP/HTTPS or DVB URL.
     * @param runAsOpApp Whether the newly created app will be launched as an OpApp.
     *
     * A HTTP/HTTPS URL may refer to the entry page or XML AIT of a broadcast-independent app.
     *
     * A DVB URL may refer to a broadcast-related app signalled in the current service AIT. This
     * will result in the signalled URL being loaded, which may be HTTP/HTTPS for broadband or DVB
     * for carousel.
     *
     * @return The id of the newly created application. In case of failure, INVALID_APP_ID is returned.
     */
    int CreateApplication(int callingAppId, const std::string &url, bool runAsOpApp);

    /**
     * Create and run an App by url.
     *
     * @param url The url the of the App.
     * @param runAsOpApp When true, the newly created app will be lauched as an OpApp,
     *      otherwise as an BaseApp.
     *
     * @return The id of the application. In case of failure, INVALID_APP_ID is returned.
     */
    int CreateAndRunApp(std::string url, bool runAsOpApp = false);

    /**
     * Create and run an App by AIT description.
     *
     * @param desc The AIT description the new App will use to set its initial state.
     * @param urlParams Additional url parameters that will be concatenated with the
     *      loaded url of the new App.
     * @param isBroadcast Is the new App broadcast related?
     * @param isTrusted Is the new App trusted?
     * @param runAsOpApp When true, the newly created app will be lauched as an OpApp,
     *      otherwise as an BaseApp.
     *
     * @return The id of the application. In case of failure, INVALID_APP_ID is returned.
     */
    int CreateAndRunApp(const Ait::S_AIT_APP_DESC &desc,
        const std::string &urlParams,
        bool isBroadcast,
        bool isTrusted,
        bool runAsOpApp = false);

    void DestroyApplication(int callingAppId);

    void ShowApplication(int callingAppId);

    void HideApplication(int callingAppId);

    /**
     * Set the key set mask for an application.
     *
     * @param appId The application.
     * @param keySetMask The key set mask.
     * @param otherKeys optional other keys
     * @return The key set mask for the application.
     */
    uint16_t SetKeySetMask(int appId, uint16_t keySetMask, std::vector<uint16_t> otherKeys);

    /**
     * Get the key set mask for an application.
     *
     * @param appId The application.
     * @return The key set mask for the application.
     */
    uint16_t GetKeySetMask(int appId);

    /**
     * Get the other keys for an application.
     *
     * @param appId The application.
     * @return The other keys for the application.
     */
    std::vector<uint16_t> GetOtherKeyValues(int appId);

    /**
     * Check the key code is accepted by the current key mask. Activate the app as a result if the
     * key is accepted.
     *
     * @param appId The application.
     * @param keyCode The key code to check.
     * @return The supplied key_code is accepted by the current app's key set.
     */
    bool InKeySet(int appId, uint16_t keyCode);

    /**
     * Process an AIT section. The table will be processed when it is completed or updated.
     *
     * @param aitPid The PID of this section.
     * @param serviceId The service this section was received for.
     * @param sectionData The section section_data.
     * @param sectionDataBytes The size of section_data in bytes.
     */
    void ProcessAitSection(uint16_t aitPid, uint16_t serviceId, const uint8_t *sectionData, uint32_t
        sectionDataBytes);

    /**
     * Process an XML AIT and create and run a new broadcast-independent application.
     *
     * @param xmlAit The XML AIT contents.
     * @param isDvbi true when the caller a DVB-I application.
     * @param scheme The linked application scheme.
     *
     * @return The id of the newly created application. In case of failure, INVALID_APP_ID is returned.
     */
    int ProcessXmlAit(const std::string &xmlAit, const bool isDvbi = false,
        const std::string &scheme = LINKED_APP_SCHEME_1_1);

    /**
     * Check whether a Teletext application is signalled.
     *
     * @return true if a Teletext application is signalled, otherwise false
     */
    bool IsTeletextApplicationSignalled(void);

    /**
     * Run the signalled Teletext application.
     *
     * @return true if the Teletext application can be created, otherwise false
     */
    bool RunTeletextApplication(void);

    /**
     * Check whether a request from the polyfill is allowed.
     *
     * @param callingAppId The app ID making the request.
     * @param callingPageUrl The page URL making the request.
     * @param methodRequirement Any additional requirement of the method.
     * @return true if the request is allowed, otherwise false
     */
    bool IsRequestAllowed(int callingAppId, const std::string &callingPageUrl,
        MethodRequirement methodRequirement);

    /**
     * Provide access to the AIT organization id
     *
     * @return uint32_t the organization id
     */
    uint32_t GetOrganizationId();

    /**
     * Get the names of the current app.
     *
     * @return The current app names as a map of <lang,name> pairs
     */
    std::map<std::string, std::string> GetCurrentAppNames();

    /**
     * Called when broadcast is stopped (for example when v/b object setChannel is called with null).
     *
     * If a broadcast-related application is running, it will transition to broadcast-independent or
     * be killed depending on the signalling.
     */
    void OnBroadcastStopped();

    /**
     * Called when the selected broadcast channel is changed (e.g. by the user or by v/b object).
     *
     * Once the first complete AIT is received or times out:
     *
     * If a broadcast-related application is running, it will continue to run or be killed depending
     * on the signalling.
     *
     * If a broadcast-independent application is running, it will transition to broadcast-related or
     * be killed depending on the signalling.
     */
    void OnChannelChanged(uint16_t originalNetworkId, uint16_t transportStreamId, uint16_t
        serviceId);

    /**
     * Called when the network availability has changed.
     *
     * @param available true if the network is available, otherwise false
     */
    void OnNetworkAvailabilityChanged(bool available);

    /**
     * Notify the application manager that a call to loadApplication failed.
     *
     * @param appId The application ID of the application that failed to load.
     */
    void OnLoadApplicationFailed(int appId);

    /**
     * Notify the application manager of application page changed, before the new page is
     * loaded. For example, when the user follows a link.
     *
     * @param appId The application ID.
     * @param url The URL of the new page.
     */
    void OnApplicationPageChanged(int appId, const std::string &url);

    std::string GetApplicationScheme(int appId);

    std::string GetApplicationUrl(int appId);

    std::vector<int> GetRunningAppIds();

private:
    void onSelectedServiceAitReceived();
    void onSelectedServiceAitTimeout();
    void onSelectedServiceAitUpdated();
    void onRunningAppExited();
    void onPerformBroadcastAutostart();

    int runOpApp(std::unique_ptr<OpApp> app);
    int runHbbTVApp(std::unique_ptr<HbbTVApp> app);
    bool updateRunningApp(const Ait::S_AIT_APP_DESC &desc);
    void killRunningApp(int appid);

    // Helper methods for accessing apps with the new pointer structure
    BaseApp* getAppById(int appId);
    bool isHbbTVAppRunning() const;
    bool isOpAppRunning() const;
    int getCurrentHbbTVAppId() const;
    int getCurrentOpAppId() const;

    /**
     * Transition the running app to broadcast-related, if conditions permit.
     *
     * @return true on success, false on failure.
     */
    bool transitionRunningAppToBroadcastRelated();

    /**
     * Transition the running app to broadcast-independent, if conditions permit.
     *
     * @return true on success, false on failure.
     */
    bool transitionRunningAppToBroadcastIndependent();

    /**
     * Whether the app should be trusted or not TODO
     *
     * @param is_broadcast Whether the app is broadcast-related
     * @return True if the app is trusted, false otherwise
     */
    bool isAppTrusted(bool is_broadcast);

    /**
     * Call to Ait::AutoStartApp() passing the parental restrictions.
     *
     * @param aitTable AIT table.
     * @return The App to auto start.
     */
    const Ait::S_AIT_APP_DESC* getAutoStartApp(const Ait::S_AIT_TABLE *aitTable);

    /**
     * Return the KeySet a key code belongs to.
     *
     * @param keyCode The key code.
     * @return The key set.
     */
    uint16_t getKeySet(const uint16_t keyCode);

    std::array<ApplicationSessionCallback*, MAX_CBS> m_sessionCallback;
    int m_cif; // current app type interface

    Ait m_ait;
    std::unique_ptr<IXmlParser> m_xmlParser;
    std::unique_ptr<HbbTVApp> m_hbbtvApp;
    std::unique_ptr<OpApp> m_opApp;
    Utils::S_DVB_TRIPLET m_currentService = Utils::MakeInvalidDvbTriplet();
    Utils::S_DVB_TRIPLET m_previousService = Utils::MakeInvalidDvbTriplet();
    bool m_currentServiceReceivedFirstAit = false;
    uint16_t m_currentServiceAitPid = 0;
    bool m_isNetworkAvailable = false;
    std::recursive_mutex m_lock;
    Utils::Timeout m_aitTimeout;
};

} // namespace orb

#endif // HBBTV_SERVICE_MANAGER_H
