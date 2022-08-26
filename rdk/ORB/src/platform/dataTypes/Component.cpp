/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#include "Component.h"

namespace orb {
/**
 * Create a video component.
 *
 * @param componentTag
 * @param pid
 * @param encoding
 * @param encrypted
 * @param active
 * @param defaultComponent
 * @param hidden
 * @param aspectRatio
 *
 * @return Pointer to the new video component
 */
std::shared_ptr<Component> Component::CreateVideoComponent(
   int componentTag,
   int pid,
   std::string encoding,
   bool encrypted,
   bool active,
   bool defaultComponent,
   bool hidden,
   float aspectRatio
   )
{
   return std::make_shared<Component>(
      componentTag,
      pid,
      encoding,
      encrypted,
      active,
      defaultComponent,
      hidden,
      aspectRatio
      );
}

/**
 * Create a new audio component.
 *
 * @param componentTag
 * @param pid
 * @param encoding
 * @param encrypted
 * @param active
 * @param defaultComponent
 * @param hidden
 * @param language
 * @param audioDescription
 * @param audioChannels
 *
 * @return Pointer to the new audio component
 */
std::shared_ptr<Component> Component::CreateAudioComponent(
   int componentTag,
   int pid,
   std::string encoding,
   bool encrypted,
   bool active,
   bool defaultComponent,
   bool hidden,
   std::string language,
   bool audioDescription,
   int audioChannels
   )
{
   return std::make_shared<Component>(
      componentTag,
      pid,
      encoding,
      encrypted,
      active,
      defaultComponent,
      hidden,
      language,
      audioDescription,
      audioChannels
      );
}

/**
 * Create a new subtitle component.
 *
 * @param componentTag
 * @param pid
 * @param encoding
 * @param encrypted
 * @param active
 * @param defaultComponent
 * @param hidden
 * @param language
 * @param hearingImpaired
 * @param label
 *
 * @return Pointer to the new subtitle component
 */
std::shared_ptr<Component> Component::CreateSubtitleComponent(
   int componentTag,
   int pid,
   std::string encoding,
   bool encrypted,
   bool active,
   bool defaultComponent,
   bool hidden,
   std::string language,
   bool hearingImpaired,
   std::string label
   )
{
   return std::make_shared<Component>(
      componentTag,
      pid,
      encoding,
      encrypted,
      active,
      defaultComponent,
      hidden,
      language,
      hearingImpaired,
      label
      );
}

/**
 * Constructor for video components.
 *
 * @param componentTag
 * @param pid
 * @param encoding
 * @param encrypted
 * @param active
 * @param defaultComponent
 * @param hidden
 * @param aspectRatio
 */
Component::Component(
   int componentTag,
   int pid,
   std::string encoding,
   bool encrypted,
   bool active,
   bool defaultComponent,
   bool hidden,
   float aspectRatio
   )
{
   m_componentType = COMPONENT_TYPE_VIDEO;
   m_componentTag = componentTag;
   m_pid = pid;
   m_encoding = encoding;
   m_encrypted = encrypted;
   m_active = active;
   m_defaultComponent = defaultComponent;
   m_hidden = hidden;
   m_aspectRatio = aspectRatio;
}

/**
 * Constructor for audio components.
 *
 * @param componentTag
 * @param pid
 * @param encoding
 * @param encrypted
 * @param active
 * @param defaultComponent
 * @param hidden
 * @param language
 * @param audioDescription
 * @param audioChannels
 */
Component::Component(
   int componentTag,
   int pid,
   std::string encoding,
   bool encrypted,
   bool active,
   bool defaultComponent,
   bool hidden,
   std::string language,
   bool audioDescription,
   int audioChannels
   )
{
   m_componentType = COMPONENT_TYPE_AUDIO;
   m_componentTag = componentTag;
   m_pid = pid;
   m_encoding = encoding;
   m_encrypted = encrypted;
   m_active = active;
   m_defaultComponent = defaultComponent;
   m_hidden = hidden;
   m_language = language;
   m_audioDescription = audioDescription;
   m_audioChannels = audioChannels;
}

/**
 * Constructor for subtitle components.
 *
 * @param componentTag
 * @param pid
 * @param encoding
 * @param encrypted
 * @param active
 * @param defaultComponent
 * @param hidden
 * @param language
 * @param hearingImpaired
 * @param label
 */
Component::Component(
   int componentTag,
   int pid,
   std::string encoding,
   bool encrypted,
   bool active,
   bool defaultComponent,
   bool hidden,
   std::string language,
   bool hearingImpaired,
   std::string label
   )
{
   m_componentType = COMPONENT_TYPE_SUBTITLE;
   m_componentTag = componentTag;
   m_pid = pid;
   m_encoding = encoding;
   m_encrypted = encrypted;
   m_active = active;
   m_defaultComponent = defaultComponent;
   m_hidden = hidden;
   m_language = language;
   m_hearingImpaired = hearingImpaired;
   m_label = label;
}

/**
 * Destructor.
 */
Component::~Component()
{
}

int Component::GetComponentType() const
{
   return m_componentType;
}

int Component::GetComponentTag() const
{
   return m_componentTag;
}

int Component::GetPid() const
{
   return m_pid;
}

std::string Component::GetEncoding() const
{
   return m_encoding;
}

bool Component::IsEncrypted() const
{
   return m_encrypted;
}

bool Component::IsActive() const
{
   return m_active;
}

bool Component::IsDefaultComponent() const
{
   return m_defaultComponent;
}

bool Component::IsHidden() const
{
   return m_hidden;
}

std::string Component::GetLanguage() const
{
   return m_language;
}

bool Component::HasAudioDescription() const
{
   return m_audioDescription;
}

int Component::GetAudioChannels() const
{
   return m_audioChannels;
}

bool Component::IsHearingImpaired() const
{
   return m_hearingImpaired;
}

std::string Component::GetLabel() const
{
   return m_label;
}

float Component::GetAspectRatio() const
{
   return m_aspectRatio;
}

JsonObject Component::ToJsonObject() const
{
   // Video component
   if (m_componentType == COMPONENT_TYPE_VIDEO)
   {
      JsonObject json_videoComponent;
      json_videoComponent.Set("componentTag", GetComponentTag());
      json_videoComponent.Set("pid", GetPid());
      json_videoComponent.Set("type", COMPONENT_TYPE_VIDEO);
      json_videoComponent.Set("encoding", GetEncoding());
      json_videoComponent.Set("encrypted", IsEncrypted());
      json_videoComponent.Set("aspectRatio", GetAspectRatio());
      json_videoComponent.Set("active", IsActive());
      json_videoComponent.Set("default", IsDefaultComponent());
      if (IsHidden())
      {
         json_videoComponent.Set("hidden", true);
      }
      return json_videoComponent;
   }
   // Audio component
   else if (m_componentType == COMPONENT_TYPE_AUDIO)
   {
      JsonObject json_audioComponent;
      json_audioComponent.Set("componentTag", GetComponentTag());
      json_audioComponent.Set("pid", GetPid());
      json_audioComponent.Set("type", COMPONENT_TYPE_AUDIO);
      json_audioComponent.Set("encoding", GetEncoding());
      json_audioComponent.Set("encrypted", IsEncrypted());
      json_audioComponent.Set("language", GetLanguage());
      json_audioComponent.Set("audioDescription", HasAudioDescription());
      json_audioComponent.Set("audioChannels", GetAudioChannels());
      json_audioComponent.Set("active", IsActive());
      json_audioComponent.Set("default", IsDefaultComponent());
      if (IsHidden())
      {
         json_audioComponent.Set("hidden", true);
      }
      return json_audioComponent;
   }
   // Subtitle component
   else if (m_componentType == COMPONENT_TYPE_SUBTITLE)
   {
      JsonObject json_subtitleComponent;
      json_subtitleComponent.Set("componentTag", GetComponentTag());
      json_subtitleComponent.Set("pid", GetPid());
      json_subtitleComponent.Set("type", COMPONENT_TYPE_SUBTITLE);
      json_subtitleComponent.Set("encoding", GetEncoding());
      json_subtitleComponent.Set("encrypted", IsEncrypted());
      json_subtitleComponent.Set("language", GetLanguage());
      json_subtitleComponent.Set("hearingImpaired", IsHearingImpaired());
      json_subtitleComponent.Set("label", GetLabel());
      json_subtitleComponent.Set("active", IsActive());
      json_subtitleComponent.Set("default", IsDefaultComponent());
      if (IsHidden())
      {
         json_subtitleComponent.Set("hidden", true);
      }
      return json_subtitleComponent;
   }

   JsonObject json_invalidComponent;
   return json_invalidComponent;
}
} // namespace orb
