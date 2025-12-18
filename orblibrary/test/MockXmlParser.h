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
 * Mock XML Parser for unit testing
 */

#ifndef MOCK_XML_PARSER_H
#define MOCK_XML_PARSER_H

#include "testing/gmock/include/gmock/gmock.h"
#include "xml_parser.h"

namespace orb
{

/**
 * Mock implementation of IXmlParser interface for unit testing
 */
class MockXmlParser : public IXmlParser
{
public:
    /**
     * Mock implementation of ParseAit method
     * @param content pointer to Xml data
     * @param length length of Xml data
     * @return AIT table data in same format as generated from DVB broadcast data
     */
    MOCK_METHOD(std::unique_ptr<Ait::S_AIT_TABLE>, ParseAit, (const char *content, uint32_t length), (override));
};

} // namespace orb

#endif /* MOCK_XML_PARSER_H */
