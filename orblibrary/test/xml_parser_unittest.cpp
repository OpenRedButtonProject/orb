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
 * Unit tests for XmlParser::ParseAit - OpApp extensions support
 */

#include <string>
#include <memory>

#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/orb/orblibrary/common/xml_parser.h"
#include "third_party/orb/orblibrary/common/ait.h"

using namespace orb;

/**
 * Test fixture class for XmlParser unit tests
 * Provides common setup and helper methods for all test cases
 */
// TS 102796 Table 7: applicationDescriptor/type/OtherApp
// "Shall be application/vnd.hbbtv.xhtml+xml for HbbTV applications"
static constexpr const char* HBBTV_MIME_TYPE = "application/vnd.hbbtv.xhtml+xml";

class XmlParserTest : public ::testing::Test {
protected:
    void SetUp() override {
        m_xmlParser = std::make_unique<XmlParser>();

        // Base AIT XML template with placeholders for customization
        // Includes OtherApp type element by default (common to HbbTV apps and OpApps)
        // TS 102796 Table 7: "Shall be application/vnd.hbbtv.xhtml+xml for HbbTV applications"
        m_baseAitXml = R"(<?xml version="1.0" encoding="UTF-8"?>
<mhp:ServiceDiscovery xmlns:mhp="urn:dvb:mhp:2009">
  <mhp:ApplicationDiscovery DomainName="test.example.com">
    <mhp:ApplicationList>
      <mhp:Application>
        <mhp:appName Language="eng">Test App</mhp:appName>
        <mhp:applicationIdentifier>
          <mhp:orgId>12345</mhp:orgId>
          <mhp:appId>1</mhp:appId>
        </mhp:applicationIdentifier>
        <mhp:applicationDescriptor>
          <mhp:type>
            <mhp:OtherApp>${OTHER_APP_MIME_TYPE}</mhp:OtherApp>
          </mhp:type>
          <mhp:controlCode>AUTOSTART</mhp:controlCode>
          <mhp:visibility>VISIBLE_ALL</mhp:visibility>
          <mhp:serviceBound>false</mhp:serviceBound>
          <mhp:priority>1</mhp:priority>
          <mhp:version>${VERSION}</mhp:version>
        </mhp:applicationDescriptor>
        ${USAGE_DESCRIPTOR}
        ${TRANSPORT}
        <mhp:applicationLocation>index.html</mhp:applicationLocation>
      </mhp:Application>
    </mhp:ApplicationList>
  </mhp:ApplicationDiscovery>
</mhp:ServiceDiscovery>)";

