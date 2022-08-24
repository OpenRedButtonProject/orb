/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#pragma once

#include "application_manager.h"

namespace orb {

/**
 * @brief orb::SessionCallback
 *
 * Implementation of the session callback being used by the application manager
 * to interact with the integration components.
 */
class SessionCallbackImpl : public ApplicationManager::SessionCallback {

public:

  /**
   * Constructor.
   */
  SessionCallbackImpl();

  /**
   * Destructor.
   */
  ~SessionCallbackImpl();

  /**
   * @brief SessionCallbackImpl::LoadApplication
   *
   * Tell the browser to load an application. If the entry page fails to load, the browser
   * should call ApplicationManager::OnLoadApplicationFailed.
   *
   * @param app_id    The application ID
   * @param url       The entry page URL
   */
  virtual void LoadApplication(uint16_t app_id, const char *url) override;

  /**
   * @brief SessionCallbackImpl::ShowApplication
   *
   * Tell the browser to show the loaded application.
   */
  virtual void ShowApplication() override;

  /**
   * @brief SessionCallbackImpl::HideApplication
   *
   * Tell the browser to hide the loaded application.
   */
  virtual void HideApplication() override;

  /**
   * @brief SessionCallbackImpl::GetXmlAitContents
   *
   * Perform an HTTP GET request and return the contents, which should be an XML AIT resource.
   *
   * @param url The URL to get
   *
   * @return The contents of the resource at URL
   */
  virtual std::string GetXmlAitContents(const std::string &url) override;

  /**
   * @brief SessionCallbackImpl::StopBroadcast
   *
   * Tell the broadcast-integration to stop presenting any broadcast component, equivalent to
   * selecting a null service.
   */
  virtual void StopBroadcast() override;
  
  /**
   * @brief SessionCallbackImpl::DispatchTransitionedToBroadcastRelatedEvent
   *
   * Tell the bridge to dispatch TransitionedToBroadcastRelated to the loaded application.
   */
  virtual void DispatchTransitionedToBroadcastRelatedEvent() override;

  /**
   * @brief SessionCallbackImpl::ResetBroadcastPresentation
   *
   * Tell the broadcast-integration to reset any calls by HbbTV to suspend presentation, set
   * the video rectangle or set the presented components.
   */
  virtual void ResetBroadcastPresentation() override;

  /**
   *  Tell the bridge to dispatch ApplicationLoadError to the loaded application.
   */
  virtual void DispatchApplicationLoadErrorEvent() override;

  /**
   * Get the currently set parental control age.
   *
   * @return The currently set parental control age
   */
  virtual int GetParentalControlAge() override;

  /**
   * Get the 2-character country code of the current parental control.
   *
   * @return The 2-character country code
   */
  virtual std::string GetParentalControlRegion() override;

  /**
   * Get the 3-character country code of the current parental control.
   *
   * @return The 3-character country code
   */
  virtual std::string GetParentalControlRegion3() override;

}; // class SessionCallbackImpl

} // namespace orb