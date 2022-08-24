/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
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
    uint8_t num_langs = 0, num_transports = 0;
    const xmlChar *cptr;
    xmlChar *dptr;
    Ait::S_LANG_STRING *lang_ptr;

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
                    num_transports++;
                }
            }
        }
        node = node->next;
    }
    app_ptr->app_name.num_langs = num_langs;
    app_ptr->app_name.names.resize(num_langs);
    app_ptr->num_transports = num_transports;
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
    for (; index < app_name->num_langs; index++)
    {
        if (!app_name->names[index].lang_code)
        {
            break;
        }
    }
    if (index < app_name->num_langs)
    {
        dptr = NODE_PROPERTY(node, (const xmlChar *)"Language");
        if (dptr)
        {
            app_name->names[index].lang_code = (*dptr << 16u) + (*(dptr + 1) << 8u) + *(dptr + 2);
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
                app_ptr->org_id = XmlGetContentInt(node);
            }
            else if (xmlStrEqual(cptr, (const xmlChar *)"appId"))
            {
                app_ptr->app_id = (uint16_t)XmlGetContentInt(node);
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
                        app_ptr->xml_type = Ait::XML_TYP_OTHER;
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
                        app_ptr->xml_type = Ait::XML_TYP_DVB_J;
                    }
                    else if (xmlStrEqual(cptr, (const xmlChar *)"DVB-HTML"))
                    {
                        app_ptr->xml_type = Ait::XML_TYP_DVB_HTML;
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
    Ait::S_APP_PROFILE app_profile;

    node = node->xmlChildrenNode;
    while (node != nullptr)
    {
        if (node->type == XML_ELEMENT_NODE)
        {
            cptr = node->name;
            if (xmlStrEqual(cptr, (const xmlChar *)"profile"))
            {
                app_profile.app_profile = (uint16_t)XmlGetContentHex(node, 4);
            }
            else if (!xmlStrncmp(cptr, (const xmlChar *)"versionM", 8))
            {
                cptr += 8;
                if (xmlStrEqual(cptr, (const xmlChar *)"ajor"))
                {
                    app_profile.version_major = (uint8_t)XmlGetContentHex(node, 2);
                }
                else if (xmlStrEqual(cptr, (const xmlChar *)"inor"))
                {
                    app_profile.version_minor = (uint8_t)XmlGetContentHex(node, 2);
                }
                else if (xmlStrEqual(cptr, (const xmlChar *)"icro"))
                {
                    app_profile.version_micro = (uint8_t)XmlGetContentHex(node, 2);
                }
            }
        }
        node = node->next;
    }
    app_ptr->app_desc.app_profiles.push_back(app_profile);
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
    app_ptr->app_desc.service_bound = true;

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
                app_ptr->control_code = (uint8_t)XmlGetContentEnumControl(node);
            }
            else if (xmlStrEqual(cptr, (const xmlChar *)"visibility"))
            {
                app_ptr->app_desc.visibility = XmlGetContentVisibility(node);
            }
            else if (xmlStrEqual(cptr, (const xmlChar *)"serviceBound"))
            {
                app_ptr->app_desc.service_bound = XmlGetContentBool(node);
            }
            else if (xmlStrEqual(cptr, (const xmlChar *)"priority"))
            {
                app_ptr->app_desc.priority = (uint8_t)XmlGetContentHex(node, 2);
            }
            else if (xmlStrEqual(cptr, (const xmlChar *)"version"))
            {
                app_ptr->xml_version = (uint8_t)XmlGetContentInt(node);
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
                app_ptr->parental_ratings.push_back(pr);
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
        dvb->original_network_id = (uint16_t)XmlParseInt(dptr);
        NODE_PROP_FREE(dptr)
    }
    dptr = NODE_PROPERTY(node, (const xmlChar *)"TSId");
    if (dptr)
    {
        dvb->transport_stream_id = (uint16_t)XmlParseInt(dptr);
        NODE_PROP_FREE(dptr)
    }
    dptr = NODE_PROPERTY(node, (const xmlChar *)"ServiceId");
    if (dptr)
    {
        dvb->service_id = (uint16_t)XmlParseInt(dptr);
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
    uint16_t protocol_id = 0;
    uint8_t i, free_index;
    Ait::S_TRANSPORT_PROTOCOL_DESC *trns_ptr;

    dptr = NODE_PROPERTY(node, (const xmlChar *)"type");
    if (dptr)
    {
        if (xmlStrEqual(dptr, (const xmlChar *)"mhp:HTTPTransportType"))
        {
            protocol_id = AIT_PROTOCOL_HTTP;
        }
        else if (xmlStrEqual(dptr, (const xmlChar *)"mhp:OCTransportType"))
        {
            protocol_id = AIT_PROTOCOL_OBJECT_CAROUSEL;
        }
        NODE_PROP_FREE(dptr)
    }

    free_index = AIT_MAX_NUM_PROTOCOLS;
    for (i = 0; i < AIT_MAX_NUM_PROTOCOLS; i++)
    {
        if (protocol_id == trns[i].protocol_id)
        {
            break;
        }
        if ((trns[i].protocol_id == 0) && (free_index == AIT_MAX_NUM_PROTOCOLS))
        {
            /* Save the first free index to be used for the new protocol */
            free_index = i;
        }
    }
    if (free_index == AIT_MAX_NUM_PROTOCOLS)
    {
        LOG(LOG_ERROR, "No free slots for this protocol: %d", protocol_id);
        i = AIT_MAX_NUM_PROTOCOLS;
    }

    if (i >= AIT_MAX_NUM_PROTOCOLS)
    {
        trns_ptr = &(trns[free_index]);
        trns_ptr->protocol_id = protocol_id;

        node = node->xmlChildrenNode;
        switch (protocol_id)
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
                                trns_ptr->url.base_url = std::string(reinterpret_cast<char *>(dptr),
                                    xmlStrlen(dptr));
                                NODE_CONTENT_RELEASE();
                            }
                        }
                        else if (xmlStrEqual(cptr, (const xmlChar *)"URLExtension"))
                        {
                            NODE_CONTENT_GET(node, dptr);
                            if (dptr)
                            {
                                trns_ptr->url.extension_urls.emplace_back(
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
                            trns_ptr->oc.remote_connection = true;
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
                                trns_ptr->oc.component_tag = (uint8_t)XmlParseHex(dptr, 2);
                                LOG(LOG_DEBUG, "ComponentTag=%x", trns_ptr->oc.component_tag);
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

        trns_ptr->failed_to_load = false;
    }
    else
    {
        LOG(LOG_DEBUG, "protocol %d already parsed for this app, skipping", protocol_id);
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
                XmlParseAppName(node, &app_ptr->app_name);
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
                else if (xmlStrEqual(cptr, (const xmlChar *)"Boundary"))
                {
                    XmlParseAppBoundary(node, app_ptr);
                }
                else if (xmlStrEqual(cptr, (const xmlChar *)"Transport"))
                {
                    XmlParseAppTransport(node, app_ptr->transport_array);
                }
                else if (xmlStrEqual(cptr, (const xmlChar *)"Location"))
                {
                    XmlParseAppLocation(node, app_ptr);
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
                                &ait_table->app_array[index]);
                            XmlParseApplication(lnode->xmlChildrenNode,
                                &ait_table->app_array[index]);
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
    std::unique_ptr<Ait::S_AIT_TABLE> ait_table = nullptr;
    xmlDocPtr doc;
    xmlNodePtr node;
    uint32_t num_apps;
    Ait::S_AIT_APP_DESC *app_ptr;


    LOG(LOG_DEBUG, "data=%p len=%d", content, length);
    doc = xmlReadMemory(content, length, "noname.xml", nullptr, 0);
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
            num_apps = XmlCountApplications(node);
            ait_table = std::make_unique<Ait::S_AIT_TABLE>();
            if (ait_table != nullptr)
            {
                ait_table->app_type = Ait::APP_TYP_XML;
                ait_table->num_apps = (uint8_t)num_apps;
                ait_table->app_array.resize(ait_table->num_apps);
                XmlParseApplications(node, ait_table.get());
            }
        }
        xmlFreeDoc(doc);
    }

    return ait_table;
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