        // Default transport (HTTP)
        m_defaultTransport = R"(<mhp:applicationTransport xsi:type="mhp:HTTPTransportType" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
          <mhp:URLBase>https://test.example.com/app/</mhp:URLBase>
        </mhp:applicationTransport>)";
    }

    void TearDown() override {
        m_xmlParser.reset();
    }

    std::unique_ptr<Ait::S_AIT_TABLE> parseAitXml(const std::string& xmlContent) {
        return m_xmlParser->ParseAit(xmlContent.c_str(), xmlContent.length());
    }

    /**
     * Build AIT XML from template with customizable parts.
     * Template includes OtherApp type element by default with HbbTV MIME type.
     */
    std::string buildAitXml(
        const std::string& version = "1",
        const std::string& usageDescriptor = "",
        const std::string& otherAppMimeType = HBBTV_MIME_TYPE,
        const std::string& transport = "") {

        std::string xml = m_baseAitXml;

        // Replace placeholders
        replaceAll(xml, "${VERSION}", version);
        replaceAll(xml, "${USAGE_DESCRIPTOR}", usageDescriptor);
        replaceAll(xml, "${OTHER_APP_MIME_TYPE}", otherAppMimeType);
        replaceAll(xml, "${TRANSPORT}", transport.empty() ? m_defaultTransport : transport);

        return xml;
    }

    /**
     * Build applicationUsageDescriptor element
     */
    static std::string buildUsageDescriptor(const std::string& usage) {
        return R"(<mhp:applicationUsageDescriptor>
          <mhp:ApplicationUsage>)" + usage + R"(</mhp:ApplicationUsage>
        </mhp:applicationUsageDescriptor>)";
    }

    /**
     * Build AIT XML without OtherApp type element (for testing missing type)
     */
    std::string buildAitXmlWithoutType(const std::string& version = "1") {
        return R"(<?xml version="1.0" encoding="UTF-8"?>
<mhp:ServiceDiscovery xmlns:mhp="urn:dvb:mhp:2009">
  <mhp:ApplicationDiscovery DomainName="test.example.com">
    <mhp:ApplicationList>
      <mhp:Application>
        <mhp:appName Language="eng">Test App</mhp:appName>
        <mhp:applicationIdentifier>
          <mhp:orgId>12345</mhp:orgId>
          <mhp:appId>1</mhp:appId>
        </mhp:applicationIdentifier>
        <mhp:applicationDescriptor>
          <mhp:controlCode>AUTOSTART</mhp:controlCode>
          <mhp:visibility>VISIBLE_ALL</mhp:visibility>
          <mhp:serviceBound>false</mhp:serviceBound>
          <mhp:priority>1</mhp:priority>
          <mhp:version>)" + version + R"(</mhp:version>
        </mhp:applicationDescriptor>
        <mhp:applicationTransport xsi:type="mhp:HTTPTransportType" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
          <mhp:URLBase>https://test.example.com/app/</mhp:URLBase>
        </mhp:applicationTransport>
        <mhp:applicationLocation>index.html</mhp:applicationLocation>
      </mhp:Application>
    </mhp:ApplicationList>
  </mhp:ApplicationDiscovery>
</mhp:ServiceDiscovery>)";
    }

    /**
     * Build HTTP transport element
     */
    static std::string buildHttpTransport(const std::string& baseUrl,
        const std::vector<std::string>& extensions = {}) {

        std::string transport = R"(<mhp:applicationTransport xsi:type="mhp:HTTPTransportType" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
          <mhp:URLBase>)" + baseUrl + R"(</mhp:URLBase>)";

        for (const auto& ext : extensions) {
            transport += "\n          <mhp:URLExtension>" + ext + "</mhp:URLExtension>";
        }

        transport += "\n        </mhp:applicationTransport>";
        return transport;
    }

    /**
     * Build OC transport element
     */
    static std::string buildOcTransport(int origNetId, int tsId, int serviceId,
        const std::string& componentTag) {

        return R"(<mhp:applicationTransport xsi:type="mhp:OCTransportType" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
          <mhp:DvbTriplet OrigNetId=")" + std::to_string(origNetId) +
            R"(" TSId=")" + std::to_string(tsId) +
            R"(" ServiceId=")" + std::to_string(serviceId) + R"("/>
          <mhp:ComponentTag ComponentTag=")" + componentTag + R"("/>
        </mhp:applicationTransport>)";
    }

private:
    static void replaceAll(std::string& str, const std::string& from, const std::string& to) {
        size_t pos = 0;
        while ((pos = str.find(from, pos)) != std::string::npos) {
            str.replace(pos, from.length(), to);
            pos += to.length();
        }
    }

protected:
    std::unique_ptr<XmlParser> m_xmlParser;
    std::string m_baseAitXml;
    std::string m_defaultTransport;
};

// =============================================================================
// applicationUsageDescriptor/ApplicationUsage Tests
// =============================================================================

TEST_F(XmlParserTest, ParseAit_ApplicationUsageDescriptor_ParsesApplicationUsage)
{
    // GIVEN: An AIT XML with applicationUsageDescriptor containing ApplicationUsage
    std::string aitXml = buildAitXml("1", buildUsageDescriptor("urn:dvb:opapp:usage:epg"));

    // WHEN: Parsing the AIT XML
    auto aitTable = parseAitXml(aitXml);

    // THEN: The applicationUsage field should be parsed correctly
    ASSERT_NE(aitTable, nullptr);
    ASSERT_EQ(aitTable->numApps, 1);
    EXPECT_EQ(aitTable->appArray[0].appUsage, "urn:dvb:opapp:usage:epg");
}

