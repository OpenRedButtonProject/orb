#include "HtmlBuilder.h"

extern "C" {
  __attribute__((visibility(
      "default"))) extern const char _binary_gen_third_party_orb_hbbtv_js_start[];
  __attribute__((visibility(
      "default"))) extern const char _binary_gen_third_party_orb_hbbtv_js_end[];
}

namespace orb::polyfill
{
HtmlBuilder::HtmlBuilder()
{

}

//static
const std::string HtmlBuilder::getHbbtvJs()
{
  return std::string(_binary_gen_third_party_orb_hbbtv_js_start,
    _binary_gen_third_party_orb_hbbtv_js_end -
        _binary_gen_third_party_orb_hbbtv_js_start);
}

const std::string HtmlBuilder::getHbbtvInjection()
{
  return std::string(script_tag_open) + getHbbtvJs() + std::string(script_tag_close);
}

} // namespace orb::polyfill

