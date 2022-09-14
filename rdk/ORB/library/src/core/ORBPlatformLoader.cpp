/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#include "ORBPlatformLoader.h"
#include "Logging.h"
#include <dlfcn.h>

#define ORB_PLATFORM_IMPL_LIBRARY_NAME "/usr/lib/libORBPlatformImpl.so"

namespace orb {
/**
 * Constructor.
 */
ORBPlatformLoader::ORBPlatformLoader()
    : m_lib(nullptr)
{
}

/**
 * Destructor.
 */
ORBPlatformLoader::~ORBPlatformLoader()
{
}

/**
 * Load the ORB implementation library.
 *
 * @return A pointer to the resulting ORBPlatform object
 */
ORBPlatform * ORBPlatformLoader::Load()
{
    ORB_LOG_NO_ARGS();

    char *errorMsg = nullptr;

    dlerror();
    m_lib = dlopen(ORB_PLATFORM_IMPL_LIBRARY_NAME, RTLD_LAZY);
    if (NULL == m_lib)
    {
        errorMsg = dlerror();
        if (errorMsg)
        {
            ORB_LOG("ERROR: %s", errorMsg);
            return nullptr;
        }
    }

    ORB_LOG("dlopen success");

    dlerror();
    CreatePlatformInstance_t *Create = reinterpret_cast<CreatePlatformInstance_t *>(dlsym(m_lib,
        "Create"));
    errorMsg = dlerror();
    if (errorMsg)
    {
        ORB_LOG("ERROR: %s", errorMsg);
        return nullptr;
    }

    ORB_LOG("dlsym success");

    ORBPlatform *platform = Create();

    ORB_LOG("Create ORBPlatform success");

    return platform;
}

/**
 * Unload the ORB platform implementation library.
 *
 * @param orbPlatform Pointer to the ORB platform object
 *
 * @return true in success, false otherwise
 */
bool ORBPlatformLoader::Unload(ORBPlatform *orbPlatform)
{
    char *errorMsg = nullptr;

    ORB_LOG_NO_ARGS();

    dlerror();
    DestroyPlatformInstance_t *Destroy = reinterpret_cast<DestroyPlatformInstance_t *>(dlsym(m_lib,
        "Destroy"));
    errorMsg = dlerror();
    if (errorMsg)
    {
        ORB_LOG("ERROR: %s", errorMsg);
        return false;
    }

    ORB_LOG("dlsym success");

    Destroy(orbPlatform);

    ORB_LOG("Destroy ORBPlatform success");

    dlerror();
    dlclose(m_lib);
    errorMsg = dlerror();
    if (errorMsg)
    {
        ORB_LOG("ERROR: %s", errorMsg);
        return false;
    }

    ORB_LOG("Success");
    return true;
}
} // namespace orb
