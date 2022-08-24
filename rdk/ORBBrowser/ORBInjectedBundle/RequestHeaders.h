/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */
#pragma once

#include <WPE/WebKit.h>
#include <WPE/WebKit/WKBundlePage.h>
#include <WPE/WebKit/WKURLRequest.h>

namespace WPEFramework {
namespace WebKit {

void SetRequestHeaders(WKBundlePageRef, WKTypeRef);
void RemoveRequestHeaders(WKBundlePageRef);
void ApplyRequestHeaders(WKBundlePageRef, WKURLRequestRef);

}  // WebKit
}  // WPEFramework
