/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
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
         * @param app_id The application ID.
         * @param entry_url The entry page URL.
         */
        virtual void LoadApplication(uint16_t app_id, const char *entry_url) = 0;

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
     * @param calling_app_id The calling app ID or INVALID_APP_ID if not called by an app.
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
    bool CreateApplication(uint16_t calling_app_id, const std::string &url);

    /**
     * Destroy the calling application.
     *
     * @param calling_app_id The calling app ID.
     */
    void DestroyApplication(uint16_t calling_app_id);

    /**
     * Show the calling application.
     *
     * @param calling_app_id The calling app ID.
     */
    void ShowApplication(uint16_t calling_app_id);

    /**
     * Hide the calling application.
     *
     * @param calling_app_id The calling app ID.
     */
    void HideApplication(uint16_t calling_app_id);

    /**
     * Set the key set mask for an application.
     *
     * @param app_id The application.
     * @param key_set_mask The key set mask.
     */
    void SetKeySetMask(uint16_t app_id, uint16_t key_set_mask);

    /**
     * Get the key set mask for an application.
     *
     * @param app_id The application.
     * @return The key set mask for the application.
     */
    uint16_t GetKeySetMask(uint16_t app_id);

    /**
     * Process an AIT section. The table will be processed when it is completed or updated.
     *
     * @param ait_pid The PID of this section.
     * @param service_id The service this section was received for.
     * @param section_data The section section_data.
     * @param section_data_bytes The size of section_data in bytes.
     */
    void ProcessAitSection(uint16_t ait_pid, uint16_t service_id, uint8_t *section_data, uint32_t
        section_data_bytes);

    /**
     * Process an XML AIT and create and run a new broadcast-independent application.
     *
     * @param xml_ait The XML AIT contents.
     * @return true if the application can be created, otherwise false
     */
    bool ProcessXmlAit(const std::string &xml_ait);

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
     * @param calling_app_id The app ID making the request.
     * @param calling_page_url The page URL making the request.
     * @param method_requirement Any additional requirement of the method.
     * @return true if the request is allowed, otherwise false
     */
    bool IsRequestAllowed(uint16_t calling_app_id, const std::string &calling_page_url,
        MethodRequirement method_requirement);

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
    void OnChannelChanged(uint16_t original_network_id, uint16_t transport_stream_id, uint16_t
        service_id);

    /**
     * Called when the network availability has changed.
     *
     * @param available true if the network is available, otherwise false
     */
    void OnNetworkAvailabilityChanged(bool available);

    /**
     * Notify the application manager that a call to loadApplication failed.
     *
     * @param app_id The application ID of the application that failed to load.
     */
    void OnLoadApplicationFailed(uint16_t app_id);

    /**
     * Notify the application manager of application page changed, before the new page is
     * loaded. For example, when the user follows a link.
     *
     * @param app_id The application ID.
     * @param url The URL of the new page.
     */
    void OnApplicationPageChanged(uint16_t app_id, const std::string &url);

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
     * @param ait_table AIT table.
     * @return The App to auto start.
     */
    const Ait::S_AIT_APP_DESC* GetAutoStartApp(const Ait::S_AIT_TABLE *ait_table);

    std::unique_ptr<SessionCallback> session_callback_;
    uint16_t next_app_id_;
    Ait ait_;
    std::unique_ptr<Ait::S_AIT_TABLE> xml_ait_;
    App app_;
    Utils::S_DVB_TRIPLET current_service_ = Utils::MakeInvalidDvbTriplet();
    uint16_t current_service_received_first_ait_ = false;
    uint16_t current_service_ait_pid_ = 0;
    bool is_network_available_ = false;
    std::recursive_mutex lock_;
    Utils::Timeout ait_timeout_;
};

#endif // HBBTV_SERVICE_MANAGER_H
