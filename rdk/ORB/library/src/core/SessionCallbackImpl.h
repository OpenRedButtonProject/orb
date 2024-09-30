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

#pragma once

#include "application_manager.h"

namespace orb {
/**
 * @brief orb::SessionCallback
 *
 * Implementation of the session callback being used by the application manager
 * to interact with the integration components.
 */
class SessionCallbackImpl : public ApplicationManager::SessionCallback {
public:

    /**
     * Constructor.
     */
    SessionCallbackImpl();

    /**
     * Destructor.
     */
    ~SessionCallbackImpl();

    /**
     * @brief SessionCallbackImpl::LoadApplication
     *
     * Tell the browser to load an application. If the entry page fails to load, the browser
     * should call ApplicationManager::OnLoadApplicationFailed.
     *
     * @param app_id    The application ID
     * @param url       The entry page URL
     */
    virtual void LoadApplication(uint16_t app_id, const char *url) override;

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
        std::vector<uint16_t> graphics) override;

    /**
     * @brief SessionCallbackImpl::ShowApplication
     *
     * Tell the browser to show the loaded application.
     */
    virtual void ShowApplication() override;

    /**
     * @brief SessionCallbackImpl::HideApplication
     *
     * Tell the browser to hide the loaded application.
     */
    virtual void HideApplication() override;

    /**
     * @brief SessionCallbackImpl::GetXmlAitContents
     *
     * Perform an HTTP GET request and return the contents, which should be an XML AIT resource.
     *
     * @param url The URL to get
     *
     * @return The contents of the resource at URL
     */
    virtual std::string GetXmlAitContents(const std::string &url) override;

    /**
     * @brief SessionCallbackImpl::StopBroadcast
     *
     * Tell the broadcast-integration to stop presenting any broadcast component, equivalent to
     * selecting a null service.
     */
    virtual void StopBroadcast() override;

    /**
     * @brief SessionCallbackImpl::DispatchTransitionedToBroadcastRelatedEvent
     *
     * Tell the bridge to dispatch TransitionedToBroadcastRelated to the loaded application.
     */
    virtual void DispatchTransitionedToBroadcastRelatedEvent() override;

    /**
     * @brief SessionCallbackImpl::ResetBroadcastPresentation
     *
     * Tell the broadcast-integration to reset any calls by HbbTV to suspend presentation, set
     * the video rectangle or set the presented components.
     */
    virtual void ResetBroadcastPresentation() override;

    /**
     *  Tell the bridge to dispatch ApplicationLoadError to the loaded application.
     */
    virtual void DispatchApplicationLoadErrorEvent() override;

    /**
     * Get the currently set parental control age.
     *
     * @return The currently set parental control age
     */
    virtual int GetParentalControlAge() override;

    /**
     * Get the 2-character country code of the current parental control.
     *
     * @return The 2-character country code
     */
    virtual std::string GetParentalControlRegion() override;

    /**
     * Get the 3-character country code of the current parental control.
     *
     * @return The 3-character country code
     */
    virtual std::string GetParentalControlRegion3() override;

    virtual void DispatchApplicationSchemeUpdatedEvent(const std::string &scheme) override;
    
    /**
     * Returns true if the provided triplet is in an instance within the
     * currently playing service, otherwise false.
     */
    virtual bool isInstanceInCurrentService(const Utils::S_DVB_TRIPLET &triplet) override;
}; // class SessionCallbackImpl
} // namespace orb
