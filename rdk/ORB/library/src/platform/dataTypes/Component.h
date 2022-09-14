/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#pragma once

#include <string>
#include <memory>

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
 * (See OIPF DAE spec section 7.16.5.2)
 */
class Component
{
public:

    /**
     * Create a video component.
     *
     * @param componentTag     The component tag identifies a component
     * @param pid              The MPEG Program ID (PID) of the component in the MPEG2-TS in which
     *                         it is carried
     * @param encoding         The encoding of the stream
     * @param encrypted        Flag indicating whether the component is encrypted or not
     * @param active
     * @param defaultComponent
     * @param hidden
     * @param aspectRatio      Indicates the aspect ratio of the video
     *
     * @return Pointer to the new video component
     */
    static std::shared_ptr<Component> CreateVideoComponent(
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
     * @param componentTag     The component tag identifies a component
     * @param pid              The MPEG Program ID (PID) of the component in the MPEG2-TS in which
     *                         it is carried
     * @param encoding         The encoding of the stream
     * @param encrypted        Flag indicating whether the component is encrypted or not
     * @param active
     * @param defaultComponent
     * @param hidden
     * @param language         An ISO 639-2 language code representing the language of the stream
     * @param audioDescription Has value true if the stream contains an audio description intended
     *                         for people with a visual impairment, false otherwise
     * @param audioChannels    Indicates the number of channels present in this stream
     *
     * @return Pointer to the new audio component
     */
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
     * @param componentTag     The component tag identifies a component
     * @param pid              The MPEG Program ID (PID) of the component in the MPEG2-TS in which
     *                         it is carried
     * @param encoding         The encoding of the stream
     * @param encrypted        Flag indicating whether the component is encrypted or not
     * @param active
     * @param defaultComponent
     * @param hidden
     * @param language         An ISO 639-2 language code representing the language of the stream
     * @param hearingImpaired  Has value true if the stream is intended for the hearing-impaired
     *                         (e.g. contains a written description of the sound effects), false
     *                         otherwise
     * @param label
     *
     * @return Pointer to the new subtitle component
     */
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
    Component(
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
    ~Component()
    {
    }

    int GetComponentType() const
    {
        return m_componentType;
    }

    int GetComponentTag() const
    {
        return m_componentTag;
    }

    int GetPid() const
    {
        return m_pid;
    }

    std::string GetEncoding() const
    {
        return m_encoding;
    }

    bool IsEncrypted() const
    {
        return m_encrypted;
    }

    bool IsActive() const
    {
        return m_active;
    }

    bool IsDefaultComponent() const
    {
        return m_defaultComponent;
    }

    bool IsHidden() const
    {
        return m_hidden;
    }

    std::string GetLanguage() const
    {
        return m_language;
    }

    bool HasAudioDescription() const
    {
        return m_audioDescription;
    }

    int GetAudioChannels() const
    {
        return m_audioChannels;
    }

    bool IsHearingImpaired() const
    {
        return m_hearingImpaired;
    }

    std::string GetLabel() const
    {
        return m_label;
    }

    float GetAspectRatio() const
    {
        return m_aspectRatio;
    }

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