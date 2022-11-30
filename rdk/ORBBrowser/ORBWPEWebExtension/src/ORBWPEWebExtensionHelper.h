/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
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

    std::shared_ptr<ORBGenericClient> GetORBClient()
    {
        return m_orbClient;
    }

private:

    std::shared_ptr<ORBGenericClient> m_orbClient;
}; // class ORBWPEWebExtensionHelper
} // namespace orb
