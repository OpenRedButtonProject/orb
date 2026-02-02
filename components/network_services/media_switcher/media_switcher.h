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

#ifndef HBBTV_MEDIA_SWITCHER_H
#define HBBTV_MEDIA_SWITCHER_H

#include <string>
#include <memory>
#include <map>

namespace NetworkServices {

class MediaSwitcher {
public:
    MediaSwitcher(const MediaSwitcher &other) = delete;
    MediaSwitcher &operator=(const MediaSwitcher &other) = delete;

    MediaSwitcher() = default;
    ~MediaSwitcher() = default;

    // Media switching operations
    // Note: Timeline monitoring is handled by MediaSynchroniserManager to avoid duplication
    bool switchMediaPresentation(const std::string &paramsJson);

private:
    // TODO: Add private members as needed
};

class MediaSwitcherManager {
public:
    MediaSwitcherManager(const MediaSwitcherManager &other) = delete;
    MediaSwitcherManager &operator=(const MediaSwitcherManager &other) = delete;

    MediaSwitcherManager() = default;
    ~MediaSwitcherManager() = default;

    int createMediaSwitcher();
    void destroyMediaSwitcher(int id);
    MediaSwitcher* getMediaSwitcher(int id);
    void releaseResources();

private:
    std::map<int, std::unique_ptr<MediaSwitcher>> m_switchers;
    int m_nextId = 1;
};

} // namespace NetworkServices

#endif // HBBTV_MEDIA_SWITCHER_H
