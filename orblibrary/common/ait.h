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
 *
 * AIT parsing
 *
 * Note: This file is part of the platform-agnostic application manager library.
 */

#ifndef AIT_PARSE_H
#define AIT_PARSE_H

#include "utils.h"
#include <cstdint>
#include <vector>
#include <string>

#define AIT_NUM_RECEIVED_SECTION_MASK_BYTES (256 / 8)


#if ORB_HBBTV_VERSION == 204
    #define HBBTV_VERSION_MAJOR 1
    #define HBBTV_VERSION_MINOR 7
    #define HBBTV_VERSION_MICRO 1
#elif ORB_HBBTV_VERSION == 203
    #define HBBTV_VERSION_MAJOR 1
    #define HBBTV_VERSION_MINOR 6
    #define HBBTV_VERSION_MICRO 1
#else
    #error Unsupported ORB_HBBTV_VERSION
#endif


namespace orb
{
class Ait
{
public:
    static constexpr uint8_t AIT_USAGE_TELETEXT = 0x01;

    static constexpr uint8_t AIT_MAX_NUM_PROTOCOLS = 2;
    static constexpr uint16_t AIT_PROTOCOL_OBJECT_CAROUSEL = 0x0001;
    static constexpr uint16_t AIT_PROTOCOL_HTTP = 0x0003;

    static constexpr uint8_t AIT_NOT_VISIBLE_ALL = 0x00;
    static constexpr uint8_t AIT_NOT_VISIBLE_USERS = 0x01;
    static constexpr uint8_t AIT_VISIBLE_ALL = 0x03;

    typedef enum
    {
        APP_TYP_MHEG5 = 0x0008,
        APP_TYP_HBBTV = 0x0010,
        APP_TYP_XML   = 0x8000,
    } E_AIT_APP_TYPE;

    typedef enum
    {
        XML_TYP_UNKNOWN  = 0x00,
        XML_TYP_OTHER    = 0x01,
        XML_TYP_DVB_HTML = 0x10,
        XML_TYP_DVB_J    = 0x11,
        XML_TYP_OPAPP    = 0x80,
    } E_AIT_XML_TYPE;

    typedef enum
    {
        APP_CTL_UNKNOWN   = 0x00,
        APP_CTL_AUTOSTART = 0x01,
        APP_CTL_PRESENT   = 0x02,
        APP_CTL_DESTROY   = 0x03,
        APP_CTL_KILL      = 0x04,
        APP_CTL_PREFETCH  = 0x05,
        APP_CTL_REMOTE    = 0x06,
        APP_CTL_DISABLED  = 0x07,
        APP_CTL_PB_AUTO   = 0x08
    } E_AIT_APP_CONTROL;

    typedef struct
    {
        uint32_t langCode;
        std::string name;
    } S_LANG_STRING;

    typedef struct
    {
        uint8_t numLangs;
        std::vector<S_LANG_STRING> names;
    } S_APP_NAME_DESC;

    typedef struct
    {
        Utils::S_DVB_TRIPLET dvb;
        uint8_t componentTag;
        bool remoteConnection;
    } S_OC_SELECTOR_BYTES;

    typedef struct
    {
        std::string baseUrl;
        std::vector<std::string> extensionUrls;
    } S_URL_SELECTOR_BYTES;

    typedef struct
    {
        uint16_t protocolId;
        uint8_t transportProtocolLabel;
        S_OC_SELECTOR_BYTES oc;
        S_URL_SELECTOR_BYTES url;
        bool failedToLoad;
    } S_TRANSPORT_PROTOCOL_DESC;

    typedef struct
    {
        uint16_t appProfile;
        uint8_t versionMajor;
        uint8_t versionMinor;
        uint8_t versionMicro;
    } S_APP_PROFILE;

