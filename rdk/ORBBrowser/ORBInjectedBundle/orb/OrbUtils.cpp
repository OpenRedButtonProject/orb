/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

/*---includes for this file--------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include "OrbUtils.h"
#include "ORBBridge.h"

using namespace orb;

/*---constant definitions for this file--------------------------------------*/
#ifndef UTILS_SUCCESS
#define UTILS_SUCCESS 0
#endif

#ifndef UTILS_FAILURE
#define UTILS_FAILURE 1
#endif

#ifndef JAVASCRIPT_PAYLOAD_PATH
#warning Using default JS payload path
#define JAVASCRIPT_PAYLOAD_PATH "/usr/share/WPEFramework/ORBBrowser"
#endif


/*---local typedef structs for this file-------------------------------------*/

/*---local (static) variable declarations for this file----------------------*/
/*   (internal variables declared static to make them local) */


/*---local function prototypes for this file---------------------------------*/
/*   (internal functions declared static to make them local) */
static int ReadTextFileIntoStringBuffer(const char *absoluteFilePath, char **buffer,
    long *bufferSize);
static char* InsertAt(const char *destination, const char *chunk, int index);
static int FindInjectionIndex(const char *htmlSource);
static const char* PreparePayload();


/*---global function definitions---------------------------------------------*/


/**
 * Attempts to perform the JavaScript injection into the specified HTML source.
 *
 * @param htmlSource The HTML source
 * @param injected Output parameter holding the injection result (0 or !)
 *
 * @return The htmlSource after the injection, or NULL
 */
const char* InjectInto(const char *htmlSource, int *injected)
{
    const char *payload;
    const char *result = NULL;

    int injectionIndex = FindInjectionIndex(htmlSource);
    if (-1 == injectionIndex)
    {
        goto fail;
    }

    payload = PreparePayload();
    if (NULL == payload)
    {
        goto fail;
    }

    result = InsertAt(htmlSource, payload, injectionIndex);
    *injected = 1;

    return result;

fail:
    *injected = 0;
    return NULL;
}

/**
 * Attempts to load the specified DVB URL.
 * The caller shall be informed of the result via the provided callback.
 *
 * @param url The DVB URL
 * @param caller Raw pointer to the caller object
 * @param callback Callback function
 */
void LoadDvbUrl(const char *url, void *caller, OnDvbUrlLoaded callback)
{
    static int requestId = 0;
    requestId++;

    fprintf(stderr, "[Utils::LoadDvbUrl] url=%s requestId=%d\n", url, requestId);

    // This is the first time the ORBBridge signleton is called from the WPE network process.
    // The ORBClient instance needs to only subscribe with the 'dvburlloaded' event of the ORB Thunder plugin.
    ORBBridge::GetSharedInstance().GetORBClient()->SubscribeToDvbUrlLoadedEvent();
    ORBBridge::GetSharedInstance().AddDsmccCaller(requestId, caller);
    ORBBridge::GetSharedInstance().AddDsmccCallback(requestId, callback);
    ORBBridge::GetSharedInstance().GetORBClient()->LoadDvbUrl(url, requestId);
}

/*---local function definitions----------------------------------------------*/

/**
 * Reads the specified text file into a string buffer.
 *
 * @param absoluteFilePath (in) The absolute path of the text file
 * @param buffer (out) The string buffer that will hold the text file content
 * @param bufferSize (out) The string buffer size in bytes
 *
 * @return UTILS_SUCCESS or UTILS_FAILURE
 */
static int ReadTextFileIntoStringBuffer(const char *absoluteFilePath, char **buffer,
    long *bufferSize)
{
    char *buf = 0;
    long fileSize;
    FILE *fp = NULL;

    /* open file */
    fp = fopen(absoluteFilePath, "rb");
    if (NULL == fp)
    {
        return UTILS_FAILURE;
    }

    /* resolve file size */
    fseek(fp, 0, SEEK_END);
    fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    /* allocate buffer memory and read file into buffer */
    buf = (char *)malloc(fileSize + 1);
    if (NULL == buf)
    {
        return UTILS_FAILURE;
    }
    fread(buf, 1, fileSize, fp);
    buf[fileSize] = '\0';

    /* close file */
    fclose(fp);

    /* prepare output parameters */
    *buffer = buf;
    *bufferSize = fileSize;

    return UTILS_SUCCESS;
}