TEST_F(XmlParserTest, ParseAit_ApplicationUsageDescriptor_EmptyUsage)
{
    // GIVEN: An AIT XML with empty ApplicationUsage
    std::string aitXml = buildAitXml("1", buildUsageDescriptor(""));

    // WHEN: Parsing the AIT XML
    auto aitTable = parseAitXml(aitXml);

    // THEN: The applicationUsage field should be empty
    ASSERT_NE(aitTable, nullptr);
    ASSERT_EQ(aitTable->numApps, 1);
    EXPECT_TRUE(aitTable->appArray[0].appUsage.empty());
}

TEST_F(XmlParserTest, ParseAit_NoApplicationUsageDescriptor)
{
    // GIVEN: An AIT XML without applicationUsageDescriptor
    std::string aitXml = buildAitXml();

    // WHEN: Parsing the AIT XML
    auto aitTable = parseAitXml(aitXml);

    // THEN: The applicationUsage field should be empty (default)
    ASSERT_NE(aitTable, nullptr);
    ASSERT_EQ(aitTable->numApps, 1);
    EXPECT_TRUE(aitTable->appArray[0].appUsage.empty());
}

// =============================================================================
// applicationDescriptor/version Tests
// XSD definition: <xsd:element name="version" type="mhp:unsignedInt31Bit"/>
// Note: XSD allows values 0 to 2^31-1, but current implementation stores as uint8_t
// =============================================================================

TEST_F(XmlParserTest, ParseAit_ApplicationDescriptor_ParsesVersion)
{
    // GIVEN: An AIT XML with applicationDescriptor containing version 42
    std::string aitXml = buildAitXml("42");

    // WHEN: Parsing the AIT XML
    auto aitTable = parseAitXml(aitXml);

    // THEN: The xmlVersion field should be parsed correctly
    ASSERT_NE(aitTable, nullptr);
    ASSERT_EQ(aitTable->numApps, 1);
    EXPECT_EQ(aitTable->appArray[0].xmlVersion, 42);
}

TEST_F(XmlParserTest, ParseAit_ApplicationDescriptor_VersionZero)
{
    // GIVEN: An AIT XML with version set to 0
    std::string aitXml = buildAitXml("0");

    // WHEN: Parsing the AIT XML
    auto aitTable = parseAitXml(aitXml);

    // THEN: The xmlVersion field should be 0
    ASSERT_NE(aitTable, nullptr);
    ASSERT_EQ(aitTable->numApps, 1);
    EXPECT_EQ(aitTable->appArray[0].xmlVersion, 0);
}

TEST_F(XmlParserTest, ParseAit_ApplicationDescriptor_VersionLargeValue)
{
    // GIVEN: An AIT XML with version set to a large value within unsignedInt31Bit range
    // XSD: unsignedInt31Bit allows values 0 to 2147483647
    std::string aitXml = buildAitXml("123456789");

    // WHEN: Parsing the AIT XML
    auto aitTable = parseAitXml(aitXml);

    // THEN: The xmlVersion field should parse the value
    // Note: Current implementation stores as uint8_t, so only lower 8 bits are preserved
    ASSERT_NE(aitTable, nullptr);
    ASSERT_EQ(aitTable->numApps, 1);
    // 123456789 & 0xFF = 21 (truncation to uint8_t)
    EXPECT_EQ(aitTable->appArray[0].xmlVersion, static_cast<uint8_t>(123456789));
}

TEST_F(XmlParserTest, ParseAit_ApplicationDescriptor_VersionMaxUnsignedInt31Bit)
{
    // GIVEN: An AIT XML with version set to max unsignedInt31Bit value (2^31 - 1)
    std::string aitXml = buildAitXml("2147483647");

    // WHEN: Parsing the AIT XML
    auto aitTable = parseAitXml(aitXml);

    // THEN: The value should be parsed (truncated to uint8_t in current implementation)
    ASSERT_NE(aitTable, nullptr);
    ASSERT_EQ(aitTable->numApps, 1);
    // 2147483647 & 0xFF = 255
    EXPECT_EQ(aitTable->appArray[0].xmlVersion, static_cast<uint8_t>(2147483647));
}

// =============================================================================
// applicationDescriptor/type/OtherApp Tests
// =============================================================================

