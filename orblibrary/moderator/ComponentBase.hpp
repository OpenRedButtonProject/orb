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

#include <string>
#include "IJson.h"
#include <memory>

namespace orb
{

class ComponentBase
{
public:
    ComponentBase() {}
    virtual ~ComponentBase() = default;

    ComponentBase (const ComponentBase&) = delete;
    ComponentBase& operator= (const ComponentBase&) = delete;

    /**
     * ComponentBase request
     *
     * @param method ComponentBase's method
     * @param token TODO to be replaced by application ID
     * @param params JSON params. TODO add details
     *
     * @return JSON encoded response string
     */
    virtual std::string executeRequest(std::string method, std::string token, std::unique_ptr<IJson> params) = 0;
}; // class ComponentBase

} // namespace orb
