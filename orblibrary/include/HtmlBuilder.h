#ifndef ORB_HTML_BUILDER_H
#define ORB_HTML_BUILDER_H

#include <string>

namespace orb::polyfill
{
class HtmlBuilder
{
public:
  HtmlBuilder();
  ~HtmlBuilder() = default;

  static const std::string getHbbtvInjection();
};

} // namespace orb::polyfill

#endif // ORB_HTML_BUILDER_H
