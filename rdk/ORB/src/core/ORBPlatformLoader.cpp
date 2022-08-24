/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#include "ORBPlatformLoader.h"
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
ORBPlatform *ORBPlatformLoader::Load()
{
  fprintf(stderr, "[ORBPlatformLoader::Load]\n");

  char *errorMsg = nullptr;

  dlerror();
  m_lib = dlopen(ORB_PLATFORM_IMPL_LIBRARY_NAME, RTLD_LAZY);
  if (NULL == m_lib) {
    errorMsg = dlerror();
    if (errorMsg) {
      fprintf(stderr, "[ORBPlatformLoader::Load] ERROR: %s\n", errorMsg);
      return nullptr;
    }
  }

  fprintf(stderr, "[ORBPlatformLoader::Load] dlopen success\n");

  dlerror();
  CreatePlatformInstance_t *Create = reinterpret_cast<CreatePlatformInstance_t *>(dlsym(m_lib, "Create"));
  errorMsg = dlerror();
  if (errorMsg) {
    fprintf(stderr, "[ORBPlatformLoader::Load] ERROR: %s\n", errorMsg);
    return nullptr;
  }

  fprintf(stderr, "[ORBPlatformLoader::Load] dlsym success\n");

  ORBPlatform *platform = Create();

  fprintf(stderr, "[ORBPlatformLoader::Load] Create ORBPlatform success\n");

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

  fprintf(stderr, "[ORBPlatformLoader::Unload]\n");

  dlerror();
  DestroyPlatformInstance_t *Destroy = reinterpret_cast<DestroyPlatformInstance_t *>(dlsym(m_lib, "Destroy"));
  errorMsg = dlerror();
  if (errorMsg) {
    fprintf(stderr, "[ORBPlatformLoader::Unload] ERROR: %s\n", errorMsg);
    return false;
  }

  fprintf(stderr, "[ORBPlatformLoader::Unload] dlsym success\n");

  Destroy(orbPlatform);

  fprintf(stderr, "[ORBPlatformLoader::Unload] Destroy ORBPlatform success\n");

  dlerror();
  dlclose(m_lib);
  errorMsg = dlerror();
  if (errorMsg) {
    fprintf(stderr, "[ORBPlatformLoader::Unload] ERROR: %s\n", errorMsg);
    return false;
  }

  fprintf(stderr, "[ORBPlatformLoader::Unload] Success\n");
  return true;
}

} // namespace orb
