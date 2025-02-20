#include <iostream>
#include <string>

#include "testing/gtest/include/gtest/gtest.h"

extern "C" {
__attribute__((visibility(
    "default"))) extern const char _binary_gen_third_party_orb_hbbtv_js_start[];
__attribute__((visibility(
    "default"))) extern const char _binary_gen_third_party_orb_hbbtv_js_end[];
}

std::string getEmbeddedJS() {
  return std::string(_binary_gen_third_party_orb_hbbtv_js_start,
                     _binary_gen_third_party_orb_hbbtv_js_end -
                         _binary_gen_third_party_orb_hbbtv_js_start - 1);
}

TEST(OrbPolyfill, TestPolyfillExists) {
  std::string js_polyfill_str = getEmbeddedJS();

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
}
