/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */
#pragma once

#include <map>
#include <vector>
#include "Channel.h"
#include "Programme.h"

using namespace orb;

class DVB
{
public:

    DVB();
    ~DVB();

public:

    void Initialise();
    void Finalise();

public:

    std::vector<Channel> GetChannels();
    std::vector<Programme> GetProgrammes(std::string ccid);

private:

    std::vector<Channel> m_channels;
    std::map<std::string, std::vector<Programme> > m_programmes;
};