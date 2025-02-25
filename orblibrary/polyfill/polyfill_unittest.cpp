#include <iostream>
#include <string>

#include "testing/gtest/include/gtest/gtest.h"
#include "HtmlBuilder.h"


TEST(OrbPolyfill, TestPolyfillExists) {
  std::string js_polyfill_str = orb::polyfill::HtmlBuilder::getHbbtvJs();

  EXPECT_GT(js_polyfill_str.size(), 0u);

  // Test for the existence of a few substrings

  // src/housekeeping/banner.js
  const std::string banner(
      "ORB Software. Copyright (c) 2022 Ocean Blue Software Limited");

  EXPECT_EQ(js_polyfill_str.find(banner), 6u);

  // src/housekeeping/beginiffe.js
  const std::string beginiffe(
    R"((function() {

    "use strict";
    let hbbtv = {};

    const defaultEntities = {
        URL: URL,
        Map: Map
    };)");

  EXPECT_EQ(js_polyfill_str.find(beginiffe), 627u); // Care: these values can change!!

  const std::string run_js(
    R"(hbbtv.core.initialise();)");

  EXPECT_EQ(js_polyfill_str.find(run_js), 573429u); // Care: these values can change!!

  // Check the very last character, typically a newline
  EXPECT_EQ(js_polyfill_str.back(), '\n');
}

TEST(OrbPolyfill, TestPolyfillInjection)
{
  orb::polyfill::HtmlBuilder builder;

  const std::string polyfill_injection = builder.getHbbtvInjection();

  EXPECT_EQ(polyfill_injection.find(
    orb::polyfill::HtmlBuilder::script_tag_open), 0u);
  EXPECT_EQ(polyfill_injection.find(
    orb::polyfill::HtmlBuilder::script_tag_close),
    polyfill_injection.length() - orb::polyfill::HtmlBuilder::script_tag_close.length());
}
