/**
 * ORB Software. Copyright (c) 2023 Ocean Blue Software Limited
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

 #include <android/log.h>

 #include "BrowserCallback.h"

 #define TAG                "BrowserCallback"
 #define LOGI(x, ...)    __android_log_print(ANDROID_LOG_INFO, TAG, "%s:%u " x "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__);
 #define LOGE(x, ...)    __android_log_print(ANDROID_LOG_ERROR, TAG, "%s:%u " x "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__);

using namespace std;
using org::orbtv::orbservice::BrowserCallback;
using org::orbtv::orbservice::IBrowserSession;


bool BrowserCallback::dispatchKeyEvent(int32_t action, int32_t key_code)
{
    return false;
}

void BrowserCallback::loadApplication(std::string app_id, std::string url)
{
}

void BrowserCallback::showApplication()
{
}

void BrowserCallback::hideApplication()
{
}

void BrowserCallback::dispatchEvent(string type, string properties)
{
}

void BrowserCallback::provideDsmccContent(string url, const vector<uint8_t>& content)
{
}
