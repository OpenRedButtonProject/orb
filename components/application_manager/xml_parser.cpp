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

#include "xml_parser.h"

#include <cstring>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <string>

#include "log.h"

/* Defining USE_XML_API_FUNC means using XML API functions to retrieve data
 * Advantage is that it avoids direct access of xml structure.
 * but disadvantage is that there is additional memory allocation, data copying,
 * and memory freeing.
 * Note: Not defining USE_XML_API_FUNC give more efficient code, but could be
 * susceptible to Xml library header changes.
 */
// #define USE_XML_API_FUNC
#ifdef USE_XML_API_FUNC
#define NODE_CONTENT_GET(node, dptr) xmlChar *pNodeCtnt = xmlNodeGetContent(node); dptr = pNodeCtnt
#define NODE_CONTENT_RELEASE() xmlFree(pNodeCtnt)
#define NODE_PROPERTY(node, prop) xmlGetProp(node, prop)
#define NODE_PROP_FREE(dptr) xmlFree(dptr);
#else
#define NODE_CONTENT_GET(node, dptr) dptr = (node->children) ? node->children->content : nullptr
#define NODE_CONTENT_RELEASE()
#define NODE_PROPERTY(node, prop) XmlGetPropertyContent(node, prop)
#define NODE_PROP_FREE(dptr)
#endif

/**
 *
 * @param dptr
 * @return
 */
static uint32_t XmlParseInt(xmlChar *dptr)
{
    uint32_t num = 0;
    while (*dptr >= '0' && *dptr <= '9')
    {
        num = (num * 10) + (*dptr - '0');
        dptr++;
    }
    return num;
}

/**
 *
 * @param dptr
 * @return
 */
static uint32_t XmlParseHex(xmlChar *dptr, uint8_t nibbles)
{
    uint32_t num = 0;
    while (nibbles--)
    {
        if (*dptr >= '0' && *dptr <= '9')
        {
            num = (num * 16) + (*dptr - '0');
        }
        else if (*dptr >= 'a' && *dptr <= 'f')
        {
            num = (num * 16) + 0xa + (*dptr - 'a');
        }
        else if (*dptr >= 'A' && *dptr <= 'F')
        {
            num = (num * 16) + 0xA + (*dptr - 'A');
        }
        else
        {
            break;
        }
        dptr++;
    }
    return num;
}

/**
 *
 * @param node
 * @return
 */
static uint32_t XmlGetContentInt(xmlNodePtr node)
{
    xmlChar *dptr;
    uint32_t num;

    NODE_CONTENT_GET(node, dptr);
    if (dptr != nullptr)
    {
        num = XmlParseInt(dptr);
        NODE_CONTENT_RELEASE();
    }
    else
    {
        num = 0;
    }
    return num;
}

/**
 *
 * @param node
 * @param nibbles
 * @return
 */
static uint32_t XmlGetContentHex(xmlNodePtr node, uint8_t nibbles)
{
    xmlChar *dptr;
    uint32_t num = 0;

    NODE_CONTENT_GET(node, dptr);
    if (dptr != nullptr)
    {
        num = XmlParseHex(dptr, nibbles);
        NODE_CONTENT_RELEASE();
    }
    return num;
}

/**
 *
 * @param node
 * @return
 */
static bool XmlGetContentBool(xmlNodePtr node)
{
    xmlChar *dptr;
    bool result = false;

    NODE_CONTENT_GET(node, dptr);
    if (dptr != nullptr)
    {
        if (xmlStrEqual(dptr, (const xmlChar *)"true"))
        {
            result = true;
        }
        NODE_CONTENT_RELEASE();
    }
    return result;
}

/**
 *
 * @param node
 * @return
 */
static std::string XmlGetContentString(xmlNodePtr node)
{
    xmlChar *dptr;
    std::string result;

    NODE_CONTENT_GET(node, dptr);
    if (dptr)
    {
        result = std::string(reinterpret_cast<char *>(dptr), xmlStrlen(dptr));
        NODE_CONTENT_RELEASE();
    }
    return result;
}

#ifndef USE_XML_API_FUNC
/**
 *
 * @param node
 * @param property
 * @return
 */
static xmlChar* XmlGetPropertyContent(xmlNodePtr node, const xmlChar *property)
{
    xmlAttrPtr attr;
    xmlChar *dptr = nullptr;
    attr = node->properties;
    while (attr != nullptr)
    {
        //assert(attr->type == XML_ATTRIBUTE_NODE);
        if (xmlStrEqual(attr->name, property) && attr->children)
        {
            dptr = attr->children->content;
            break;
        }
        attr = attr->next;
    }
    return dptr;
}

#endif // USE_XML_API_FUNC

/**
 *
 * @param node
 * @param app_ptr
 */
