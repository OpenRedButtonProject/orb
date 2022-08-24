/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
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

#define AIT_USAGE_TELETEXT 0x01

#define AIT_MAX_NUM_PROTOCOLS 2
#define AIT_PROTOCOL_OBJECT_CAROUSEL 0x0001
#define AIT_PROTOCOL_HTTP 0x0003

#define AIT_NOT_VISIBLE_ALL 0x00
#define AIT_NOT_VISIBLE_USERS 0x01
#define AIT_VISIBLE_ALL 0x03

#define AIT_NUM_RECEIVED_SECTION_MASK_BYTES (256 / 8)

#define HBBTV_VERSION_MAJOR 1
#define HBBTV_VERSION_MINOR 6
#define HBBTV_VERSION_MICRO 1

class Ait
{
public:
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
      uint32_t lang_code;
      std::string name;
   } S_LANG_STRING;

   typedef struct
   {
      uint8_t num_langs;
      std::vector<S_LANG_STRING> names;
   } S_APP_NAME_DESC;

   typedef struct
   {
      Utils::S_DVB_TRIPLET dvb;
      uint8_t component_tag;
      bool remote_connection;
   } S_OC_SELECTOR_BYTES;

   typedef struct
   {
      std::string base_url;
      std::vector<std::string> extension_urls;
   } S_URL_SELECTOR_BYTES;

   typedef struct
   {
      uint16_t protocol_id;
      uint8_t transport_protocol_label;
      S_OC_SELECTOR_BYTES oc;
      S_URL_SELECTOR_BYTES url;
      bool failed_to_load;
   } S_TRANSPORT_PROTOCOL_DESC;

   typedef struct
   {
      uint16_t app_profile;
      uint8_t version_major;
      uint8_t version_minor;
      uint8_t version_micro;
   } S_APP_PROFILE;

   typedef struct
   {
      uint8_t visibility;
      uint8_t priority;
      uint8_t num_labels;
      std::vector<S_APP_PROFILE> app_profiles;
      std::vector<uint8_t> transport_protocol_labels;
      bool service_bound;
   } S_APP_DESC;

   typedef struct
   {
      std::string scheme;
      std::string region;
      uint8_t value;
   } S_APP_PARENTAL_RATING;

   typedef struct
   {
      uint32_t org_id;
      uint16_t app_id;
      uint8_t control_code;
      uint8_t num_transports;
      S_TRANSPORT_PROTOCOL_DESC transport_array[AIT_MAX_NUM_PROTOCOLS];
      std::string location;
      S_APP_NAME_DESC app_name;
      S_APP_DESC app_desc;
      uint8_t xml_type;
      uint8_t xml_version;
      uint8_t usage_type;
      std::vector<std::string> boundaries;
      std::vector<S_APP_PARENTAL_RATING> parental_ratings;
   } S_AIT_APP_DESC;

   typedef struct
   {
      uint8_t section_data[AIT_NUM_RECEIVED_SECTION_MASK_BYTES];
      uint16_t app_type;
      uint8_t version;
      uint8_t num_apps;
      std::vector<S_AIT_APP_DESC> app_array;
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

   /**
    *
    * @param ait_table AIT table.
    * @param parental_control_age PC age set in the device.
    * @param parental_control_region 2 letter ISO 3166 region code.
    * @param parental_control_region3 3 letter ISO 3166 region code.
    * @return App to auto start
    */
   static const S_AIT_APP_DESC* AutoStartApp(const S_AIT_TABLE *ait_table, int parental_control_age,
      std::string &parental_control_region, std::string &parental_control_region3);

   /**
    *
    * @param ait_table
    * @return
    */
   static const S_AIT_APP_DESC* TeletextApp(const S_AIT_TABLE *ait_table);

   /**
    *
    * @param ait_table
    * @param org_id
    * @param app_id
    * @return
    */
   static S_AIT_APP_DESC* FindApp(S_AIT_TABLE *ait_table, uint32_t org_id, uint16_t app_id);

   /**
    *
    * @param parsed_ait
    * @return
    */
   static bool PrintInfo(const S_AIT_TABLE *parsed_ait);

   /**
    *
    * @param app_description
    * @return
    */
   static std::string GetBaseURL(const Ait::S_AIT_APP_DESC *app_description,
      const Utils::S_DVB_TRIPLET current_service, const bool is_network_available,
      uint16_t *protocol_id_selected);

/**
 * Determine whether the application has a transport with a certain protocol.
 * @param app_description The application description.
 * @param protocol_id The protocol to check for.
 * @return True if the application has a transport with the protocol, false otherwise.
 */
   static bool AppHasTransport(const Ait::S_AIT_APP_DESC *app_description, uint16_t protocol_id);

   /**
    * Set that the protocol for this app failed to load.
    * @param app_description The application description.
    * @param protocol_id The protocol that failed to load.
    */
   static void AppSetTransportFailedToLoad(Ait::S_AIT_APP_DESC *app_description, uint16_t protocol_id);

   /**
    * Checks whether an App has parental restrictions.
    * @param parental_ratings List of parental ratings included in the AIT.
    * @param parental_control_age PC age set in the device.
    * @param parental_control_region 2 letter ISO 3166 region code.
    * @param parental_control_region3 3 letter ISO 3166 region code.
    */
   static bool IsAgeRestricted(const std::vector<Ait::S_APP_PARENTAL_RATING> parental_ratings, int parental_control_age,
      std::string &parental_control_region, std::string &parental_control_region3);

private:

   /**
    *
    * @param data_ptr
    * @param desc
    */
   static void ParseAppDesc(const uint8_t *data_ptr, S_APP_DESC *desc);

   /**
    *
    * @param data_ptr
    * @param app_name
    */
   static void ParseAppNameDesc(const uint8_t *data_ptr, S_APP_NAME_DESC *app_name);

   /**
    *
    * @param data_ptr
    * @param trns
    * @return true if the descriptor is a new valid one.
    */
   static bool ParseTransportProtocolDesc(const uint8_t *data_ptr, S_TRANSPORT_PROTOCOL_DESC *trns);

   /**
    *
    * @param data_ptr
    * @param str
    */
   static void ParseSimpleAppLocationDesc(const uint8_t *data_ptr, std::string &str);

   /**
    * Parses the Simple Application Boundary Descriptor and updates the boundary list.
    * @param data_ptr
    * @param boundaries
    * @param num_boundaries
    */
   static void ParseSimpleAppBoundaryDesc(const uint8_t *data_ptr, S_AIT_APP_DESC *app_ptr);

   static void ParseParentalRatingDesc(const uint8_t *data_ptr, S_AIT_APP_DESC *app_ptr);

   /**
    *
    * @param data
    * @param len
    * @param app_ptr
    */
   static void ParseApplication(const uint8_t *data, uint16_t len, S_AIT_APP_DESC *app_ptr);

   /**
    * Returns true if the specified section has already been received
    * @param ait AIT structure
    * @param section_number Section number
    * @return true if the specified section has already been received
    */
   static bool SectionReceived(const S_AIT_TABLE *ait, uint8_t section_number);

   /**
    * Marks the bit representing the specified section number and returns true if all the sections
    * have been received
    * @param ait AIT structure
    * @param section_number Section number
    * @param last_section_number Last section number
    * @return true if all the sections have been received, false otherwise
    */
   static bool MarkSectionReceived(S_AIT_TABLE *ait, uint8_t section_number, uint8_t last_section_number);

   /**
    * Parses a section of the AIT table and updates the table structure
    * @param data_ptr Pointer to the first section byte
    * @return true if the table structure has changed
    */
   bool ParseSection(const uint8_t *data_ptr);

   std::shared_ptr<S_AIT_TABLE> ait_;
   std::shared_ptr<S_AIT_TABLE> ait_completed_;
};

#endif /* AIT_PARSE_H */
