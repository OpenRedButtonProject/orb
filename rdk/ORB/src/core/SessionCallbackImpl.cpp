/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#include "SessionCallbackImpl.h"
#include "HttpDownloader.h"
#include "ORB.h"
#include "ORBEvents.h"
#include "ORBPlatform.h"

using namespace WPEFramework::Plugin;

namespace orb {

/**
 * Constructor.
 */
SessionCallbackImpl::SessionCallbackImpl()
{}

/**
 * Destructor.
 */
SessionCallbackImpl::~SessionCallbackImpl()
{}

/**
 * @brief SessionCallbackImpl::LoadApplication
 *
 * Tell the browser to load an application. If the entry page fails to load, the browser
 * should call ApplicationManager::OnLoadApplicationFailed.
 *
 * @param app_id    The application ID
 * @param url       The entry page URL
 */
void SessionCallbackImpl::LoadApplication(uint16_t app_id, const char *url)
{
  fprintf(stderr, "[SessionCallbackImpl::LoadApplication] app_id=%hu url=%s\n", app_id, url);
  ORB::instance(nullptr)->SetCurrentAppId(app_id);
  ORB::instance(nullptr)->GetORBPlatform()->Application_Load(url);
}

/**
 * @brief SessionCallbackImpl::ShowApplication
 *
 * Tell the browser to show the loaded application.
 */
void SessionCallbackImpl::ShowApplication()
{
  fprintf(stderr, "[SessionCallbackImpl::ShowApplication]\n");
  ORB::instance(nullptr)->GetORBPlatform()->Application_SetVisible(true);
}

/**
 * @brief SessionCallbackImpl::HideApplication
 *
 * Tell the browser to hide the loaded application.
 */
void SessionCallbackImpl::HideApplication()
{
  fprintf(stderr, "[SessionCallbackImpl::HideApplication]\n");
  ORB::instance(nullptr)->GetORBPlatform()->Application_SetVisible(false);
}

/**
 * @brief SessionCallbackImpl::GetXmlAitContents
 *
 * Perform an HTTP GET request and return the contents, which should be an XML AIT resource.
 *
 * @param url The URL to get
 *
 * @return The contents of the resource at URL
 */
std::string SessionCallbackImpl::GetXmlAitContents(const std::string &url)
{
  fprintf(stderr, "[SessionCallbackImpl::GetXmlAitContents] url=%s\n", url.c_str());
  std::unique_ptr<HttpDownloader> httpDownloader = std::make_unique<HttpDownloader>();
  std::shared_ptr<HttpDownloader::DownloadedObject> downloadedObject = httpDownloader->Download(url);
  if (downloadedObject != nullptr) {
    if (downloadedObject->GetContentType().rfind("application/vnd.dvb.ait+xml;", 0) == 0) {
      return downloadedObject->GetContent();
    }
  }
  return "";
}

/**
 * @brief SessionCallbackImpl::StopBroadcast
 *
 * Tell the broadcast-integration to stop presenting any broadcast component, equivalent to
 * selecting a null service.
 */
void SessionCallbackImpl::StopBroadcast()
{
  fprintf(stderr, "[SessionCallbackImpl::StopBroadcast]\n");
  ORB::instance(nullptr)->GetORBPlatform()->Broadcast_Stop();
}

/**
 * @brief SessionCallbackImpl::DispatchTransitionedToBroadcastRelatedEvent
 *
 * Tell the bridge to dispatch TransitionedToBroadcastRelated to the loaded application.
 */
void SessionCallbackImpl::DispatchTransitionedToBroadcastRelatedEvent()
{
  fprintf(stderr, "[SessionCallbackImpl::DispatchTransitionedToBroadcastRelatedEvent]\n");
  Event_OnAppTransitionedToBroadcastRelated();
}

/**
 * @brief SessionCallbackImpl::ResetBroadcastPresentation
 *
 * Tell the broadcast-integration to reset any calls by HbbTV to suspend presentation, set
 * the video rectangle or set the presented components.
 */
void SessionCallbackImpl::ResetBroadcastPresentation()
{
  fprintf(stderr, "[SessionCallbackImpl::ResetBroadcastPresentation]\n");
  ORB::instance(nullptr)->GetORBPlatform()->Broadcast_Reset();
}

void SessionCallbackImpl::DispatchApplicationLoadErrorEvent()
{
  fprintf(stderr, "TODO DispatchApplicationLoadErrorEvent !!!!!!!!!!!!!!!\n");
  return;
}

/**
 * Get the currently set parental control age.
 *
 * @return The currently set parental control age
 */
int SessionCallbackImpl::GetParentalControlAge()
{
  return ORB::instance(nullptr)->GetORBPlatform()->ParentalControl_GetAge();
}

/**
 * Get the 2-character country code of the current parental control.
 *
 * @return The 2-character country code
 */
std::string SessionCallbackImpl::GetParentalControlRegion()
{
  return ORB::instance(nullptr)->GetORBPlatform()->ParentalControl_GetRegion();
}

/**
 * Get the 3-character country code of the current parental control.
 *
 * @return The 3-character country code
 */
std::string SessionCallbackImpl::GetParentalControlRegion3()
{
  return ORB::instance(nullptr)->GetORBPlatform()->ParentalControl_GetRegion3();
}

} // namespace orb