static void XmlAllocApplication(xmlNodePtr node, Ait::S_AIT_APP_DESC *app_ptr)
{
    uint32_t length = 0;
    uint8_t num_langs = 0, numTransports = 0;
    const xmlChar *cptr;
    xmlChar *dptr;
    Ait::S_LANG_STRING *langPtr;

    while (node != nullptr)
    {
        if (node->type == XML_ELEMENT_NODE)
        {
            cptr = node->name;
            LOG(LOG_DEBUG, "node name=%s", cptr);
            if (xmlStrEqual(cptr, (const xmlChar *)"appName"))
            {
                NODE_CONTENT_GET(node, dptr);
                if (dptr)
                {
                    length += xmlStrlen(dptr) + 1;
                    num_langs++;
                    NODE_CONTENT_RELEASE();
                }
            }
            else if (!xmlStrncmp(cptr, (const xmlChar *)"application", 11))
            {
                cptr += 11;
                if (xmlStrEqual(cptr, (const xmlChar *)"Transport"))
                {
                    numTransports++;
                }
            }
        }
        node = node->next;
    }
    app_ptr->appName.numLangs = num_langs;
    app_ptr->appName.names.resize(num_langs);
    app_ptr->numTransports = numTransports;
}

/**
 *
 * @param node
 * @param app_name
 */
static void XmlParseAppName(xmlNodePtr node, Ait::S_APP_NAME_DESC *app_name)
{
    xmlChar *dptr;
    uint8_t index = 0;
    for (; index < app_name->numLangs; index++)
    {
        if (!app_name->names[index].langCode)
        {
            break;
        }
    }
    if (index < app_name->numLangs)
    {
        dptr = NODE_PROPERTY(node, (const xmlChar *)"Language");
        if (dptr)
        {
            app_name->names[index].langCode = (*dptr << 16u) + (*(dptr + 1) << 8u) + *(dptr + 2);
            NODE_PROP_FREE(dptr)
        }
        NODE_CONTENT_GET(node, dptr);
        if (dptr != nullptr)
        {
            app_name->names[index].name = std::string(reinterpret_cast<char *>(dptr), xmlStrlen(
                dptr));
            NODE_CONTENT_RELEASE();
        }
    }
}

/**
 *
 * @param node
 * @param app_ptr
 */
static void XmlParseAppId(xmlNodePtr node, Ait::S_AIT_APP_DESC *app_ptr)
{
    const xmlChar *cptr;

    node = node->xmlChildrenNode;
    while (node != nullptr)
    {
        if (node->type == XML_ELEMENT_NODE)
        {
            cptr = node->name;
            if (xmlStrEqual(cptr, (const xmlChar *)"orgId"))
            {
                app_ptr->orgId = XmlGetContentInt(node);
            }
            else if (xmlStrEqual(cptr, (const xmlChar *)"appId"))
            {
                app_ptr->appId = (uint16_t)XmlGetContentInt(node);
            }
        }
        node = node->next;
    }
}

/**
 *
 * @param node
 * @param app_ptr
 */
static void XmlParseAppUsage(xmlNodePtr node, Ait::S_AIT_APP_DESC *app_ptr)
{
    const xmlChar *cptr;

    node = node->xmlChildrenNode;
    while (node != nullptr)
    {
        if (node->type == XML_ELEMENT_NODE)
        {
            cptr = node->name;
            if (xmlStrEqual(cptr, (const xmlChar *)"ApplicationUsage"))
            {
                app_ptr->appUsage = XmlGetContentString(node);
            }
        }
        node = node->next;
    }
}

/**
 *
 * @param node
 * @return
 */
static Ait::E_AIT_APP_CONTROL XmlGetContentEnumControl(xmlNodePtr node)
{
    xmlChar *dptr;
    Ait::E_AIT_APP_CONTROL result = Ait::APP_CTL_UNKNOWN;

    NODE_CONTENT_GET(node, dptr);
    if (dptr != nullptr)
    {
        if (xmlStrEqual(dptr, (const xmlChar *)"AUTOSTART"))
        {
            result = Ait::APP_CTL_AUTOSTART;
        }
        else if (xmlStrEqual(dptr, (const xmlChar *)"PRESENT"))
        {
            result = Ait::APP_CTL_PRESENT;
        }
        else if (xmlStrEqual(dptr, (const xmlChar *)"DESTROY"))
        {
            result = Ait::APP_CTL_DESTROY;
        }
        else if (xmlStrEqual(dptr, (const xmlChar *)"KILL"))
        {
            result = Ait::APP_CTL_KILL;
        }
        else if (xmlStrEqual(dptr, (const xmlChar *)"PREFETCH"))
        {
            result = Ait::APP_CTL_PREFETCH;
        }
        else if (xmlStrEqual(dptr, (const xmlChar *)"REMOTE"))
        {
            result = Ait::APP_CTL_REMOTE;
        }
        else if (xmlStrEqual(dptr, (const xmlChar *)"DISABLED"))
        {
            result = Ait::APP_CTL_DISABLED;
        }
        else if (xmlStrEqual(dptr, (const xmlChar *)"PLAYBACK_AUTOSTART"))
        {
            result = Ait::APP_CTL_PB_AUTO;
        }
        NODE_CONTENT_RELEASE();
    }

    return result;
}

