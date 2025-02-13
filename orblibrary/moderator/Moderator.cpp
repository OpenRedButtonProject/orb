/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * ORB Moderator
 *
 */

#include "Moderator.h"
#include "log.h"

using namespace std;

namespace orb
{

Moderator::Moderator()
{
}

Moderator::~Moderator()
{
}

// Set Orb Browser callback object
void Moderator::setBrowserCallback(IBrowser* browser)
{
    mBrowser = browser;
    LOGI("HbbTV version " << ORB_HBBTV_VERSION)
}

string Moderator::executeRequest(string jsonRequest)
{
    string response = "{\"error\": \"Request not implemented\"}";

    LOGI("json: " << jsonRequest)

    return response;
}

void Moderator::notifyApplicationPageChanged(string url)
{
    LOGI("url: " << url)
}

void Moderator::notifyApplicationLoadFailed(string url, string errorText)
{
    LOGI("url: " << url << " err: " << errorText)
}

void Moderator::getDvbContent(string url)
{
    LOGI("url: " << url)
}

string Moderator::getUserAgentString()
{
    string user_agent = "todo";
    LOGI("")
    return user_agent;
}

} // namespace orb
