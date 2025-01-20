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

#include <android/binder_auto_utils.h>

#include "org/orbtv/orbservice/BnDvbiSession.h"

#ifdef NDK_AIDL
#define STATUS ndk::ScopedAStatus
#define SH_PTR std::shared_ptr
#else
#define STATUS ::android::binder::Status
#define SH_PTR ::android::sp
#endif

using namespace std;

#ifdef NDK_AIDL
namespace aidl {
#endif

namespace org::orbtv::orbservice {

class DvbiSession : public BnDvbiSession {
public:
   DvbiSession();

public:
   STATUS getPreferredUILanguage(vector<uint8_t>* _aidl_return) override;
   STATUS getCountryId(vector<uint8_t>* _aidl_return) override;
   STATUS getSubtitlesEnabled(bool* _aidl_return) override;
   STATUS getAudioDescriptionEnabled(bool* _aidl_return) override;
   STATUS getCurrentCcid(vector<uint8_t>* _aidl_return) override;

};

} // namespace org::orbtv::orbservice

#ifdef NDK_AIDL
} // namespace aidl
#endif
