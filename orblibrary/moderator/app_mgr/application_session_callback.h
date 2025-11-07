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

#ifndef APPLICATION_SESSION_CALLBACK_H
#define APPLICATION_SESSION_CALLBACK_H

#include "utils.h"

namespace orb
{

class ApplicationSessionCallback
{
public:
    using onPageLoadedSuccess = std::function<void()>;

    /**
     * Tell the browser to load an application. If the entry page fails to load, the browser
     * should call ApplicationManager::OnLoadApplicationFailed.
     *
     * @param appId The application ID.
     * @param entryUrl The entry page URL.
     * @param callback The callback to call when the application is loaded and ready to use.
     */
    virtual void LoadApplication(
        const int appId,
        const char *entryUrl,
        onPageLoadedSuccess callback = nullptr) = 0;

    /**
     * Tell the browser to load an application. If the entry page fails to load, the browser
     * should call ApplicationManager::OnLoadApplicationFailed.
     *
     * @param appId The application ID.
     * @param entryUrl The entry page URL.
     * @param size The number of the co-ordinate graphics
     * @param graphics The list of the co-ordinate graphics supported by the application
     * @param callback The callback to call when the application is loaded and ready to use.
     */
    virtual void LoadApplication(
        const int appId,
        const char *entryUrl,
        int size,
        const std::vector<uint16_t> graphics,
        onPageLoadedSuccess callback = nullptr) = 0;

    /**
     * Tell the browser to show the loaded application.
     */
    virtual void ShowApplication(const int appId) = 0;

    /**
     * Tell the browser to hide the loaded application.
     */
    virtual void HideApplication(const int appId) = 0;

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
     *  Tell the bridge to dispatch ApplicationLoaded to the loaded application.
     */
    virtual void DispatchApplicationLoadedEvent(const int appId) = 0;

    /**
     *  Tell the bridge to dispatch ApplicationUnloaded to the loaded application.
     */
    virtual void DispatchApplicationUnloadedEvent(const int appId) = 0;

    /**
     *  Tell the bridge to dispatch TransitionedToBroadcastRelated to the loaded application.
     */
    virtual void DispatchTransitionedToBroadcastRelatedEvent(const int appId) = 0;

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

    virtual void DispatchApplicationSchemeUpdatedEvent(const int appId, const std::string &scheme) = 0;

    virtual void DispatchOperatorApplicationStateChange(const int appId, const std::string &oldState, const std::string &newState) = 0;
    virtual void DispatchOperatorApplicationStateChangeCompleted(const int appId, const std::string &oldState, const std::string &newState) = 0;
    virtual void DispatchOperatorApplicationContextChange(const int appId, const std::string &startupLocation, const std::string &launchLocation = "") = 0;
    virtual void DispatchOpAppUpdate(const int appId, const std::string &updateEvent) = 0;

    /**
     * Returns true if the provided triplet is in an instance within the
     * currently playing service, otherwise false.
     */
    virtual bool isInstanceInCurrentService(const Utils::S_DVB_TRIPLET &triplet) = 0;

    /**
     *
     */
    virtual ~ApplicationSessionCallback() = default;
};

} // namespace orb

#endif // APPLICATION_SESSION_CALLBACK_H
