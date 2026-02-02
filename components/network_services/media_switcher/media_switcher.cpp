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
 */

#include "media_switcher.h"
#include "../log.h"

namespace NetworkServices {

bool MediaSwitcher::switchMediaPresentation(const std::string &paramsJson)
{
    // TODO: Implement media switching logic
    LOGI("");
    return true;
}


int MediaSwitcherManager::createMediaSwitcher()
{
    int id = m_nextId++;
    m_switchers[id] = std::make_unique<MediaSwitcher>();
    return id;
}

void MediaSwitcherManager::destroyMediaSwitcher(int id)
{
    auto it = m_switchers.find(id);
    if (it != m_switchers.end()) {
        m_switchers.erase(it);
    }
}

MediaSwitcher* MediaSwitcherManager::getMediaSwitcher(int id)
{
    auto it = m_switchers.find(id);
    if (it != m_switchers.end()) {
        return it->second.get();
    }
    return nullptr;
}

void MediaSwitcherManager::releaseResources()
{
    m_switchers.clear();
    m_nextId = 1;
}

} // namespace NetworkServices
