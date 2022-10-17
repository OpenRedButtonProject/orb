#pragma once

#include "Module.h"

using namespace WPEFramework;

/**
 * JSON container for custom config options we want to use for
 * this plugin
 */
class ORBConfiguration : public Core::JSON::Container {
public:
    ORBConfiguration()
        : Core::JSON::Container()
    {
        // Map a json object name to c++ object
        Add(_T("privateComRpc"), &PrivateComRpcServer);
    }
    ~ORBConfiguration() = default;

    ORBConfiguration(const ORBConfiguration&) = delete;
    ORBConfiguration& operator=(const ORBConfiguration&) = delete;

public:
    Core::JSON::Boolean PrivateComRpcServer;
};