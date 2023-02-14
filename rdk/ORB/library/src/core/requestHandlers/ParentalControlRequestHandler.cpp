/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#include "ParentalControlRequestHandler.h"
#include "ORBPlatform.h"
#include "ORBEngine.h"
#include "JsonUtil.h"

#define PARENTAL_CONTROL_GET_RATING_SCHEMES "getRatingSchemes"
#define PARENTAL_CONTROL_GET_THRESHOLD "getThreshold"
#define PARENTAL_CONTROL_IS_RATING_BLOCKED "isRatingBlocked"

namespace orb {
/**
 * Constructor.
 */
ParentalControlRequestHandler::ParentalControlRequestHandler()
{
}

/**
 * Destructor.
 */
ParentalControlRequestHandler::~ParentalControlRequestHandler()
{
}

/**
 * @brief ParentalControlRequestHandler::Handle
 *
 * Handles the given Programme request.
 *
 * @param token    (in)  The request token
 * @param method   (in)  The requested method
 * @param params   (in)  A JSON object containing the input parameters (if any)
 * @param response (out) A JSON object containing the response
 *
 * @return true in success, otherwise false
 */
bool ParentalControlRequestHandler::Handle(
    json token,
    std::string method,
    json params,
    json& response)
{
    bool ret = true;

    // ParentalControl.getRatingSchemes
    if (method == PARENTAL_CONTROL_GET_RATING_SCHEMES)
    {
        typedef std::map<std::string, std::vector<ParentalRating> > RatingSchemes;
        RatingSchemes ratingSchemes = GetRatingSchemes();
        json resultsArray;
        for (RatingSchemes::const_iterator it = ratingSchemes.begin(); it != ratingSchemes.end();
             ++it)
        {
            std::string scheme = it->first;
            std::vector<ParentalRating> ratings = it->second;
            json array;
            json schemeObject;
            for (auto rating : ratings)
            {
                array.push_back(JsonUtil::ParentalRatingToJsonObject(rating));
            }
            schemeObject["name"] = scheme;
            schemeObject["ratings"] = array;
            resultsArray.push_back(schemeObject);
        }
        response.emplace("result", resultsArray);
    }
    // ParentalControl.getThreshold
    else if (method == PARENTAL_CONTROL_GET_THRESHOLD)
    {
        std::shared_ptr<ParentalRating> threshold = GetThreshold(params);
        response["result"] = JsonUtil::ParentalRatingToJsonObject(*(threshold.get()));
    }
    // ParentalControl.isRatingBlocked
    else if (method == PARENTAL_CONTROL_IS_RATING_BLOCKED)
    {
        bool blocked = IsRatingBlocked(params);
        response["result"] = blocked;
    }
    // UnknownMethod
    else
    {
        response = ORBBridgeRequestHandler::MakeErrorResponse("UnknownMethod");
        ret = false;
    }

    return ret;
}

/**
 * Get the rating schemes supported by the system.
 *
 * @return The rating schemes supported by the system
 */
std::map<std::string,
         std::vector<ParentalRating> > ParentalControlRequestHandler::GetRatingSchemes()
{
    std::map<std::string, std::vector<ParentalRating> > schemes =
        ORBEngine::GetSharedInstance().GetORBPlatform()->ParentalControl_GetRatingSchemes();
    return schemes;
}

/**
 * Get the parental rating threshold currently set on the system for the provided scheme.
 *
 * @param params The request parameters
 *
 * @return The parental rating threshold
 */
std::shared_ptr<ParentalRating> ParentalControlRequestHandler::GetThreshold(json params)
{
    std::shared_ptr<ParentalRating> threshold =
        ORBEngine::GetSharedInstance().GetORBPlatform()->ParentalControl_GetThreshold(params.value(
            "scheme", ""));
    return threshold;
}

/**
 * Retrieve the blocked property for the provided parental rating.
 *
 * @param params The request parameters
 *
 * @return The blocked property
 */
bool ParentalControlRequestHandler::IsRatingBlocked(json params)
{
    bool blocked = ORBEngine::GetSharedInstance().GetORBPlatform()->ParentalControl_IsRatingBlocked(
        params.value("scheme", ""),
        params.value("region", ""),
        params.value("value", -1)
        );
    return blocked;
}
} // namespace orb
