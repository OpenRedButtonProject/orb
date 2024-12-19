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

#include "utils.h"
#include "ait.h"
#include "app.h"

#define INVALID_APP_ID 0

class ApplicationManager {
public:
    enum class MethodRequirement
    {
        FOR_RUNNING_APP_ONLY = 0,
        FOR_BROADCAST_APP_ONLY = 1,
        FOR_BROADCAST_OR_TRANSITIONING_APP_ONLY = 2,
        FOR_TRUSTED_APP_ONLY = 3
    };

    class SessionCallback
    {
public:
        /**
         * Tell the browser to load an application. If the entry page fails to load, the browser
         * should call ApplicationManager::OnLoadApplicationFailed.
         *
         * @param appId The application ID.
         * @param entryUrl The entry page URL.
         */
        virtual void LoadApplication(uint16_t appId, const char *entryUrl) = 0;

        /**
         * Tell the browser to load an application. If the entry page fails to load, the browser
         * should call ApplicationManager::OnLoadApplicationFailed.
         *
         * @param appId The application ID.
         * @param entryUrl The entry page URL.
         * @param size The number of the co-ordinate graphics
         * @param graphics The list of the co-ordinate graphics supported by the application
         */
        virtual void LoadApplication(uint16_t appId, const char *entryUrl, int size, const
            std::vector<uint16_t> graphics) = 0;

        /**
         * Tell the browser to show the loaded application.
         */
        virtual void ShowApplication() = 0;

        /**
         * Tell the browser to hide the loaded application.
         */
        virtual void HideApplication() = 0;

        /**
         * Tell the broadcast-integration to stop presenting any broadcast component, equivalent to
         * selecting a null service.
         */
        virtual void StopBroadcast() = 0;

        /**
         * Tell the broadcast-integration to reset any calls by HbbTV to suspend presentation, set
         * the video rectangle or set the presented components.
         */
        virtual void ResetBroadcastPresentation() = 0;

        /**
         *  Tell the bridge to dispatch ApplicationLoadError to the loaded application.
         */
        virtual void DispatchApplicationLoadErrorEvent() = 0;

        /**
         *  Tell the bridge to dispatch TransitionedToBroadcastRelated to the loaded application.
         */
        virtual void DispatchTransitionedToBroadcastRelatedEvent() = 0;

        /**
         * Perform a HTTP GET request and return the contents, which should be an XML AIT resource.
         *
         * @param url The URL to get.
         * @return The contents of the resource at URL.
         */
        virtual std::string GetXmlAitContents(const std::string &url) = 0;

        virtual int GetParentalControlAge() = 0;

        virtual std::string GetParentalControlRegion() = 0;

        virtual std::string GetParentalControlRegion3() = 0;

        virtual void DispatchApplicationSchemeUpdatedEvent(const std::string &scheme) = 0;

        /**
         * Returns true if the provided triplet is in an instance within the
         * currently playing service, otherwise false.
         */
        virtual bool isInstanceInCurrentService(const Utils::S_DVB_TRIPLET &triplet) = 0;
        
        /**
         *
         */
        virtual ~SessionCallback() = default;
    };

    /**
     * Application manager
     *
     * @param sessionCallback Implementation of ApplicationManager::SessionCallback interface.
     */
    ApplicationManager(std::unique_ptr<SessionCallback> sessionCallback);

    /**
     *
     */
    ~ApplicationManager();

    /**
     * Create and run a new application. If called by an application, check it is allowed.
     *
     * @param callingAppId The calling app ID or INVALID_APP_ID if not called by an app.
     * @param url A HTTP/HTTPS or DVB URL.
     *
     * A HTTP/HTTPS URL may refer to the entry page or XML AIT of a broadcast-independent app.
     *
     * A DVB URL may refer to a broadcast-related app signalled in the current service AIT. This
     * will result in the signalled URL being loaded, which may be HTTP/HTTPS for broadband or DVB
     * for carousel.
     *
     * @return true if the application can be created, otherwise false
     */
    bool CreateApplication(uint16_t callingAppId, const std::string &url);

    /**
     * Destroy the calling application.
     *
     * @param callingAppId The calling app ID.
     */
    void DestroyApplication(uint16_t callingAppId);

    /**
     * Show the calling application.
     *
     * @param callingAppId The calling app ID.
     */
    void ShowApplication(uint16_t callingAppId);

    /**
     * Hide the calling application.
     *
     * @param callingAppId The calling app ID.
     */
    void HideApplication(uint16_t callingAppId);

