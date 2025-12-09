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

#ifndef BASE_APP_H
#define BASE_APP_H

#include <string>
#include "OrbConstants.h"
#include <ostream>

namespace orb
{
class ApplicationSessionCallback;

std::ostream& operator<<(std::ostream& os, const ApplicationType& type);


class BaseApp
{
public:
    typedef enum
    {
        BACKGROUND_STATE = 0,
        FOREGROUND_STATE,
        TRANSIENT_STATE, /* OpApp only */
        OVERLAID_TRANSIENT_STATE, /* OpApp only */
        OVERLAID_FOREGROUND_STATE, /* OpApp only */
        INVALID_APP_STATE
    } E_APP_STATE;

    BaseApp(const ApplicationType type, const std::string &url, ApplicationSessionCallback *sessionCallback);
    BaseApp(ApplicationType type, ApplicationSessionCallback *sessionCallback);

    static const int INVALID_APP_ID;

    virtual ~BaseApp() = default;

    BaseApp(const BaseApp&) = delete;
    BaseApp& operator=(const BaseApp&) = delete;

    ApplicationType GetType() const;

    int GetId() const;

    E_APP_STATE GetState() const;

    /* Load the application and return its ID */
    virtual int Load() = 0;

    /**
     * Set the application state.
     *
     * @param state The desired state to transition to.
     * @returns true if transitioned successfully to the desired state, false otherwise.
     */
    virtual bool SetState(const E_APP_STATE &state) = 0;

    virtual std::string GetScheme() const;

    std::string GetLoadedUrl() const;
    void SetLoadedUrl(const std::string& url);

    /**
     * Get the key set mask for an application.
     *
     * @return The key set mask for the application.
     */
    uint16_t GetKeySetMask() const { return m_keySetMask; }

    /**
     * Set the key set mask for an application.
     *
     * @param keySetMask The key set mask.
     * @param otherKeys optional other keys
     * @return The key set mask for the application.
     */
    virtual uint16_t SetKeySetMask(const uint16_t keySetMask, const std::vector<uint16_t> &otherKeys);

    /**
     * Check the key code is accepted by the current key mask. Activate the app as a result if the
     * key is accepted.
     *
     * @param appId The application.
     * @param keyCode The key code to check.
     * @return The supplied key_code is accepted by the current app's key set.
     */
    virtual bool InKeySet(const uint16_t keyCode);

    /**
     * Get the other keys for an application.
     *
     * @param appId The application.
     * @return The other keys for the application.
     */
    std::vector<uint16_t> GetOtherKeyValues() const { return m_otherKeys; }

    /**
     * Return the KeySet a key code belongs to.
     *
     * @param keyCode The key code.
     * @return The key set.
     */
    static uint16_t GetKeySetMaskForKeyCode(const uint16_t keyCode);

protected:

    static bool IsKeyNavigation(const uint16_t code);
    static bool IsKeyNumeric(const uint16_t code);
    static bool IsKeyAlpha(const uint16_t code);
    static bool IsKeyVcr(const uint16_t code);
    static bool IsKeyScroll(const uint16_t code);

    ApplicationSessionCallback *m_sessionCallback;
    E_APP_STATE m_state;
    std::string m_scheme;

    uint16_t m_keySetMask = 0;
    std::vector<uint16_t> m_otherKeys;

private:
    const ApplicationType m_type;
    static int g_id;
    int m_id;
    std::string m_loadedUrl;
};

} // namespace orb

#endif