/**
 *
 * @param node
 * @return
 */
static uint8_t XmlGetContentVisibility(xmlNodePtr node)
{
    xmlChar *dptr;
    uint8_t visibility = AIT_NOT_VISIBLE_ALL;

    NODE_CONTENT_GET(node, dptr);
    if (dptr)
    {
        if (xmlStrEqual(dptr, (const xmlChar *)"VISIBLE_ALL"))
        {
            visibility = AIT_VISIBLE_ALL;
        }
        else if (xmlStrEqual(dptr, (const xmlChar *)"NOT_VISIBLE_ALL"))
        {
            visibility = AIT_NOT_VISIBLE_ALL;
        }
        else if (xmlStrEqual(dptr, (const xmlChar *)"NOT_VISIBLE_USERS"))
        {
            visibility = AIT_NOT_VISIBLE_USERS;
        }
        NODE_CONTENT_RELEASE();
    }

    return visibility;
}

/**
 *
 * @param node
 * @param app_ptr
 */
static void XmlParseAppDescType(xmlNodePtr node, Ait::S_AIT_APP_DESC *app_ptr)
{
    const xmlChar *cptr;
    xmlChar *dptr;

    node = node->xmlChildrenNode;
    while (node != nullptr)
    {
        if (node->type == XML_ELEMENT_NODE)
        {
            cptr = node->name;
            if (xmlStrEqual(cptr, (const xmlChar *)"OtherApp"))
            {
                NODE_CONTENT_GET(node, dptr);
                if (dptr)
                {
                    /* Only recognise mime type for hbbtv ... */
                    if (xmlStrEqual(cptr, (const xmlChar *)"application/vnd.hbbtv.xhtml+xml"))
                    {
                        app_ptr->xmlType = Ait::XML_TYP_OTHER;
                    }
                    NODE_CONTENT_RELEASE();
                }
            }
            else if (xmlStrEqual(cptr, (const xmlChar *)"DvbApp"))
            {
                NODE_CONTENT_GET(node, dptr);
                if (dptr)
                {
                    if (xmlStrEqual(cptr, (const xmlChar *)"DVB-J"))
                    {
                        app_ptr->xmlType = Ait::XML_TYP_DVB_J;
                    }
                    else if (xmlStrEqual(cptr, (const xmlChar *)"DVB-HTML"))
                    {
                        app_ptr->xmlType = Ait::XML_TYP_DVB_HTML;
                    }
                    NODE_CONTENT_RELEASE();
                }
            }
        }
        node = node->next;
    }
}

/**
 *
 * @param node
 * @param prof_ptr
 */
static void XmlParseAppDescProfile(xmlNodePtr node, Ait::S_AIT_APP_DESC *app_ptr)
{
    const xmlChar *cptr;
    Ait::S_APP_PROFILE appProfile;

    node = node->xmlChildrenNode;
    while (node != nullptr)
    {
        if (node->type == XML_ELEMENT_NODE)
        {
            cptr = node->name;
            if (xmlStrEqual(cptr, (const xmlChar *)"profile"))
            {
                appProfile.appProfile = (uint16_t)XmlGetContentHex(node, 4);
            }
            else if (!xmlStrncmp(cptr, (const xmlChar *)"versionM", 8))
            {
                cptr += 8;
                if (xmlStrEqual(cptr, (const xmlChar *)"ajor"))
                {
                    appProfile.versionMajor = (uint8_t)XmlGetContentHex(node, 2);
                }
                else if (xmlStrEqual(cptr, (const xmlChar *)"inor"))
                {
                    appProfile.versionMinor = (uint8_t)XmlGetContentHex(node, 2);
                }
                else if (xmlStrEqual(cptr, (const xmlChar *)"icro"))
                {
                    appProfile.versionMicro = (uint8_t)XmlGetContentHex(node, 2);
                }
            }
        }
        node = node->next;
    }
    app_ptr->appDesc.appProfiles.push_back(appProfile);
}

/**
 *
 * @param node
 * @param app_ptr
 */
