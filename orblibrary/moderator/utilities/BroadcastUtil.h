#ifndef BROADCAST_UTIL_H
#define BROADCAST_UTIL_H

#include "IPlatform.h"
#include "JsonUtil.h"

namespace orb
{
class BroadcastUtil
{
public:
    static Json::Value convertChannelListToJson(std::vector<Channel> channels);
    static Json::Value convertChannelToJson(Channel channel);
    static bool isIpChannel(std::shared_ptr<Channel> channel);
};
}

#endif // BROADCAST_UTIL_H