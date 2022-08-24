/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#include "ParentalControlRequestHandler.h"
#include "ORBPlatform.h"
#include "ORB.h"

using namespace WPEFramework::Plugin;

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
    JsonObject token,
    std::string method,
    JsonObject params,
    JsonObject& response)
{
    bool ret = true;

    // ParentalControl.getRatingSchemes
    if (method == PARENTAL_CONTROL_GET_RATING_SCHEMES)
    {
        typedef std::map<std::string, std::vector<ParentalRating> > RatingSchemes;
        RatingSchemes ratingSchemes = GetRatingSchemes();
        for (RatingSchemes::const_iterator it = ratingSchemes.begin(); it != ratingSchemes.end();
             ++it)
        {
            std::string scheme = it->first;
            std::vector<ParentalRating> ratings = it->second;
            ArrayType<JsonValue> array;
            for (auto rating : ratings)
            {
                array.Add(rating.ToJsonObject());
            }
            JsonValue jsonRatings;
            jsonRatings.Array(array);
            response.Set(scheme.c_str(), jsonRatings);
        }
    }
    // ParentalControl.getThreshold
    else if (method == PARENTAL_CONTROL_GET_THRESHOLD)
    {
        std::shared_ptr<ParentalRating> threshold = GetThreshold(params);
        response = threshold->ToJsonObject();
    }
    // ParentalControl.isRatingBlocked
    else if (method == PARENTAL_CONTROL_IS_RATING_BLOCKED)
    {
        bool blocked = IsRatingBlocked(params);
        response["value"] = blocked;
    }
    // UnknownMethod
    else
    {
        response = RequestHandler::MakeErrorResponse("UnknownMethod");
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
        ORB::instance(nullptr)->GetORBPlatform()->ParentalControl_GetRatingSchemes();
    return schemes;
}

/**
 * Get the parental rating threshold currently set on the system for the provided scheme.
 *
 * @param params The request parameters
 *
 * @return The parental rating threshold
 */
std::shared_ptr<ParentalRating> ParentalControlRequestHandler::GetThreshold(JsonObject params)
{
    std::shared_ptr<ParentalRating> threshold = ORB::instance(
        nullptr)->GetORBPlatform()->ParentalControl_GetThreshold(
        params["scheme"].String()
        );
    return threshold;
}

/**
 * Retrieve the blocked property for the provided parental rating.
 *
 * @param params The request parameters
 *
 * @return The blocked property
 */
bool ParentalControlRequestHandler::IsRatingBlocked(JsonObject params)
{
    bool blocked = ORB::instance(nullptr)->GetORBPlatform()->ParentalControl_IsRatingBlocked(
        params["scheme"].String(),
        params["region"].String(),
        params["value"].Number()
        );
    return blocked;
}
} // namespace orb