static void XmlParseAppDescGraphics(xmlNodePtr node, Ait::S_AIT_APP_DESC *app_ptr)
{
    const xmlChar *cptr;
    xmlChar *dptr;

    node = node->xmlChildrenNode;
    app_ptr->graphicsConstraints.push_back(720);
    while (node != nullptr)
    {
        if (node->type == XML_ELEMENT_NODE)
        {
            cptr = node->name;
            if (xmlStrEqual(cptr, (const xmlChar *)"GraphicsConfiguration"))
            {
                NODE_CONTENT_GET(node, dptr);
                if (dptr)
                {
                    if (xmlStrEqual(dptr, (const
                                           xmlChar *)"urn:hbbtv:graphics:resolution:1920x1080"))
                    {
                        app_ptr->graphicsConstraints.push_back(1080);
                    }
                    else if (xmlStrEqual(dptr, (const
                                                xmlChar *)"urn:hbbtv:graphics:resolution:3840x2160"))
                    {
                        app_ptr->graphicsConstraints.push_back(2160);
                    }
                    else if (xmlStrEqual(dptr, (const
                                                xmlChar *)"urn:hbbtv:graphics:resolution:7680x4320"))
                    {
                        app_ptr->graphicsConstraints.push_back(4320);
                    }
                    NODE_CONTENT_RELEASE();
                }
            }
        }
        node = node->next;
    }
}

/**
 *
 * @param node
 * @param app_ptr
 */
static void XmlParseAppDesc(xmlNodePtr node, Ait::S_AIT_APP_DESC *app_ptr)
{
    const xmlChar *cptr;
    xmlChar *dptr;
    /* TS 102809, sec 5.4.4.4 states that service_bound default is true */
    app_ptr->appDesc.serviceBound = true;

    node = node->xmlChildrenNode;
    while (node != nullptr)
    {
        if (node->type == XML_ELEMENT_NODE)
        {
            cptr = node->name;
            if (xmlStrEqual(cptr, (const xmlChar *)"type"))
            {
                XmlParseAppDescType(node, app_ptr);
            }
            else if (xmlStrEqual(cptr, (const xmlChar *)"controlCode"))
            {
                app_ptr->controlCode = (uint8_t)XmlGetContentEnumControl(node);
            }
            else if (xmlStrEqual(cptr, (const xmlChar *)"visibility"))
            {
                app_ptr->appDesc.visibility = XmlGetContentVisibility(node);
            }
            else if (xmlStrEqual(cptr, (const xmlChar *)"serviceBound"))
            {
                app_ptr->appDesc.serviceBound = XmlGetContentBool(node);
            }
            else if (xmlStrEqual(cptr, (const xmlChar *)"priority"))
            {
                app_ptr->appDesc.priority = (uint8_t)XmlGetContentHex(node, 2);
            }
            else if (xmlStrEqual(cptr, (const xmlChar *)"version"))
            {
                app_ptr->xmlVersion = (uint8_t)XmlGetContentInt(node);
            }
            else if (xmlStrEqual(cptr, (const xmlChar *)"mhpVersion"))
            {
                XmlParseAppDescProfile(node, app_ptr);
            }
            else if (xmlStrEqual(cptr, (const xmlChar *)"icon"))
            {
            }
            else if (xmlStrEqual(cptr, (const xmlChar *)"storageCapabilities"))
            {
            }
            else if (xmlStrEqual(cptr, (const xmlChar *)"ParentalRating"))
            {
                Ait::S_APP_PARENTAL_RATING pr;

                dptr = NODE_PROPERTY(node, (const xmlChar *)"Scheme");
                if (dptr)
                {
                    pr.scheme = std::string(reinterpret_cast<char *>(dptr), xmlStrlen(dptr));
                    NODE_PROP_FREE(dptr)
                }
                dptr = NODE_PROPERTY(node, (const xmlChar *)"Region");
                if (dptr)
                {
                    pr.region = std::string(reinterpret_cast<char *>(dptr), xmlStrlen(dptr));
                    NODE_PROP_FREE(dptr)
                }
                pr.value = (uint8_t)XmlGetContentInt(node);
                app_ptr->parentalRatings.push_back(pr);
            }
            else if (xmlStrEqual(cptr, (const xmlChar *)"GraphicsConstraints"))
            {
                XmlParseAppDescGraphics(node, app_ptr);
            }
        }
        node = node->next;
    }
}

/**
 *
 * @param node
 * @param app_ptr
 * @return
 */
static uint32_t XmlParseAppBoundary(xmlNodePtr node, Ait::S_AIT_APP_DESC *app_ptr)
{
    const xmlChar *cptr;
    xmlChar *dptr;
    uint32_t len;

    node = node->xmlChildrenNode;
    while (node != nullptr)
    {
        if (node->type == XML_ELEMENT_NODE)
        {
            cptr = node->name;
            if (xmlStrEqual(cptr, (const xmlChar *)"BoundaryExtension"))
            {
                NODE_CONTENT_GET(node, dptr);
                if (dptr)
                {
                    len = xmlStrlen(dptr);
                    std::string boundary(reinterpret_cast<char *>(dptr), len);
                    LOG(LOG_DEBUG, "additional boundary: \"%s\"", boundary.c_str());
                    app_ptr->boundaries.push_back(boundary);
                    NODE_CONTENT_RELEASE();
                }
            }
        }
        node = node->next;
    }

    return len;
}

