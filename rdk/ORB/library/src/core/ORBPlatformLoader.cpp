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
 */

#include "ORBPlatformLoader.h"
#include "ORBLogging.h"
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