TEST_F(XmlParserTest, ParseAit_ApplicationDescriptorType_OtherApp_HbbTV)
{
    // GIVEN: An AIT XML with OtherApp type set to HbbTV MIME type
    // TS 102796 Table 7: "Shall be application/vnd.hbbtv.xhtml+xml for HbbTV applications"
    std::string aitXml = buildAitXml();  // Uses default HbbTV MIME type

    // WHEN: Parsing the AIT XML
    auto aitTable = parseAitXml(aitXml);

    // THEN: The xmlType field should be XML_TYP_OTHER for HbbTV app
    ASSERT_NE(aitTable, nullptr);
    ASSERT_EQ(aitTable->numApps, 1);
    EXPECT_EQ(aitTable->appArray[0].xmlType, Ait::XML_TYP_OTHER);
}

TEST_F(XmlParserTest, ParseAit_ApplicationDescriptorType_OtherApp_Unknown)
{
    // GIVEN: An AIT XML with OtherApp type set to an unknown MIME type
    std::string aitXml = buildAitXml("1", "", "application/unknown-type");

    // WHEN: Parsing the AIT XML
    auto aitTable = parseAitXml(aitXml);

    // THEN: The xmlType should remain at default (XML_TYP_UNKNOWN) for unknown MIME type
    ASSERT_NE(aitTable, nullptr);
    ASSERT_EQ(aitTable->numApps, 1);
    EXPECT_EQ(aitTable->appArray[0].xmlType, Ait::XML_TYP_UNKNOWN);
}

TEST_F(XmlParserTest, ParseAit_ApplicationDescriptorType_NoOtherApp)
{
    // GIVEN: An AIT XML without type/OtherApp element
    std::string aitXml = buildAitXmlWithoutType();

    // WHEN: Parsing the AIT XML
    auto aitTable = parseAitXml(aitXml);

    // THEN: The xmlType should be XML_TYP_UNKNOWN (default)
    ASSERT_NE(aitTable, nullptr);
    ASSERT_EQ(aitTable->numApps, 1);
    EXPECT_EQ(aitTable->appArray[0].xmlType, Ait::XML_TYP_UNKNOWN);
}

// =============================================================================
// applicationTransport Tests
// =============================================================================

TEST_F(XmlParserTest, ParseAit_ApplicationTransport_HTTPTransportType)
{
    // GIVEN: An AIT XML with HTTP transport type
    std::string aitXml = buildAitXml("1", "", HBBTV_MIME_TYPE,
        buildHttpTransport("https://test.example.com/app/"));

    // WHEN: Parsing the AIT XML
    auto aitTable = parseAitXml(aitXml);

    // THEN: The HTTP transport should be parsed correctly
    ASSERT_NE(aitTable, nullptr);
    ASSERT_EQ(aitTable->numApps, 1);
    EXPECT_EQ(aitTable->appArray[0].numTransports, 1);
    EXPECT_EQ(aitTable->appArray[0].transportArray[0].protocolId, AIT_PROTOCOL_HTTP);
    EXPECT_EQ(aitTable->appArray[0].transportArray[0].url.baseUrl, "https://test.example.com/app/");
    EXPECT_FALSE(aitTable->appArray[0].transportArray[0].failedToLoad);
}

TEST_F(XmlParserTest, ParseAit_ApplicationTransport_HTTPWithURLExtensions)
{
    // GIVEN: An AIT XML with HTTP transport type containing URL extensions
    std::string aitXml = buildAitXml("1", "", HBBTV_MIME_TYPE,
        buildHttpTransport("https://test.example.com/", {"app/v1/", "app/v2/"}));

    // WHEN: Parsing the AIT XML
    auto aitTable = parseAitXml(aitXml);

    // THEN: The HTTP transport with URL extensions should be parsed correctly
    ASSERT_NE(aitTable, nullptr);
    ASSERT_EQ(aitTable->numApps, 1);
    EXPECT_EQ(aitTable->appArray[0].transportArray[0].protocolId, AIT_PROTOCOL_HTTP);
    EXPECT_EQ(aitTable->appArray[0].transportArray[0].url.baseUrl, "https://test.example.com/");
    ASSERT_EQ(aitTable->appArray[0].transportArray[0].url.extensionUrls.size(), size_t(2));
    EXPECT_EQ(aitTable->appArray[0].transportArray[0].url.extensionUrls[0], "app/v1/");
    EXPECT_EQ(aitTable->appArray[0].transportArray[0].url.extensionUrls[1], "app/v2/");
}

