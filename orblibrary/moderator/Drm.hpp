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
    std::string executeRequest(const std::string& method, const std::string& token, const IJson& params) override;

private:
    /**
     * Handle getSupportedDRMSystemIDs request
     *
     * @return JSON response string with supported DRM system IDs
     */
    std::string handleGetSupportedDRMSystemIDs();

    /**
     * Handle sendDRMMessage request
     *
     * @param params JSON parameters containing message details
     * @return JSON response string with message result
     */
    std::string handleSendDRMMessage(const IJson& params);

    /**
     * Handle canPlayContent request
     *
     * @param params JSON parameters containing DRM private data and system ID
     * @return JSON response string with play capability result
     */
    std::string handleCanPlayContent(const IJson& params);

    /**
     * Handle canRecordContent request
     *
     * @param params JSON parameters containing DRM private data and system ID
     * @return JSON response string with record capability result
     */
    std::string handleCanRecordContent(const IJson& params);

    /**
     * Handle setActiveDRM request
     *
     * @param params JSON parameters containing DRM system ID
     * @return JSON response string with activation result
     */
    std::string handleSetActiveDRM(const IJson& params);

}; // class Drm

} // namespace orb

#endif // ORB_DRM_H