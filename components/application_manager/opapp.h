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
 * App model
 *
 * Note: This file is part of the platform-agnostic application manager library.
 */

#ifndef OPAPP_H
#define OPAPP_H

#include "hbbtv_app.h"

class OpApp : public App
{
public:
    class SessionCallback : public HbbTVApp::SessionCallback
    {
public:
        virtual void DispatchOperatorApplicationStateChange(uint16_t appId, const std::string &oldState, const std::string &newState) = 0;
        virtual void DispatchOperatorApplicationStateChangeCompleted(uint16_t appId, const std::string &oldState, const std::string &newState) = 0;
        virtual void DispatchOperatorApplicationContextChange(uint16_t appId, const std::string &startupLocation, const std::string &launchLocation = "") = 0;
        virtual void DispatchOpAppUpdate(uint16_t appId, const std::string &updateEvent) = 0;
        virtual ~SessionCallback() = default;
    };

    /**
     * Create opapp from url.
     * 
     * @throws std::runtime_error
     */
    OpApp(const std::string &url, std::shared_ptr<OpApp::SessionCallback> sessionCallback);

    /**
     * Create opapp from Ait description.
     * 
     * @throws std::runtime_error
     */
    OpApp(const Ait::S_AIT_APP_DESC &desc, bool isNetworkAvailable, std::shared_ptr<OpApp::SessionCallback> sessionCallback);

    /**
     * Create opapp from url and inherit another opapp's state (ETSI TS 103 606 V1.2.1 (2024-03) 6.3.3.1).
     * 
     * @throws std::runtime_error
     */
    OpApp(const OpApp &other, const std::string &url);
    
    virtual ~OpApp() = default;

    OpApp(const HbbTVApp&) = delete;
    OpApp &operator=(const OpApp&) = delete;
    
    /**
     * Set the application state.
     * 
     * @param state The desired state to transition to.
     * @returns true if transitioned successfully to the desired state, false otherwise.
     */
    bool SetState(const E_APP_STATE &state) override;
    
    E_APP_TYPE GetType() const override { return HbbTVApp::E_APP_TYPE::OPAPP_TYPE; }
    
    bool TransitionToBroadcastRelated() override;

private:
    bool CanTransitionToState(const E_APP_STATE &state);

    Utils::Timeout m_countdown = Utils::Timeout([&]() -> void { SetState(BACKGROUND_STATE); });
};

#endif // OPAPP_H