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
 * XML parser for AIT and DSM-CC
 *
 * Note: This file is part of the platform-agnostic application manager library.
 */

#ifndef XML_PARSE_H
#define XML_PARSE_H

#include "ait.h"

namespace orb
{

class XmlParser {
public:
#if 0 // TODO(C++-ize)
    typedef struct
    {
        uint16_t event_id;
        uint16_t name_len;
        uint8_t *name_ptr;
    } S_XML_SEVENT;

    typedef struct
    {
        uint16_t component_tag;
        uint16_t event_count;
        S_XML_SEVENT *events;
    } S_XML_SE_OBJ;

    typedef struct
    {
        uint32_t se_total;
        S_XML_SE_OBJ *se_objs;
    } S_XML_DSMCC;
#endif

    /**
     * Parse Xml data as specified in TS 102 809 section 5.4
     * @param content pointer to Xml data
     * @param length length of Xml data
     * @return AIT table data in same format as generated from DVB broadcast data
     */
    static std::unique_ptr<Ait::S_AIT_TABLE> ParseAit(const char *content, uint32_t length);

#if 0
    /**
     * Parse Xml data as specified in TS 102 809 section 8.2
     * @param content pointer to Xml data
     * @param length length of Xml data
     * @return Dsmcc object data Stream event object data
     */
    static S_XML_DSMCC* ParseDsmcc(uint8_t *content, uint32_t length);

    /**
     * Clear/free table created by ParseDsmcc().
     * @param dsm_objs pointer to dsmcc obects
     */
    static void FreeDsmcc(S_XML_DSMCC *dsm_objs);
#endif
};

} // namespace orb

#endif  /* XML_PARSE_H */