/**
 * Implement parsing of DVBTripet. See TS 102 034 v1.4.1, section C.1.3.10
 * @param node
 * @param oc
 */
static void XmlParseDvbTriplet(xmlNodePtr node, Utils::S_DVB_TRIPLET *dvb)
{
    const xmlChar *cptr;
    xmlChar *dptr;
    dptr = NODE_PROPERTY(node, (const xmlChar *)"OrigNetId");
    if (dptr)
    {
        dvb->originalNetworkId = (uint16_t)XmlParseInt(dptr);
        NODE_PROP_FREE(dptr)
    }
    dptr = NODE_PROPERTY(node, (const xmlChar *)"TSId");
    if (dptr)
    {
        dvb->transportStreamId = (uint16_t)XmlParseInt(dptr);
        NODE_PROP_FREE(dptr)
    }
    dptr = NODE_PROPERTY(node, (const xmlChar *)"ServiceId");
    if (dptr)
    {
        dvb->serviceId = (uint16_t)XmlParseInt(dptr);
        NODE_PROP_FREE(dptr)
    }
}

/**
 *
 * @param node
 * @param trns
 */
static void XmlParseAppTransport(xmlNodePtr node, Ait::S_TRANSPORT_PROTOCOL_DESC *trns)
{
    const xmlChar *cptr;
    xmlChar *dptr;
    uint16_t protocolId = 0;
    uint8_t i, freeIndex;
    Ait::S_TRANSPORT_PROTOCOL_DESC *trns_ptr;

    dptr = NODE_PROPERTY(node, (const xmlChar *)"type");
    if (dptr)
    {
        if (xmlStrEqual(dptr, (const xmlChar *)"mhp:HTTPTransportType"))
        {
            protocolId = AIT_PROTOCOL_HTTP;
        }
        else if (xmlStrEqual(dptr, (const xmlChar *)"mhp:OCTransportType"))
        {
            protocolId = AIT_PROTOCOL_OBJECT_CAROUSEL;
        }
        NODE_PROP_FREE(dptr)
    }

    freeIndex = AIT_MAX_NUM_PROTOCOLS;
    for (i = 0; i < AIT_MAX_NUM_PROTOCOLS; i++)
    {
        if (protocolId == trns[i].protocolId)
        {
            break;
        }
        if ((trns[i].protocolId == 0) && (freeIndex == AIT_MAX_NUM_PROTOCOLS))
        {
            /* Save the first free index to be used for the new protocol */
            freeIndex = i;
        }
    }
    if (freeIndex == AIT_MAX_NUM_PROTOCOLS)
    {
        LOG(LOG_ERROR, "No free slots for this protocol: %d", protocolId);
        i = AIT_MAX_NUM_PROTOCOLS;
    }

    if (i >= AIT_MAX_NUM_PROTOCOLS)
    {
        trns_ptr = &(trns[freeIndex]);
        trns_ptr->protocolId = protocolId;

        node = node->xmlChildrenNode;
        switch (protocolId)
        {
            case AIT_PROTOCOL_HTTP:
                // See TS 102 809, section 5.4.4.20
                /* Length the URL extensions */
                while (node != nullptr)
                {
                    if (node->type == XML_ELEMENT_NODE)
                    {
                        cptr = node->name;
                        if (xmlStrEqual(cptr, (const xmlChar *)"URLBase"))
                        {
                            NODE_CONTENT_GET(node, dptr);
                            if (dptr)
                            {
                                trns_ptr->url.baseUrl = std::string(reinterpret_cast<char *>(dptr),
                                    xmlStrlen(dptr));
                                NODE_CONTENT_RELEASE();
                            }
                        }
                        else if (xmlStrEqual(cptr, (const xmlChar *)"URLExtension"))
                        {
                            NODE_CONTENT_GET(node, dptr);
                            if (dptr)
                            {
                                trns_ptr->url.extensionUrls.emplace_back(
                                    reinterpret_cast<char *>(dptr),
                                    xmlStrlen(dptr));
                                NODE_CONTENT_RELEASE();
                            }
                        }
                    }
                    node = node->next;
                }
                break;

            case AIT_PROTOCOL_OBJECT_CAROUSEL:
                // See TS 102 809, section 5.4.4.21
                while (node != nullptr)
                {
                    if (node->type == XML_ELEMENT_NODE)
                    {
                        cptr = node->name;
                        LOG(LOG_DEBUG, "OC: node name=%s", cptr);
                        if (xmlStrEqual(cptr, (const xmlChar *)"DvbTriplet"))
                        {
                            XmlParseDvbTriplet(node, &trns_ptr->oc.dvb);
                            trns_ptr->oc.remoteConnection = true;
                        }
                        else if (xmlStrEqual(cptr, (const xmlChar *)"TextualId"))
                        {
                        }
                        else if (xmlStrEqual(cptr, (const xmlChar *)"ComponentTag"))
                        {
                            // spec says this element MUST be present: minOccurs="1" maxOccurs="1"
                            dptr = NODE_PROPERTY(node, (const xmlChar *)"ComponentTag");
                            if (dptr)
                            {
                                trns_ptr->oc.componentTag = (uint8_t)XmlParseHex(dptr, 2);
                                LOG(LOG_DEBUG, "ComponentTag=%x", trns_ptr->oc.componentTag);
                                NODE_PROP_FREE(dptr)
                            }
                            else
                            {
                                LOG(LOG_ERROR, "No ComponentTag attr");
                            }
                        }
                    }
                    node = node->next;
                }
                break;

            default:;
        }

        trns_ptr->failedToLoad = false;
    }
    else
    {
        LOG(LOG_DEBUG, "protocol %d already parsed for this app, skipping", protocolId);
    }
}

