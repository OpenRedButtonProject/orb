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

#include "base_app.h"
#include "utils.h"

namespace orb
{
class OpApp : public BaseApp
{
public:
    OpApp(const std::string &url, ApplicationSessionCallback *sessionCallback);
    OpApp(ApplicationSessionCallback *sessionCallback);

    /* Create opapp from url and inherit another opapp's state (ETSI TS 103 606 V1.2.1 (2024-03) 6.3.3.1).
     * Unsupported in FREELY */
    // OpApp(const OpApp &other, const std::string &url);

    virtual ~OpApp() = default;

    OpApp(const OpApp&) = delete;
    OpApp &operator=(const OpApp&) = delete;

    bool SetState(const E_APP_STATE &state) override;

    int Load() override;

    static std::string opAppStateToString(const E_APP_STATE &state);

private:
    bool CanTransitionToState(const E_APP_STATE &state);

    void init();

    Utils::Timeout m_countdown;
};

} // namespace orb

#endif // OPAPP_H