TEST_F(XmlParserTest, ParseAit_ApplicationTransport_OCTransportType)
{
    // GIVEN: An AIT XML with Object Carousel transport type
    std::string aitXml = buildAitXml("1", "", HBBTV_MIME_TYPE,
        buildOcTransport(1, 2, 3, "0A"));

    // WHEN: Parsing the AIT XML
    auto aitTable = parseAitXml(aitXml);

    // THEN: The OC transport should be parsed correctly
    ASSERT_NE(aitTable, nullptr);
    ASSERT_EQ(aitTable->numApps, 1);
    EXPECT_EQ(aitTable->appArray[0].numTransports, 1);
    EXPECT_EQ(aitTable->appArray[0].transportArray[0].protocolId, AIT_PROTOCOL_OBJECT_CAROUSEL);
    EXPECT_EQ(aitTable->appArray[0].transportArray[0].oc.dvb.originalNetworkId, 1);
    EXPECT_EQ(aitTable->appArray[0].transportArray[0].oc.dvb.transportStreamId, 2);
    EXPECT_EQ(aitTable->appArray[0].transportArray[0].oc.dvb.serviceId, 3);
    EXPECT_EQ(aitTable->appArray[0].transportArray[0].oc.componentTag, 0x0A);
    EXPECT_TRUE(aitTable->appArray[0].transportArray[0].oc.remoteConnection);
}

TEST_F(XmlParserTest, ParseAit_ApplicationTransport_MultipleTransports)
{
    // GIVEN: An AIT XML with both HTTP and OC transport types
    std::string combinedTransport =
        buildHttpTransport("https://test.example.com/app/") + "\n        " +
        buildOcTransport(100, 200, 300, "1F");
    std::string aitXml = buildAitXml("1", "", HBBTV_MIME_TYPE, combinedTransport);

    // WHEN: Parsing the AIT XML
    auto aitTable = parseAitXml(aitXml);

    // THEN: Both transports should be parsed correctly
    ASSERT_NE(aitTable, nullptr);
    ASSERT_EQ(aitTable->numApps, 1);
    EXPECT_EQ(aitTable->appArray[0].numTransports, 2);

    // Find HTTP transport
    bool foundHttp = false;
    bool foundOC = false;
    for (int i = 0; i < aitTable->appArray[0].numTransports; i++) {
        if (aitTable->appArray[0].transportArray[i].protocolId == AIT_PROTOCOL_HTTP) {
            foundHttp = true;
            EXPECT_EQ(aitTable->appArray[0].transportArray[i].url.baseUrl, "https://test.example.com/app/");
        }
        if (aitTable->appArray[0].transportArray[i].protocolId == AIT_PROTOCOL_OBJECT_CAROUSEL) {
            foundOC = true;
            EXPECT_EQ(aitTable->appArray[0].transportArray[i].oc.dvb.originalNetworkId, 100);
            EXPECT_EQ(aitTable->appArray[0].transportArray[i].oc.dvb.transportStreamId, 200);
            EXPECT_EQ(aitTable->appArray[0].transportArray[i].oc.dvb.serviceId, 300);
            EXPECT_EQ(aitTable->appArray[0].transportArray[i].oc.componentTag, 0x1F);
        }
    }
    EXPECT_TRUE(foundHttp);
    EXPECT_TRUE(foundOC);
}

TEST_F(XmlParserTest, ParseAit_ApplicationTransport_NoTransport)
{
    // GIVEN: An AIT XML without applicationTransport
    // Note: Pass " " (space) not "" to avoid using default transport
    std::string aitXml = buildAitXml("1", "", HBBTV_MIME_TYPE, " ");

    // WHEN: Parsing the AIT XML
    auto aitTable = parseAitXml(aitXml);

    // THEN: numTransports should be 0
    ASSERT_NE(aitTable, nullptr);
    ASSERT_EQ(aitTable->numApps, 1);
    EXPECT_EQ(aitTable->appArray[0].numTransports, 0);
}

