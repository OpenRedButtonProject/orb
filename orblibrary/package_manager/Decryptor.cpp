#include "Decryptor.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>

#ifdef IS_CHROMIUM
#include "third_party/boringssl/src/include/openssl/sha.h"
#else
#include <openssl/sha.h>
#endif

PackageOperationResult Decryptor::decrypt(const std::string& filePath) const
{
  return PackageOperationResult(true, "", {filePath});
}
