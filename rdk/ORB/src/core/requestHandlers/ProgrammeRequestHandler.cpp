/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#include "ProgrammeRequestHandler.h"
#include "ORBPlatform.h"
#include "ORB.h"

using namespace WPEFramework::Plugin;

#define PROGRAMME_GET_PARENTAL_RATING "getParentalRating"
#define PROGRAMME_GET_SI_DESCRIPTORS "getSIDescriptors"

namespace orb {
/**
 * Constructor.
 */
ProgrammeRequestHandler::ProgrammeRequestHandler()
{
}

/**
 * Destructor.
 */
ProgrammeRequestHandler::~ProgrammeRequestHandler()
{
}

/**
 * @brief ProgrammeRequestHandler::Handle
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
bool ProgrammeRequestHandler::Handle(
    JsonObject token,
    std::string method,
    JsonObject params,
    JsonObject& response)
{
    bool ret = true;

    // Programme.getParentalRating
    if (method == PROGRAMME_GET_PARENTAL_RATING)
    {
        std::shared_ptr<ParentalRating> parentalRating = GetParentalRating();
        if (parentalRating != nullptr)
        {
            response.Set("result", parentalRating->ToJsonObject());
        }
    }
    // Programme.getSIDescriptors
    else if (method == PROGRAMME_GET_SI_DESCRIPTORS)
    {
        std::string ccid = params["ccid"].String();
        std::string programmeId = params["programmeID"].String();
        int descriptorTag = params["descriptorTag"].Number();
        int descriptorTagExtension = params["descriptorTagExtension"].Number();
        int privateDataSpecifier = params["privateDataSpecifier"].Number();
        std::vector<std::string> siDescriptors = ORB::instance(
            nullptr)->GetORBPlatform()->Programme_GetSiDescriptors(
            ccid, programmeId, descriptorTag, descriptorTagExtension, privateDataSpecifier);
        ArrayType<JsonValue> array;
        for (auto descriptor : siDescriptors)
        {
            array.Add(descriptor);
        }
        JsonValue jsonSiDescriptors;
        jsonSiDescriptors.Array(array);
        response.Set("result", jsonSiDescriptors);
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
 * Get the parental rating of the current programme of the currently tuned
 * broadcast channel.
 *
 * @return Pointer to the ParentalRating object or nullptr if no channel is set,
 *         or if the current programme has no parental rating.
 */
std::shared_ptr<ParentalRating> ProgrammeRequestHandler::GetParentalRating()
{
    fprintf(stderr, "[ProgrammeRequestHandler::GetParentalRating]\n");

    std::shared_ptr<Channel> currentChannel = ORB::instance(
        nullptr)->GetORBPlatform()->Broadcast_GetCurrentChannel();
    if (currentChannel == nullptr)
    {
        return nullptr;
    }
    std::string ccid = currentChannel->GetCcid();
    if (ccid.empty())
    {
        return nullptr;
    }
    std::vector<Programme> programmes = ORB::instance(
        nullptr)->GetORBPlatform()->Broadcast_GetProgrammes(ccid);
    if (programmes.empty())
    {
        return nullptr;
    }
    Programme currentProgramme = programmes[0];
    std::vector<ParentalRating> parentalRatings = currentProgramme.GetParentalRatings();
    if (parentalRatings.empty())
    {
        return nullptr;
    }
    return std::make_shared<ParentalRating>(parentalRatings[0]);
}
} // namespace orb