/**
 *
 * @param node
 * @param app_ptr
 */
static void XmlParseAppLocation(xmlNodePtr node, Ait::S_AIT_APP_DESC *app_ptr)
{
    xmlChar *dptr;

    NODE_CONTENT_GET(node, dptr);
    if (dptr)
    {
        app_ptr->location = std::string(reinterpret_cast<char *>(dptr), xmlStrlen(dptr));
        LOG(LOG_DEBUG, "location: %s", app_ptr->location.c_str());
        NODE_CONTENT_RELEASE();
    }
}

/**
 *
 * @param node
 * @param app_ptr
 */
static void XmlParseApplication(xmlNodePtr node, Ait::S_AIT_APP_DESC *app_ptr)
{
    const xmlChar *cptr;

    while (node != nullptr)
    {
        if (node->type == XML_ELEMENT_NODE)
        {
            cptr = node->name;
            LOG(LOG_DEBUG, "node name=%s", cptr);
            if (xmlStrEqual(cptr, (const xmlChar *)"appName"))
            {
                XmlParseAppName(node, &app_ptr->appName);
            }
            else if (!xmlStrncmp(cptr, (const xmlChar *)"application", 11))
            {
                cptr += 11;
                if (xmlStrEqual(cptr, (const xmlChar *)"Identifier"))
                {
                    XmlParseAppId(node, app_ptr);
                }
                else if (xmlStrEqual(cptr, (const xmlChar *)"Descriptor"))
                {
                    XmlParseAppDesc(node, app_ptr);
                }
                else if (xmlStrEqual(cptr, (const xmlChar *)"UsageDescriptor"))
                {
                    XmlParseAppUsage(node, app_ptr);
                }
                else if (xmlStrEqual(cptr, (const xmlChar *)"Boundary"))
                {
                    XmlParseAppBoundary(node, app_ptr);
                }
                else if (xmlStrEqual(cptr, (const xmlChar *)"Transport"))
                {
                    XmlParseAppTransport(node, app_ptr->transportArray);
                }
                else if (xmlStrEqual(cptr, (const xmlChar *)"Location"))
                {
                    XmlParseAppLocation(node, app_ptr);
                }
            }
            else if (xmlStrEqual(cptr, (const xmlChar *)"GraphicsConstraints")) {
                XmlParseAppDescGraphics(node, app_ptr);
            }
        }
        node = node->next;
    }
}

/**
 *
 * @param node
 * @param app_ptr
 */
static void XmlParseApplications(xmlNodePtr node, Ait::S_AIT_TABLE *ait_table)
{
    uint32_t index = 0;
    while (node != nullptr)
    {
        if (node->type == XML_ELEMENT_NODE &&
            xmlStrEqual(node->name, (const xmlChar *)"ApplicationDiscovery"))
        {
            xmlNodePtr dnode;
            dnode = node->xmlChildrenNode;
            while (dnode != nullptr)
            {
                if (dnode->type == XML_ELEMENT_NODE &&
                    xmlStrEqual(dnode->name, (const xmlChar *)"ApplicationList"))
                {
                    xmlNodePtr lnode = dnode->xmlChildrenNode;
                    while (lnode != nullptr)
                    {
                        if (lnode->type == XML_ELEMENT_NODE &&
                            xmlStrEqual(lnode->name, (const xmlChar *)"Application"))
                        {
                            /* Gather required malloc sizes for this application */
                            XmlAllocApplication(lnode->xmlChildrenNode,
                                &ait_table->appArray[index]);
                            XmlParseApplication(lnode->xmlChildrenNode,
                                &ait_table->appArray[index]);
                            index++;
                        }
                        lnode = lnode->next;
                    }
                }
                dnode = dnode->next;
            }
        }
        node = node->next;
    }
}

