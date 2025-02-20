#include "HtmlBuilder.h"

extern "C" {
  __attribute__((visibility(
      "default"))) extern const char _binary_gen_third_party_orb_hbbtv_js_start[];
  __attribute__((visibility(
      "default"))) extern const char _binary_gen_third_party_orb_hbbtv_js_end[];
}

namespace orb::polyfill
{
//static
const std::string HtmlBuilder::getHbbtvInjection()
{
  return std::string(_binary_gen_third_party_orb_hbbtv_js_start,
    _binary_gen_third_party_orb_hbbtv_js_end -
        _binary_gen_third_party_orb_hbbtv_js_start);
}

} // namespace orb::polyfill