// =============================================================================
// Combined OpApp Extensions Tests
// =============================================================================

TEST_F(XmlParserTest, ParseAit_OpAppExtensions_AllFieldsPresent)
{
    // GIVEN: An AIT XML with all opapp extension fields
    // Using version 1000017 to test unsignedInt31Bit range (XSD spec)
    std::string aitXml = buildAitXml(
        "1000017",  // version (unsignedInt31Bit per XSD)
        buildUsageDescriptor("urn:dvb:opapp:usage:launcher"),
        HBBTV_MIME_TYPE,  // TS 102796 Table 7
        buildHttpTransport("https://opapp.example.com/launcher/", {"v2/"})
    );

    // WHEN: Parsing the AIT XML
    auto aitTable = parseAitXml(aitXml);

    // THEN: All opapp extension fields should be parsed correctly
    ASSERT_NE(aitTable, nullptr);
    ASSERT_EQ(aitTable->numApps, 1);

    const auto& app = aitTable->appArray[0];

    // Verify applicationUsageDescriptor/ApplicationUsage
    EXPECT_EQ(app.appUsage, "urn:dvb:opapp:usage:launcher");

    // Verify applicationDescriptor/version (stored as uint8_t, so truncated)
    EXPECT_EQ(app.xmlVersion, static_cast<uint8_t>(1000017));

    // Verify applicationDescriptor/type/OtherApp
    EXPECT_EQ(app.xmlType, Ait::XML_TYP_OTHER);

    // Verify applicationTransport
    EXPECT_EQ(app.numTransports, 1);
    EXPECT_EQ(app.transportArray[0].protocolId, AIT_PROTOCOL_HTTP);
    EXPECT_EQ(app.transportArray[0].url.baseUrl, "https://opapp.example.com/launcher/");
    ASSERT_EQ(app.transportArray[0].url.extensionUrls.size(), size_t(1));
    EXPECT_EQ(app.transportArray[0].url.extensionUrls[0], "v2/");
}

