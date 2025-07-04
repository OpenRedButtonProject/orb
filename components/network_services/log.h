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
 * General logging for each supported platform
 *
 * Note: This file is part of the platform-agnostic application manager library.
 */

#ifndef OBS_NS_LOG_H
#define OBS_NS_LOG_H

#ifdef IS_CHROMIUM
#include <base/logging.h>
#elif defined(ANDROID)
#include <android-base/logging.h>
#else
#error Not Chromium or Android build
#endif


namespace orb { 
namespace networkServices {
    
#define LOGI(str)   LOG(INFO) << __FUNCTION__ << "," << __LINE__ << ": " << str
#define LOGE(str)   LOG(ERROR) << __FUNCTION__ << "," << __LINE__ << ": " << str
#define ENTER "ENTER"
#define LEAVE "LEAVE"

} // namespace networkServices
} // namespace orb

#endif // OBS_NS_LOG_H


