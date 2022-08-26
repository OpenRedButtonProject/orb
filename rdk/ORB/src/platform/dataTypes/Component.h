/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#pragma once

#include <core/core.h>
#include <memory>

using namespace WPEFramework::Core::JSON;

// Supported component types
#define COMPONENT_TYPE_ANY -1
#define COMPONENT_TYPE_VIDEO 0
#define COMPONENT_TYPE_AUDIO 1
#define COMPONENT_TYPE_SUBTITLE 2

namespace orb {
/**
 * @brief orb::Component
 *
 * Representation of video/audio/subtitle components.
 */
class Component {
public:

   static std::shared_ptr<Component> CreateVideoComponent(
      int componentTag,
      int pid,
      std::string encoding,
      bool encrypted,
      bool active,
      bool defaultComponent,
      bool hidden,
      float aspectRatio
      );

   static std::shared_ptr<Component> CreateAudioComponent(
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
      );

   static std::shared_ptr<Component> CreateSubtitleComponent(
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
      );

   ~Component();

   // common
   int GetComponentType() const;
   int GetComponentTag() const;
   int GetPid() const;
   std::string GetEncoding() const;
   bool IsEncrypted() const;
   bool IsActive() const;
   bool IsDefaultComponent() const;
   bool IsHidden() const;

   // audio/subtitle
   std::string GetLanguage() const;

   // audio
   bool HasAudioDescription() const;
   int GetAudioChannels() const;

   // subtitle
   bool IsHearingImpaired() const;
   std::string GetLabel() const;

   // video
   float GetAspectRatio() const;

   JsonObject ToJsonObject() const;

public:

   Component(
      int componentTag,
      int pid,
      std::string encoding,
      bool encrypted,
      bool active,
      bool defaultComponent,
      bool hidden,
      float aspectRatio
      );

   Component(
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
      );

   Component(
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
      );

private:

   // common attributes
   int m_componentType;
   int m_componentTag;
   int m_pid;
   std::string m_encoding;
   bool m_encrypted;
   bool m_active;
   bool m_defaultComponent;
   bool m_hidden;

   // audio/subtitle attributes
   std::string m_language;

   // audio-only attributes
   bool m_audioDescription;
   int m_audioChannels;

   // subtitle-only attributes
   bool m_hearingImpaired;
   std::string m_label;

   // video-only attributes
   float m_aspectRatio;
}; // class Component
} // namespace orb
