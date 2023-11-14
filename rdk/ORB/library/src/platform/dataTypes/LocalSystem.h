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

namespace orb
{
/**
 * @brief orb::LocalSystem
 *
 * Representation of the local system.
 * (See OIPF DAE spec section 7.3.3 an in particular 7.3.3.2)
 */
class LocalSystem
{
public:

    /**
     * Constructor.
     *
     * @param vendorName      String identifying the vendor name of the device
     * @param modelName       String identifying the model name of the device
     * @param softwareVersion String identifying the version number of the platform firmware
     * @param hardwareVersion String identifying the version number of the platform hardware
     */
    LocalSystem(
        std::string vendorName,
        std::string modelName,
        std::string softwareVersion,
        std::string hardwareVersion
        )
        : m_vendorName(vendorName)
        , m_modelName(modelName)
        , m_softwareVersion(softwareVersion)
        , m_hardwareVersion(hardwareVersion)
    {
    }

    /**
     * Destructor.
     */
    ~LocalSystem()
    {
    }

    std::string GetVendorName() const
    {
        return m_vendorName;
    }

    std::string GetModelName() const
    {
        return m_modelName;
    }

    std::string GetSoftwareVersion() const
    {
        return m_softwareVersion;
    }

    std::string GetHardwareVersion() const
    {
        return m_hardwareVersion;
    }

private:

    std::string m_vendorName;
    std::string m_modelName;
    std::string m_softwareVersion;
    std::string m_hardwareVersion;
}; // class LocalSystem
} // namespace orb