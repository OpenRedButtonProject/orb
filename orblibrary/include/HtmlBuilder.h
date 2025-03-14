#ifndef ORB_HTML_BUILDER_H
#define ORB_HTML_BUILDER_H

#include <string>
#include <string_view>

namespace orb::polyfill
{
class HtmlBuilder
{
public:
  static constexpr std::string_view script_tag_open = "//<![CDATA[\n";
  static constexpr std::string_view script_tag_close = "\n//]]>\n";

  HtmlBuilder();
  ~HtmlBuilder() = default;

  static const std::string getHbbtvJs();

  const std::string getHbbtvInjection();

  // TODO Add remaining items:
  // See android/orblibrary/src/main/java/org/orbtv/orblibrary/HtmlBuilder.java
};

} // namespace orb::polyfill

#endif // ORB_HTML_BUILDER_H
