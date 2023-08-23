/**
 * ORB Software. Copyright (c) 2023 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#pragma once

#ifdef BBC_API_ENABLE

#include <string>
#include <vector>

namespace orb
{
class DisplayInfo
{
public:

    class VideoMode
    {
public:

        /**
         * Constructor.
         *
         * @param width       Width of the video content
         * @param height      Height of the video content
         * @param frameRate   Frame rate of the video content
         * @param colorimetry A space separated list of colorimetry strings
         */
        VideoMode(int width, int height, int frameRate, std::string colorimetry)
            : m_width(width)
            , m_height(height)
            , m_frameRate(frameRate)
            , m_colorimetry(colorimetry)
        {
        }

        /**
         * Destructor.
         */
        ~VideoMode()
        {
        }

        int GetWidth() const
        {
            return m_width;
        }

        int GetHeight() const
        {
            return m_height;
        }

        int GetFrameRate() const
        {
            return m_frameRate;
        }

        std::string GetColorimetry() const
        {
            return m_colorimetry;
        }

private:

        int m_width;
        int m_height;
        int m_frameRate;
        std::string m_colorimetry;
    }; // class VideoMode

    /**
     * Constructor.
     *
     * @param physicalWidth  The physical width of the display device
     * @param physicalHeight The physical height of the display device
     * @param videoModes     The supported video modes of the display device
     */
    DisplayInfo(int physicalWidth, int physicalHeight, std::vector<VideoMode> videoModes)
        : m_physicalWidth(physicalWidth)
        , m_physicalHeight(physicalHeight)
        , m_videoModes(videoModes)
    {
    }

    /**
     * Destructor.
     */
    ~DisplayInfo()
    {
        m_videoModes.clear();
    }

    int GetPhysicalWidth() const
    {
        return m_physicalWidth;
    }

    int GetPhysicalHeight() const
    {
        return m_physicalHeight;
    }

    std::vector<VideoMode> GetVideoModes() const
    {
        return m_videoModes;
    }

private:

    int m_physicalWidth;
    int m_physicalHeight;
    std::vector<VideoMode> m_videoModes;
}; // class DisplayInfo
} // namespace orb

#endif