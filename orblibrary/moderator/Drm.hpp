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
#ifndef ORB_DRM_H
#define ORB_DRM_H

#include <string>
#include <json/json.h>

#include "ComponentBase.hpp"

namespace orb
{

class Drm : public ComponentBase
{
public:
    Drm() = default;
    virtual ~Drm() = default;

    /**
     * DRM component request handler
     *
     * @param method DRM's method
     * @param token TODO to be replaced by application ID
     * @param params JSON params. TODO add details
     *
     * @return JSON encoded response string
     */
    std::string executeRequest(std::string method, std::string token, std::unique_ptr<IJson> params) override;

private:
    /**
     * Handle getSupportedDRMSystemIDs request
     *
     * @return JSON response with supported DRM system IDs
     */
    Json::Value handleGetSupportedDRMSystemIDs();

    /**
     * Handle sendDRMMessage request
     *
     * @param params JSON parameters containing message details
     * @return JSON response with message result
     */
    Json::Value handleSendDRMMessage(const Json::Value& params);

    /**
     * Handle canPlayContent request
     *
     * @param params JSON parameters containing DRM private data and system ID
     * @return JSON response with play capability result
     */
    Json::Value handleCanPlayContent(const Json::Value& params);

    /**
     * Handle canRecordContent request
     *
     * @param params JSON parameters containing DRM private data and system ID
     * @return JSON response with record capability result
     */
    Json::Value handleCanRecordContent(const Json::Value& params);

    /**
     * Handle setActiveDRM request
     *
     * @param params JSON parameters containing DRM system ID
     * @return JSON response with activation result
     */
    Json::Value handleSetActiveDRM(const Json::Value& params);

}; // class Drm

} // namespace orb

#endif // ORB_DRM_H