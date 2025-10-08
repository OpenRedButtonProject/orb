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


namespace orb
{
class ApplicationSessionCallback;

class BaseApp
{
public:
    typedef enum
    {
        INVALID_APP_TYPE = 0,
        HBBTV_APP_TYPE,
        OPAPP_TYPE
    } E_APP_TYPE;

    typedef enum
    {
        BACKGROUND_STATE = 0,
        FOREGROUND_STATE,
        TRANSIENT_STATE, /* OpApp only */
        OVERLAID_TRANSIENT_STATE, /* OpApp only */
        OVERLAID_FOREGROUND_STATE, /* OpApp only */
        INVALID_APP_STATE
    } E_APP_STATE;

    BaseApp(const E_APP_TYPE type, const std::string &url, ApplicationSessionCallback *sessionCallback);
    BaseApp(E_APP_TYPE type, ApplicationSessionCallback *sessionCallback);

    static const int INVALID_APP_ID;

    virtual ~BaseApp() = default;

    BaseApp(const BaseApp&) = delete;
    BaseApp& operator=(const BaseApp&) = delete;

    E_APP_TYPE GetType() const;

    int GetId() const;

    E_APP_STATE GetState() const;

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

protected:
    ApplicationSessionCallback *m_sessionCallback;
    E_APP_STATE m_state;
    std::string m_scheme;

private:
    const E_APP_TYPE m_type;
    static int g_id;
    int m_id;
    std::string m_loadedUrl;
};

} // namespace orb

#endif