TEST_F(XmlParserTest, ParseAit_OpAppExtensions_MultipleApplications)
{
    // GIVEN: An AIT XML with multiple applications, each with different opapp extensions
    std::string aitXml = R"(<?xml version="1.0" encoding="UTF-8"?>
<mhp:ServiceDiscovery xmlns:mhp="urn:dvb:mhp:2009">
  <mhp:ApplicationDiscovery DomainName="opapp.example.com">
    <mhp:ApplicationList>
      <mhp:Application>
        <mhp:appName Language="eng">EPG App</mhp:appName>
        <mhp:applicationIdentifier>
          <mhp:orgId>11111</mhp:orgId>
          <mhp:appId>1</mhp:appId>
        </mhp:applicationIdentifier>
        <mhp:applicationDescriptor>
          <mhp:type>
            <mhp:OtherApp>application/vnd.hbbtv.xhtml+xml</mhp:OtherApp>
          </mhp:type>
          <mhp:controlCode>PRESENT</mhp:controlCode>
          <mhp:visibility>VISIBLE_ALL</mhp:visibility>
          <mhp:serviceBound>false</mhp:serviceBound>
          <mhp:priority>1</mhp:priority>
          <mhp:version>10</mhp:version>
        </mhp:applicationDescriptor>
        <mhp:applicationUsageDescriptor>
          <mhp:ApplicationUsage>urn:dvb:opapp:usage:epg</mhp:ApplicationUsage>
        </mhp:applicationUsageDescriptor>
        <mhp:applicationTransport xsi:type="mhp:HTTPTransportType" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
          <mhp:URLBase>https://epg.example.com/</mhp:URLBase>
        </mhp:applicationTransport>
        <mhp:applicationLocation>epg.html</mhp:applicationLocation>
      </mhp:Application>
      <mhp:Application>
        <mhp:appName Language="eng">Launcher App</mhp:appName>
        <mhp:applicationIdentifier>
          <mhp:orgId>22222</mhp:orgId>
          <mhp:appId>2</mhp:appId>
        </mhp:applicationIdentifier>
        <mhp:applicationDescriptor>
          <mhp:type>
            <mhp:OtherApp>application/vnd.hbbtv.xhtml+xml</mhp:OtherApp>
          </mhp:type>
          <mhp:controlCode>AUTOSTART</mhp:controlCode>
          <mhp:visibility>VISIBLE_ALL</mhp:visibility>
          <mhp:serviceBound>false</mhp:serviceBound>
          <mhp:priority>2</mhp:priority>
          <mhp:version>20</mhp:version>
        </mhp:applicationDescriptor>
        <mhp:applicationUsageDescriptor>
          <mhp:ApplicationUsage>urn:dvb:opapp:usage:launcher</mhp:ApplicationUsage>
        </mhp:applicationUsageDescriptor>
        <mhp:applicationTransport xsi:type="mhp:HTTPTransportType" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
          <mhp:URLBase>https://launcher.example.com/</mhp:URLBase>
        </mhp:applicationTransport>
        <mhp:applicationLocation>launcher.html</mhp:applicationLocation>
      </mhp:Application>
    </mhp:ApplicationList>
  </mhp:ApplicationDiscovery>
</mhp:ServiceDiscovery>)";

    // WHEN: Parsing the AIT XML
    auto aitTable = parseAitXml(aitXml);

    // THEN: Both applications should be parsed with their respective opapp extensions
    ASSERT_NE(aitTable, nullptr);
    ASSERT_EQ(aitTable->numApps, 2);

    // Verify first app (EPG)
    EXPECT_EQ(aitTable->appArray[0].appUsage, "urn:dvb:opapp:usage:epg");
    EXPECT_EQ(aitTable->appArray[0].xmlVersion, 10);
    EXPECT_EQ(aitTable->appArray[0].xmlType, Ait::XML_TYP_OTHER);
    EXPECT_EQ(aitTable->appArray[0].transportArray[0].url.baseUrl, "https://epg.example.com/");

    // Verify second app (Launcher)
    EXPECT_EQ(aitTable->appArray[1].appUsage, "urn:dvb:opapp:usage:launcher");
    EXPECT_EQ(aitTable->appArray[1].xmlVersion, 20);
    EXPECT_EQ(aitTable->appArray[1].xmlType, Ait::XML_TYP_OTHER);
    EXPECT_EQ(aitTable->appArray[1].transportArray[0].url.baseUrl, "https://launcher.example.com/");
}

// =============================================================================
// Error Handling Tests
// =============================================================================

TEST_F(XmlParserTest, ParseAit_InvalidXml_ReturnsNullptr)
{
    // GIVEN: Invalid XML content
    std::string invalidXml = "This is not valid XML content";

    // WHEN: Parsing the invalid XML
    auto aitTable = parseAitXml(invalidXml);

    // THEN: Should return nullptr
    EXPECT_EQ(aitTable, nullptr);
}

TEST_F(XmlParserTest, ParseAit_EmptyXml_ReturnsNullptr)
{
    // GIVEN: Empty XML content
    std::string emptyXml = "";

    // WHEN: Parsing the empty XML
    auto aitTable = parseAitXml(emptyXml);

    // THEN: Should return nullptr
    EXPECT_EQ(aitTable, nullptr);
}

TEST_F(XmlParserTest, ParseAit_EmptyApplicationList_ReturnsZeroApps)
{
    // GIVEN: An AIT XML with empty ApplicationList
    std::string aitXml = R"(<?xml version="1.0" encoding="UTF-8"?>
<mhp:ServiceDiscovery xmlns:mhp="urn:dvb:mhp:2009">
  <mhp:ApplicationDiscovery DomainName="test.example.com">
    <mhp:ApplicationList>
    </mhp:ApplicationList>
  </mhp:ApplicationDiscovery>
</mhp:ServiceDiscovery>)";

    // WHEN: Parsing the AIT XML
    auto aitTable = parseAitXml(aitXml);

    // THEN: Should return table with zero apps
    ASSERT_NE(aitTable, nullptr);
    EXPECT_EQ(aitTable->numApps, 0);
}