    typedef struct
    {
        uint8_t visibility;
        uint8_t priority;
        uint8_t numLabels;
        std::vector<S_APP_PROFILE> appProfiles;
        std::vector<uint8_t> transportProtocolLabels;
        bool serviceBound;
    } S_APP_DESC;

    typedef struct
    {
        std::string scheme;
        std::string region;
        uint8_t value;
    } S_APP_PARENTAL_RATING;

    typedef struct
    {
        uint32_t orgId;
        uint16_t appId;
        uint8_t controlCode;
        uint8_t numTransports;
        S_TRANSPORT_PROTOCOL_DESC transportArray[AIT_MAX_NUM_PROTOCOLS];
        std::string location;
        S_APP_NAME_DESC appName;
        S_APP_DESC appDesc;
        uint8_t xmlType;
        uint32_t xmlVersion;  // unsignedInt31Bit per XSD definition for OpApp.
        uint8_t usageType;
        std::vector<std::string> boundaries;
        std::vector<S_APP_PARENTAL_RATING> parentalRatings;
        std::vector<uint16_t> graphicsConstraints;
        std::string scheme;
        std::string appUsage;
    } S_AIT_APP_DESC;

    typedef struct
    {
        uint8_t sectionData[AIT_NUM_RECEIVED_SECTION_MASK_BYTES];
        uint16_t appType;
        uint8_t version;
        uint8_t numApps;
        std::vector<S_AIT_APP_DESC> appArray;
        bool complete;
    } S_AIT_TABLE;

    /**
     * Get the last completed AIT table. This value may be invalidated by calling ProcessSection(),
     * consumers of this API should ensure serialization.
     * @return An AIT table or nullptr.
     */
    Ait::S_AIT_TABLE* Get();

    /**
     * Clear any partial or completed data. This should be called when the service is changed or the
     * AIT PID is changed.
     */
    void Clear();

    /**
     * Process the input AIT section and update the AIT returned by Get(). Prior values from Get()
     * may be invalidated by calling this method, consumers of this API should ensure serialization.
     * @param data AIT section data
     * @param nbytes size of AIT section data in bytes
     * @return true if the Get() value was changed (i.e. a table was completed or the service changed)
     */
    bool ProcessSection(const uint8_t *data, uint32_t nbytes);

    void ApplyAitTable(std::unique_ptr<Ait::S_AIT_TABLE> &aitTable);

    /**
     *
     * @param aitTable AIT table.
     * @param parentalControlAge PC age set in the device.
     * @param parentalControlRegion 2 letter ISO 3166 region code.
     * @param parentalControlRegion3 3 letter ISO 3166 region code.
     * @param isNetworkAvailable
     * @return App to auto start
     */
    static const S_AIT_APP_DESC* AutoStartApp(const S_AIT_TABLE *aitTable, int parentalControlAge,
        std::string &parentalControlRegion, std::string &parentalControlRegion3, const bool isNetworkAvailable);

    /**
     *
     * @param aitTable
     * @return
     */
    static const S_AIT_APP_DESC* TeletextApp(const S_AIT_TABLE *aitTable);

    /**
     *
     * @param aitTable
     * @param orgId
     * @param appId
     * @return
     */
    static S_AIT_APP_DESC* FindApp(S_AIT_TABLE *aitTable, uint32_t orgId, uint16_t appId);

    /**
     *
     * @param parsedAit
     * @return
     */
    static bool PrintInfo(const S_AIT_TABLE *parsedAit);

    /**
     *
     * @param appDescription
     * @return
     */
    static std::string ExtractBaseURL(const Ait::S_AIT_APP_DESC &appDescription,
        const Utils::S_DVB_TRIPLET currentService, const bool isNetworkAvailable);

    static uint16_t ExtractProtocolId(const Ait::S_AIT_APP_DESC &appDescription,
        const bool isNetworkAvailable);

