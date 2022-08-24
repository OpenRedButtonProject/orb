/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#ifndef __BROWSERCONSOLELOG_H
#define __BROWSERCONSOLELOG_H

#include <tracing/tracing.h>
#ifndef WEBKIT_GLIB_API
#include "ORBInjectedBundle/Utils.h"
#endif
using namespace WPEFramework;

class BrowserConsoleLog {
private:
    BrowserConsoleLog() = delete;
    BrowserConsoleLog(const BrowserConsoleLog& a_Copy) = delete;
    BrowserConsoleLog& operator=(const BrowserConsoleLog& a_RHS) = delete;

public:
#ifdef WEBKIT_GLIB_API
    BrowserConsoleLog(const string& message, const uint64_t line, const uint64_t column)
    {
        _text = '[' + Core::NumberType<uint64_t>(line).Text() + ',' + Core::NumberType<uint64_t>(
            column).Text() + ']' + message;
        const uint16_t maxStringLength = Trace::TRACINGBUFFERSIZE - 1;
        if (_text.length() > maxStringLength)
        {
            _text = _text.substr(0, maxStringLength);
        }
    }

#else
    BrowserConsoleLog(const WKStringRef message, const uint64_t line, const uint64_t column)
    {
        _text = '[' + Core::NumberType<uint64_t>(line).Text() + ',' + Core::NumberType<uint64_t>(
            column).Text() + ']' + WebKit::Utils::WKStringToString(message);
        const uint16_t maxStringLength = Trace::TRACINGBUFFERSIZE - 1;
        if (_text.length() > maxStringLength)
        {
            _text = _text.substr(0, maxStringLength);
        }
    }

#endif
    ~BrowserConsoleLog()
    {
    }

public:
    inline const char* Data() const
    {
        return(_text.c_str());
    }

    inline uint16_t Length() const
    {
        return(static_cast<uint16_t>(_text.length()));
    }

private:
    std::string _text;
};

#endif // __BROWSERCONSOLELOG_H