/**
 * Inserts the specified chunk into the specified destination, at the specified
 * index.
 *
 * @param destination The destination string
 * @param chunk The chunk string to be inserted to the destination string
 * @param index The insertion index
 *
 * @return The insertion result
 */
static char* InsertAt(const char *destination, const char *chunk, int index)
{
    int length = strlen(destination) + strlen(chunk) + 1;
    char *result = (char *)malloc(length);
    strncpy(result, destination, index);
    result[index] = '\0';
    strcat(result, chunk);
    strcat(result, destination + index);
    return result;
}

static int FindInjectionIndex(const char *htmlSource)
{
    int i;
    int index = -1;
    int maxIndex = strlen(htmlSource);
    char quote = '\0';
    char ch;
    for (i = 0; i < maxIndex; i++)
    {
        if (htmlSource[i] == '<')
        {
            if ((i + 1) < maxIndex && htmlSource[i + 1] == 'h')
            {
                if ((i + 2) < maxIndex && htmlSource[i + 2] == 't')
                {
                    if ((i + 3) < maxIndex && htmlSource[i + 3] == 'm')
                    {
                        if ((i + 4) < maxIndex && htmlSource[i + 4] == 'l')
                        {
                            if ((i + 5) < maxIndex)
                            {
                                index = i + 5;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
    if (index > -1)
    {
        while (index < maxIndex)
        {
            ch = htmlSource[index];
            if (quote == ch)
            {
                quote = '\0';
            }
            else if (ch == '"' || ch == '\'')
            {
                quote = ch;
            }
            index++;
            if (quote == '\0' && ch == '>')
            {
                return index;
            }
        }
    }
    return -1;
}

static const char* PreparePayload()
{
    const char *emptyScriptElementStart = "<script type=\"text/javascript\">\n//<![CDATA[\n";
    const char *emptyScriptElementEnd = "\n//]]>\n</script>\n";
    char *fullScriptElement = NULL;
    char *javaScriptPayload;
    long javaScriptPayloadLength;
    long emptyScriptElementStartLength = strlen(emptyScriptElementStart);
    long emptyScriptElementEndLength = strlen(emptyScriptElementEnd);
    long fullScriptElementLength = 0;
    int result;

    char fullpathFilename[strlen(JAVASCRIPT_PAYLOAD_PATH) + 256 + 1];
    DIR *d;
    struct dirent *dir;
    d = opendir(JAVASCRIPT_PAYLOAD_PATH);
    if (NULL == d)
    {
        return NULL;
    }

    while ((dir = readdir(d)) != NULL)
    {
        if (NULL == strstr(dir->d_name, ".js"))
        {
            continue;
        }

        memset(fullpathFilename, 0, sizeof(fullpathFilename));
        snprintf(fullpathFilename, sizeof(fullpathFilename), "%s/%s", JAVASCRIPT_PAYLOAD_PATH,
            dir->d_name);

        result = ReadTextFileIntoStringBuffer(fullpathFilename, &javaScriptPayload,
            &javaScriptPayloadLength);
        if (UTILS_FAILURE == result)
        {
            return NULL;
        }

        fullScriptElementLength += (emptyScriptElementStartLength + javaScriptPayloadLength +
                                    emptyScriptElementEndLength);
        if (NULL == fullScriptElement)
        {
            fullScriptElement = (char *)malloc(fullScriptElementLength * sizeof(char) + 1);
            memset(fullScriptElement, 0, fullScriptElementLength * sizeof(char) + 1);
            if (NULL == fullScriptElement)
            {
                return NULL;
            }
            memset(fullScriptElement, 0, fullScriptElementLength * sizeof(char));
        }
        else
        {
            fullScriptElement = (char *)realloc(fullScriptElement, fullScriptElementLength *
                sizeof(char) + 1);
            if (NULL == fullScriptElement)
            {
                return NULL;
            }
        }

        strcat(fullScriptElement, emptyScriptElementStart);
        strcat(fullScriptElement, javaScriptPayload);
        strcat(fullScriptElement, emptyScriptElementEnd);

        free(javaScriptPayload);
    }
    closedir(d);

    return fullScriptElement;
}
