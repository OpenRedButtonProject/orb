/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */
#pragma once

#include <WPE/WebKit.h>

namespace WPEFramework {
namespace JavaScript {
namespace WPEBridge {

void Initialise();
void InjectJS(WKBundleFrameRef frame);
bool HandleMessageToPage(WKBundlePageRef page, WKStringRef messageName, WKTypeRef messageBody);

} // WPEBridge
} // JavaScript
} // WPEFramework
