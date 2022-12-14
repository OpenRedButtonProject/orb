/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */
#pragma once

#include <string>

namespace orb
{
class ORBBrowserApi
{
public:

    /**
     * Execute the given bridge request.
     * The request is a string representation of a JSON object with the following form:
     *
     * {
     *    "token": <token>
     *    "method": <method>
     *    "params": <params>
     * }
     *
     * The response is also a string representation of a JSON object containing the results, if any.
     *
     * @param jsonRequest String representation of the JSON request
     *
     * @return A string representation of the JSON response
     */
    virtual std::string ExecuteBridgeRequest(std::string jsonRequest) = 0;

    /**
     * Create a new JSON token for the current application and the given URI.
     *
     * @param uri The given URI
     *
     * @return A string representation of the resulting JSON token
     */
    virtual std::string CreateToken(std::string uri) = 0;

    /**
     * Load the specified DVB URL through the underlying platform DSM-CC implementation.
     *
     * @param url       The DVB URL
     * @param requestId The distinctive request id
     */
    virtual void LoadDvbUrl(std::string url, int requestId) = 0;

    /**
     * Notify the application manager and the current JavaScript context that the specified HbbTV
     * application has failed to load.
     *
     * @param url              The application URL
     * @param errorDescription The error description
     */
    virtual void NotifyApplicationLoadFailed(std::string url, std::string errorDescription) = 0;

    /**
     * Notify the application manager that the page of the current HbbTV application has changed
     * and is about to load.
     *
     * @param url The application page URL
     */
    virtual void NotifyApplicationPageChanged(std::string url) = 0;

    /**
     * Get the User-Agent string.
     *
     * @return The User-Agent string
     */
    virtual std::string GetUserAgentString() = 0;
}; // class ORBBrowserApi
} // namespace orb