    /**
     * Determine whether the application has a transport with a certain protocol.
     * @param appDescription The application description.
     * @param protocolId The protocol to check for.
     * @return True if the application has a transport with the protocol, false otherwise.
     */
    static bool AppHasTransport(const Ait::S_AIT_APP_DESC *appDescription, uint16_t protocolId);

    /**
     * Check whether App description contains a viable transport protocol
     * @param appDesc
     * @param isNetworkAvailable
     * @return true if there is a viable transport
     */
    static bool HasViableTransport(const S_AIT_APP_DESC *appDesc, const bool isNetworkAvailable);

    /**
     * Set that the protocol for this app failed to load.
     * @param appDescription The application description.
     * @param protocolId The protocol that failed to load.
     */
    static void AppSetTransportFailedToLoad(Ait::S_AIT_APP_DESC *appDescription, uint16_t
        protocolId);

    /**
     * Checks whether an App has parental restrictions.
     * @param parentalRatings List of parental ratings included in the AIT.
     * @param parentalControlAge PC age set in the device.
     * @param parentalControlRegion 2 letter ISO 3166 region code.
     * @param parentalControlRegion3 3 letter ISO 3166 region code.
     */
    static bool IsAgeRestricted(const std::vector<Ait::S_APP_PARENTAL_RATING> parentalRatings,
        int parentalControlAge, std::string &parentalControlRegion,
        std::string &parentalControlRegion3);

private:

    /**
     *
     * @param dataPtr
     * @param desc
     */
    static void ParseAppDesc(const uint8_t *dataPtr, S_APP_DESC *desc);

    /**
     *
     * @param dataPtr
     * @param appName
     */
    static void ParseAppNameDesc(const uint8_t *dataPtr, S_APP_NAME_DESC *appName);

    /**
     *
     * @param dataPtr
     * @param trns
     * @return true if the descriptor is a new valid one.
     */
    static bool ParseTransportProtocolDesc(const uint8_t *dataPtr,
        S_TRANSPORT_PROTOCOL_DESC *trns);

    /**
     *
     * @param dataPtr
     * @param str
     */
    static void ParseSimpleAppLocationDesc(const uint8_t *dataPtr, std::string &str);

    /**
     * Parses the Simple Application Boundary Descriptor and updates the boundary list.
     * @param dataPtr
     * @param boundaries
     * @param num_boundaries
     */
    static void ParseSimpleAppBoundaryDesc(const uint8_t *dataPtr, S_AIT_APP_DESC *appPtr);

    static void ParseParentalRatingDesc(const uint8_t *dataPtr, S_AIT_APP_DESC *appPtr);

    static void ParseGraphicsConstraints(const uint8_t *dataPtr, S_AIT_APP_DESC *appPtr);

    /**
     *
     * @param data
     * @param len
     * @param appPtr
     */
    static void ParseApplication(const uint8_t *data, uint16_t len, S_AIT_APP_DESC *appPtr);

    /**
     * Returns true if the specified section has already been received
     * @param ait AIT structure
     * @param sectionNumber Section number
     * @return true if the specified section has already been received
     */
    static bool SectionReceived(const S_AIT_TABLE *ait, uint8_t sectionNumber);

    /**
     * Marks the bit representing the specified section number and returns true if all the sections
     * have been received
     * @param ait AIT structure
     * @param sectionNumber Section number
     * @param lastSectionNumber Last section number
     * @return true if all the sections have been received, false otherwise
     */
    static bool MarkSectionReceived(S_AIT_TABLE *ait, uint8_t sectionNumber, uint8_t
        lastSectionNumber);

    /**
     * Parses a section of the AIT table and updates the table structure
     * @param dataPtr Pointer to the first section byte
     * @return true if the table structure has changed
     */
    bool ParseSection(const uint8_t *dataPtr);

    std::shared_ptr<S_AIT_TABLE> m_ait;
    std::shared_ptr<S_AIT_TABLE> m_aitCompleted;
};

} // namespace orb

#endif /* AIT_PARSE_H */