/**
 *
 * @param node
 * @return
 */
static uint32_t XmlCountApplications(xmlNodePtr node)
{
    uint32_t count = 0;

    while (node != nullptr)
    {
        if (node->type == XML_ELEMENT_NODE &&
            xmlStrEqual(node->name, (const xmlChar *)"ApplicationDiscovery"))
        {
            xmlNodePtr dnode = node->xmlChildrenNode;
            while (dnode != nullptr)
            {
                if (dnode->type == XML_ELEMENT_NODE &&
                    xmlStrEqual(dnode->name, (const xmlChar *)"ApplicationList"))
                {
                    xmlNodePtr lnode = dnode->xmlChildrenNode;
                    while (lnode != nullptr)
                    {
                        if (lnode->type == XML_ELEMENT_NODE &&
                            xmlStrEqual(lnode->name, (const xmlChar *)"Application"))
                        {
                            count++;
                        }
                        lnode = lnode->next;
                    }
                }
                dnode = dnode->next;
            }
        }
        node = node->next;
    }

    return count;
}

/**
 * Parse Xml data as specified in TS 102 809 section 5.4
 * @param content pointer to Xml data
 * @param length length of Xml data
 * @return AIT table data in same format as generated from DVB broadcast data
 */
std::unique_ptr<Ait::S_AIT_TABLE> XmlParser::ParseAit(const char *content, uint32_t length)
{
    std::unique_ptr<Ait::S_AIT_TABLE> aitTable = nullptr;
    xmlDocPtr doc;
    xmlNodePtr node;
    uint32_t numApps;
    Ait::S_AIT_APP_DESC *appPtr;
    int options = 0;

#ifdef RDK
    options = XML_PARSE_RECOVER;
#endif


    LOG(LOG_DEBUG, "data=%p len=%d", content, length);
    doc = xmlReadMemory(content, length, "noname.xml", nullptr, options);
    if (doc == nullptr)
    {
        LOG(LOG_ERROR, "Failed to parse document!!");
    }
    else
    {
        node = xmlDocGetRootElement(doc);
        if (node == nullptr)
        {
            LOG(LOG_ERROR, "Empty document");
        }
        else
        {
            node = node->xmlChildrenNode;
            numApps = XmlCountApplications(node);
            aitTable = std::make_unique<Ait::S_AIT_TABLE>();
            if (aitTable != nullptr)
            {
                aitTable->appType = Ait::APP_TYP_XML;
                aitTable->numApps = (uint8_t)numApps;
                aitTable->appArray.resize(aitTable->numApps);
                XmlParseApplications(node, aitTable.get());
            }
        }
        xmlFreeDoc(doc);
    }

    return aitTable;
}

#if 0 // TODO(C++-ize)

/**
 *
 * @param dsm_node
 * @param se_obj
 * @param sevent
 * @param strptr
 */
static void XmlParseDsmObjects(xmlNodePtr dsm_node, XmlParser::S_XML_SE_OBJ *se_obj,
    XmlParser::S_XML_SEVENT *sevent, uint8_t *strptr)
{
    uint16_t ecnt;
    xmlChar *dptr;
    xmlNodePtr se_node;
    uint16_t nsz;

    while (dsm_node != nullptr)
    {
        if (dsm_node->type == XML_ELEMENT_NODE &&
            xmlStrEqual(dsm_node->name, (const xmlChar *)"dsmcc_object"))
        {
            dptr = NODE_PROPERTY(dsm_node, (const xmlChar *)"component_tag");
            if (dptr)
            {
                se_obj->component_tag = (uint16_t)XmlParseInt(dptr);
                NODE_PROP_FREE(dptr);
            }
            ecnt = 0;
            se_obj->events = sevent;
            se_node = dsm_node->xmlChildrenNode;
            while (se_node != nullptr)
            {
                if (se_node->type == XML_ELEMENT_NODE &&
                    xmlStrEqual(se_node->name, (const xmlChar *)"stream_event"))
                {
                    dptr = NODE_PROPERTY(se_node, (const xmlChar *)"stream_event_id");
                    if (dptr)
                    {
                        sevent->event_id = (uint16_t)XmlParseInt(dptr);
                        NODE_PROP_FREE(dptr);
                    }
                    dptr = NODE_PROPERTY(se_node, (const xmlChar *)"stream_event_name");
                    if (dptr)
                    {
                        nsz = xmlStrlen(dptr);
                        sevent->name_len = nsz;
                        sevent->name_ptr = strptr;
                        memcpy(strptr, dptr, nsz);
                        strptr += nsz;
                        *strptr++ = 0; // add null char
                        NODE_PROP_FREE(dptr);
                    }
                    else
                    {
                        sevent->name_len = 0;
                        sevent->name_ptr = nullptr;
                    }
                    ecnt++;
                }
                se_node = se_node->next;
            }
            se_obj->event_count = ecnt;
            se_obj++;
        }
        dsm_node = dsm_node->next;
    }
}

