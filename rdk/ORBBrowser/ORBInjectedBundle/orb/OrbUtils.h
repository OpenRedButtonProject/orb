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

// pre-processor mechanism so multiple inclusions don't cause compilation error
#ifndef __ORB_UTILS_H__
#define __ORB_UTILS_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//---Constant and macro definitions for public use-----------------------------

//---Enumerations for public use-----------------------------------------------

//---Global type defs for public use-------------------------------------------
typedef void (*OnDvbUrlLoadedNoData)(int requestId, unsigned int bufferLength, void *caller);

//---Global Function prototypes for public use---------------------------------

/**
 * Attempts to perform the JavaScript injection into the specified HTML source.
 *
 * @param htmlSource The HTML source
 * @param injected Output parameter holding the injection result (0 or !)
 *
 * @return The htmlSource after the injection, or NULL
 */
const char* InjectInto(const char *htmlSource, int *injected);

/**
 * Attempts to load the specified DVB URL.
 * The caller shall be informed of the result via the provided callback.
 *
 * @param url The DVB URL
 * @param caller Raw pointer to the caller object
 * @param callback Callback function
 */
void LoadDvbUrl(const char *url, void *caller, OnDvbUrlLoadedNoData callback);

#ifdef __cplusplus
}
#endif

#endif // __ORB_UTILS_H__
