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

#include <wpe/webkit.h>

namespace orb
{
/**
 * This class is used to load dvb:// URLs on behalf of the browser.
 */
class ORBDVBURILoader
{
public:

    /**
     * Constructor.
     *
     * @param requestId The request identifier
     * @param request   The DVB URI scheme request
     */
    ORBDVBURILoader(int requestId, WebKitURISchemeRequest *request);

    /**
     * Destructor.
     */
    ~ORBDVBURILoader();

    /**
     * Start the load process by sending an asynchronous request to the ORB service.
     */
    void StartAsync();

    /**
     * Finish the load process by dispatching the retrieved content to the browser.
     */
    void Finish();

    /**
     * Set the dataReady flag to indicate that the content corresponding to the dvb URI is now
     * available.
     *
     * @param dataReady true/false
     */
    void SetDataReady(bool dataReady)
    {
        m_dataReady = dataReady;
    }

    /**
     * Set the loaded data.
     *
     * @param data       The data
     * @param dataLength The data length
     */
    void SetData(unsigned char *data, unsigned int dataLength);

    /**
     * Get the dataReady flag.
     *
     * @return true/false
     */
    bool IsDataReady() const
    {
        return m_dataReady;
    }

    /**
     * Get the loaded data.
     *
     * @return The loaded data or nullptr
     */
    unsigned char* GetData()
    {
        return m_data;
    }

    /**
     * Get the loaded data length.
     *
     * @return The data length
     */
    unsigned int GetDataLength()
    {
        return m_dataLength;
    }

private:

    int m_requestId;
    WebKitURISchemeRequest *m_request;
    bool m_dataReady;
    unsigned char *m_data;
    unsigned int m_dataLength;
}; // class ORBDVBURILoader
} // namespace orb
