/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
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
typedef void (*OnDvbUrlLoaded)(int requestId, unsigned int bufferLength, void *caller);

//---Global Function prototypes for public use---------------------------------

/**
 * Attempts to perform the JavaScript injection into the specified HTML source.
 * 
 * @param htmlSource The HTML source
 * @param injected Output parameter holding the injection result (0 or !)
 * 
 * @return The htmlSource after the injection, or NULL
 */
const char *InjectInto(const char *htmlSource, int *injected);

/**
 * Attempts to load the specified DVB URL.
 * The caller shall be informed of the result via the provided callback.
 *
 * @param url The DVB URL
 * @param caller Raw pointer to the caller object
 * @param callback Callback function
 */
void LoadDvbUrl(const char *url, void *caller, OnDvbUrlLoaded callback);

#ifdef __cplusplus
}
#endif

#endif // __ORB_UTILS_H__