    /**
     * Set the key set mask for an application.
     *
     * @param appId The application.
     * @param keySetMask The key set mask.
     * @param otherKeys optional other keys
     * @return The key set mask for the application.
     */
    uint16_t SetKeySetMask(uint16_t appId, uint16_t keySetMask, std::vector<uint16_t> otherKeys);

    /**
     * Get the key set mask for an application.
     *
     * @param appId The application.
     * @return The key set mask for the application.
     */
    uint16_t GetKeySetMask(uint16_t appId);

    /**
     * Get the other keys for an application.
     *
     * @param appId The application.
     * @return The other keys for the application.
     */
    std::vector<uint16_t> GetOtherKeyValues(uint16_t appId);

    /**
     * Check the key code is accepted by the current key mask. Activate the app as a result if the
     * key is accepted.
     *
     * @param appId The application.
     * @param keyCode The key code to check.
     * @return The supplied key_code is accepted by the current app's key set.
     */
    bool InKeySet(uint16_t appId, uint16_t keyCode);

    /**
     * Process an AIT section. The table will be processed when it is completed or updated.
     *
     * @param aitPid The PID of this section.
     * @param serviceId The service this section was received for.
     * @param sectionData The section section_data.
     * @param sectionDataBytes The size of section_data in bytes.
     */
    void ProcessAitSection(uint16_t aitPid, uint16_t serviceId, uint8_t *sectionData, uint32_t
        sectionDataBytes);

    /**
     * Process an XML AIT and create and run a new broadcast-independent application.
     *
     * @param xmlAit The XML AIT contents.
     * @return true if the application can be created, otherwise false
     */
    bool ProcessXmlAit(const std::string &xmlAit, const bool &isDvbi = false, const
        std::string &scheme = LINKED_APP_SCHEME_1_1);

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
    bool IsRequestAllowed(uint16_t callingAppId, const std::string &callingPageUrl,
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
    void OnLoadApplicationFailed(uint16_t appId);

    /**
     * Notify the application manager of application page changed, before the new page is
     * loaded. For example, when the user follows a link.
     *
     * @param appId The application ID.
     * @param url The URL of the new page.
     */
    void OnApplicationPageChanged(uint16_t appId, const std::string &url);

    std::string GetApplicationScheme(uint16_t appId);

private:
    /**
     * Called when the AIT for the selected service is received.
     */
    void OnSelectedServiceAitReceived();

    /**
     * Called when the AIT for the selected service is not received after some timeout.
     */
    void OnSelectedServiceAitTimeout();

    /**
     * Called when the AIT for the selected service is updated.
     */
    void OnSelectedServiceAitUpdated();

    /**
     * Called when the running app has exited.
     */
    void OnRunningAppExited();

    /**
     * Called at a time when the broadcast autostart app should be started.
     */
    void OnPerformBroadcastAutostart();

    /**
     * Run the app.
     *
     * @param app The app to run.
     * @return True on success, false on failure.
     */
    bool RunApp(const App &app);

    /**
     * Kill the running app.
     */
    void KillRunningApp();

    /**
     * Transition the running app to broadcast-related, if conditions permit.
     *
     * @return true on success, false on failure.
     */
    bool TransitionRunningAppToBroadcastRelated();

    /**
     * Transition the running app to broadcast-independent, if conditions permit.
     *
     * @return true on success, false on failure.
     */
    bool TransitionRunningAppToBroadcastIndependent();

    /**
     * Whether the app should be trusted or not TODO
     *
     * @param is_broadcast Whether the app is broadcast-related
     * @return True if the app is trusted, false otherwise
     */
    bool IsAppTrusted(bool is_broadcast);

    /**
     * Call to Ait::AutoStartApp() passing the parental restrictions.
     *
     * @param aitTable AIT table.
     * @return The App to auto start.
     */
    const Ait::S_AIT_APP_DESC* GetAutoStartApp(const Ait::S_AIT_TABLE *aitTable);

    /**
     * Return the KeySet a key code belongs to.
     *
     * @param keyCode The key code.
     * @return The key set.
     */
    uint16_t GetKeySet(const uint16_t keyCode);

    std::unique_ptr<SessionCallback> m_sessionCallback;
    uint16_t m_nextAppId;
    Ait m_ait;
    App m_app;
    Utils::S_DVB_TRIPLET m_currentService = Utils::MakeInvalidDvbTriplet();
    Utils::S_DVB_TRIPLET m_previousService = Utils::MakeInvalidDvbTriplet();
    uint16_t m_currentServiceReceivedFirstAit = false;
    uint16_t m_currentServiceAitPid = 0;
    bool m_isNetworkAvailable = false;
    std::recursive_mutex m_lock;
    Utils::Timeout m_aitTimeout;
};

#endif // HBBTV_SERVICE_MANAGER_H
