#include <iostream>
#include <string>

#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/orb/orblibrary/include/Moderator.h"

TEST(OrbModerator, TestModeratorInvalidRequest)
{
  // GIVEN: an orb::Moderator object
  orb::Moderator moderator;

  // WHEN: executeRequest is called with an empty string
  std::string response = moderator.executeRequest("");

  // THEN: an invalid JSON error reponse is returned
  EXPECT_EQ(response, "{\"error\": \"Invalid Request\"}");
}

TEST(OrbModerator, TestModeratorValidRequestAppManager)
{
  // GIVEN: an orb::Moderator object
  orb::Moderator moderator;

  // WHEN: executeRequest is called with a valid request for Manager,
  std::string json_request = "TODO";

  std::string response = moderator.executeRequest(json_request);

  // THEN: a valid reponse is returned
  EXPECT_EQ(response, " SOME RESPONSE ");
}

// TODO add tests for
// - Network and MediaSynchroniser
// - DVB client? Null DVB client

// Unit tests are required for each method to ensure these are properly formed and valid.