/**
 *
 * @param dsm_node
 * @param event_cnt
 * @param name_sz
 * @return
 */
static uint16_t XmlCountDsmObjects(xmlNodePtr dsm_node, uint16_t *event_cnt, uint16_t *name_sz)
{
    uint16_t count = 0, ecnt = 0, nsz = 0;
    xmlNodePtr se_node;
    xmlChar *dptr;

    while (dsm_node != nullptr)
    {
        if (dsm_node->type == XML_ELEMENT_NODE &&
            xmlStrEqual(dsm_node->name, (const xmlChar *)"dsmcc_object"))
        {
            se_node = dsm_node->xmlChildrenNode;
            while (se_node != nullptr)
            {
                if (se_node->type == XML_ELEMENT_NODE &&
                    xmlStrEqual(se_node->name, (const xmlChar *)"stream_event"))
                {
                    dptr = NODE_PROPERTY(se_node, (const xmlChar *)"stream_event_name");
                    if (dptr)
                    {
                        nsz += xmlStrlen(dptr) + 1;
                        NODE_PROP_FREE(dptr);
                    }
                    ecnt++;
                }
                se_node = se_node->next;
            }
            count++;
        }
        dsm_node = dsm_node->next;
    }
    *event_cnt = ecnt;
    *name_sz = nsz;

    return count;
}

/**
 * Parse Xml data as specified in TS 102 809 section 8.2
 * @param content pointer to Xml data
 * @param length length of Xml data
 * @return Dsmcc object data Stream event object data
 */
XmlParser::S_XML_DSMCC * XmlParser::ParseDsmcc(uint8_t *content, uint32_t length)
{
    XmlParser::S_XML_DSMCC *dsmcc_objs;
    xmlDocPtr doc;
    xmlNodePtr node;
    uint16_t dsm_cnt, evt_cnt, name_sz = 0;
    XmlParser::S_XML_SE_OBJ *se_objs;
    XmlParser::S_XML_SEVENT *events;

    LOG(LOG_DEBUG, "data=%p len=%d", content, length);
    doc = xmlReadMemory((const char *)content, length, "noname.xml", nullptr, 0);
    if (doc == nullptr)
    {
        LOG(LOG_ERROR, "Failed to parse document");
        dsmcc_objs = nullptr;
    }
    else
    {
        node = xmlDocGetRootElement(doc);
        if (node == nullptr)
        {
            LOG(LOG_ERROR, "Empty document");
            dsmcc_objs = nullptr;
        }
        else if (node->type != XML_ELEMENT_NODE || !xmlStrEqual(node->name, (const
                                                                             xmlChar *)"dsmcc"))
        {
            LOG(LOG_ERROR, "Wrong root object");
            dsmcc_objs = nullptr;
        }
        else
        {
            node = node->xmlChildrenNode;
            dsm_cnt = XmlCountDsmObjects(node, &evt_cnt, &name_sz);
            dsmcc_objs = (XmlParser::S_XML_DSMCC *)malloc(sizeof(XmlParser::S_XML_DSMCC) +
                (dsm_cnt * sizeof(XmlParser::S_XML_SE_OBJ)) +
                (evt_cnt * sizeof(XmlParser::S_XML_SEVENT)) + name_sz);
            if (dsmcc_objs != nullptr)
            {
                dsmcc_objs->se_total = dsm_cnt;
                se_objs = (XmlParser::S_XML_SE_OBJ *)(dsmcc_objs + 1);
                events = (XmlParser::S_XML_SEVENT *)(se_objs + dsm_cnt);
                dsmcc_objs->se_objs = se_objs;
                XmlParseDsmObjects(node, se_objs, events, (uint8_t *)(events + evt_cnt));
            }
        }
        xmlFreeDoc(doc);
    }
    return dsmcc_objs;
}

/**
 * Clear/free table created by XmlParser::ParseDsmcc().
 * @param dsm_objs pointer to dsmcc obects
 */
void XmlParser::FreeDsmcc(XmlParser::S_XML_DSMCC *dsm_objs)
{
    if (dsm_objs != nullptr)
    {
        free(dsm_objs);
    }
}

#endif
