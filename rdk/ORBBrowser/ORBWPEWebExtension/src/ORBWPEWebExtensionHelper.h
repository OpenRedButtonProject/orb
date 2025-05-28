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

#include <memory>
#include <wpe/webkit.h>
#include <core/core.h>

#include "ORBGenericClient.h"

namespace orb
{
/**
 * Helper class for the ORB wpe web extension.
 */
class ORBWPEWebExtensionHelper
{
public:

    /**
     * Singleton.
     */
    static ORBWPEWebExtensionHelper& GetSharedInstance()
    {
        static ORBWPEWebExtensionHelper s_sharedInstance;
        return s_sharedInstance;
    }

private:

    /**
     * Constructor.
     */
    ORBWPEWebExtensionHelper();

public:

    /**
     * Destructor.
     */
    ~ORBWPEWebExtensionHelper();

public:

    /**
     * Perform initialisation tasks related to the ORB WPE web extension.
     *
     * @param context Pointer to the WebKit web context
     */
    void InitialiseWebExtension(WebKitWebContext *context);

    /**
     * Create and properly set up the WebKit user content manager for injecting the ORB JavaScript.
     *
     * @return Pointer to the WebKit user content manager
     */
    WebKitUserContentManager* CreateWebKitUserContentManager();

    /**
     * Register the DVB URL scheme handler.
     *
     * @param context Pointer to the WebKit web context
     */
    void RegisterDVBURLSchemeHandler(WebKitWebContext *context);

    /**
     * Set custom preferences for the ORB browser.
     *
     * @param preferences        Pointer to the browser's global settings
     * @param jsonConfigAsString String containing the JSON representation of the browser config
     */
    void SetORBWPEWebExtensionPreferences(WebKitSettings *preferences, std::string
        jsonConfigAsString);

    /**
    * Get the mimetype based on the given url
    *
    * @param url The url to parse
    * @return The mimetype or * / * for unkown
    */
    std::string GetMimeTypeFromUrl(std::string url);

    std::shared_ptr<ORBGenericClient> GetORBClient()
    {
        return m_orbClient;
    }

private:

    std::shared_ptr<ORBGenericClient> m_orbClient;
    std::map<std::string, std::string> m_mimetypeMap;
}; // class ORBWPEWebExtensionHelper
} // namespace orb
